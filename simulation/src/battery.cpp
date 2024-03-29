#include "battery.h"
#define DEBUG

#ifdef DEBUG
    #define NEXT_CHANGE(x) { \
        if((x).asMsecs()<0){ \
            cerr << "generando negativo en linea " << __LINE__<< ": " << (x) << endl; \
            exit(1); \
        } else { \
            nextChange(x); \
        } \
    }
    #define NEXT_CHANGE_AND_LOG(x, currentTime) (cerr << "NEXT_TIME;" << currentTime << ";" << x << endl, nextChange(x))

    #define VTIME_FROM_HOURS(hours) (((hours) > 500) ? (cerr << "Tiempo de espera infinito " << __LINE__<< ": " << (hours) << endl, VTimeFromHoursNew((hours))) : \
        ((hours) < 0) ? (cerr << "Tiempo negative con valor " << hours << " en la linea " << __LINE__ << endl),VTimeFromHoursNew((hours)) : VTimeFromHoursNew((hours)))


#else
    #define NEXT_CHANGE(x) nextChange(x)
    #define NEXT_CHANGE_AND_LOG(x, currentTime) nextChange(x)
#endif

const VTime VTimeFromHoursNew(double hours)
{
    // Convert through using hours in seconds
    return hours > 500 ?
        VTime::Inf:
        VTime((float)(hours * 3600));
}

Battery::Battery(const string &name) :
	Atomic(name),

    // Old ports remained for back compatibility
	// Input ports
	solarPanelEnergyIn(addInputPort("solar_panel")),
	windTurbineEnergyIn(addInputPort("wind_turbine")),
	required_energy(addInputPort("required_energy")),

	// Output ports
	battery_state(addOutputPort("battery_state")),
	surplus_energy(addOutputPort("surplus_energy")),

    // State variables
    solarPanelPower(0),
    windTurbinePower(0),
    controllerDemand(0),
    charge(0),
    state(BatteryState::Empty),

    lastChargeUpdate(VTime::Zero)
{
}


// Passivate until an external transition happeds, meaning that, possibly,
// one of both generators becomes available.
Model &Battery::initFunction()
{
	holdIn(AtomicState::active, VTime::Inf);
	return *this;
}

Model &Battery::externalFunction(const ExternalMessage &aMessage)
{
    #ifdef DEBUG
        std::cerr << "Message arrived from model with id " << aMessage.senderModelId()
            << " in time " << aMessage.time() << std::endl;
    #endif

    // Update charge and reset last update time
    this->charge = calculateNewCharge(aMessage.time());

    #ifdef DEBUG
        cerr << "NewCharge;" << aMessage.time() << ";" << this->charge << endl;
    #endif

    this->lastChargeUpdate = aMessage.time();

    this->update_energy_producing(aMessage);

    // Since this is not an external transition, or it should 
    // have happend before the one that was scheduled. That means
    // that no state change will happeds here, but the time till the 
    // next one might have changed due to the generated powet update.

    double energy_producing = this->energy_producing();

    if (this->state == BatteryState::Charging)
    {
        if (energy_producing > EPSILON)
        {
            // Time to reach an available energy level
            NEXT_CHANGE_AND_LOG(VTIME_FROM_HOURS((AVAILABE_CAPACITY + EPSILON - this->charge) / energy_producing), aMessage.time());
        } 
        else
        {
            // No power being generated and not in an available level yet
            passivate();
        }
    }
    else if (this->state == BatteryState::Available)
    {
        if (energy_producing > EPSILON)
        {
            // Time to fill battery
            NEXT_CHANGE_AND_LOG(VTIME_FROM_HOURS((CAPACITY - this->charge) / energy_producing), aMessage.time());
        }
        else if (energy_producing < -EPSILON)
        {
            // Time to empty stored energy
            NEXT_CHANGE_AND_LOG(VTIME_FROM_HOURS(this->charge / -energy_producing), aMessage.time());
        }
        else
        {
            // Absolute energy being consumed or generated is zero,
            // nothing will happend until further news
            passivate();
        }
    }
    // In both Empty and Full states, a change will occur if certain conditions are given in external transitions
    else if (this->state == BatteryState::Empty && energy_producing > EPSILON)
    {
        NEXT_CHANGE_AND_LOG(VTime::Zero, aMessage.time());
    }
    else if (this->state == BatteryState::Full && energy_producing < -EPSILON)
    {
        NEXT_CHANGE_AND_LOG(VTime::Zero, aMessage.time());
    }
    else if (this->state == BatteryState::Full && energy_producing > EPSILON)
    {
        NEXT_CHANGE_AND_LOG(VTime::Zero, aMessage.time());
    }
    // In the case no branch is reached, it means that the battery is in Full or Empty state, and
    // with no transition condition
    else {
        NEXT_CHANGE_AND_LOG(VTime::Inf, aMessage.time());
    }

	return *this;
}

Model &Battery::internalFunction(const InternalMessage &aMessage)
{
    if (this->state == BatteryState::Full && this->energy_producing() > EPSILON){
        // there's no change, it's here only to send surplus energy on output function
        NEXT_CHANGE(VTime::Inf);
    } else {
        // Update charge and reset last update time
        this->charge = calculateNewCharge(aMessage.time());
        this->lastChargeUpdate = aMessage.time();

        auto state_nextChange = this->calculate_next_state(this->charge);

        this->state = state_nextChange.first;
        NEXT_CHANGE(state_nextChange.second);
    }
	return *this;
}

Model &Battery::outputFunction(const CollectMessage &aMessage)
{
    if (this->state == BatteryState::Full){
        double extra_energy = this->energy_producing();
        sendOutput(aMessage.time(), this->surplus_energy, max(0.0, extra_energy));
        if (extra_energy > EPSILON){
            // it came here only to inform surplus energy
            return *this;
        }
    }
    // If state has changed, notify port consumers
    double charge = calculateNewCharge(aMessage.time());
    sendOutput(aMessage.time(), this->battery_state, this->calculate_next_state(charge).first);
	return *this ;
}


double Battery::calculateNewCharge(VTime currentTime) const
{
    // Convert elapsed time to hours
    double elapsedTimeBetweenUpdates = (currentTime - this->lastChargeUpdate).asMsecs() / (double) (3600 * 1000);
    double chargingRate;
    if (this->state == BatteryState::Charging)
    {
        chargingRate = this->solarPanelPower + this->windTurbinePower;
    }
    else if (this->state == BatteryState::Empty)
    {
        chargingRate = 0.0;
    }
    else
    {
        chargingRate = this->solarPanelPower + this->windTurbinePower - this->controllerDemand;
    }

    double charge = this->charge + chargingRate * elapsedTimeBetweenUpdates;
    // Keep charge between 0 and CAPACITY
    return std::max(0.0, std::min((double) CAPACITY, charge));
}

void Battery::update_energy_producing(const ExternalMessage &aMessage)
{
    double messageValue = Real::from_value(aMessage.value()).value();
    if(aMessage.port() == this->solarPanelEnergyIn)
    {
		this->solarPanelPower = messageValue;
	} else 
    if(aMessage.port() == this->windTurbineEnergyIn)
    {
		this->windTurbinePower = messageValue;
	} else
    if(aMessage.port() == this->required_energy)
    {
        this->controllerDemand = messageValue;
	}
}

double Battery::energy_producing() const
{
    if (this->state == BatteryState::Charging || this->state == BatteryState::Empty){
        return this->solarPanelPower + this->windTurbinePower;
    } else {
        return this->solarPanelPower + this->windTurbinePower - this->controllerDemand;
    }
}

pair<const BatteryState, VTime> Battery::calculate_next_state(double charge) const 
{

    double generatorsEnergy = this->solarPanelPower + this->windTurbinePower; 
    double totalPower = generatorsEnergy - this->controllerDemand;

    switch (this->state) 
    {
        case BatteryState::Empty:    
            assert(generatorsEnergy > EPSILON);
            return make_pair(
                BatteryState::Charging,
                VTIME_FROM_HOURS((double)(AVAILABE_CAPACITY + EPSILON) / (generatorsEnergy))
            );
            break;

        case BatteryState::Charging:
            assert(charge > AVAILABE_CAPACITY);
            assert(generatorsEnergy > EPSILON);
            if (totalPower < -EPSILON)
            {
                return make_pair(BatteryState::Available, VTIME_FROM_HOURS((double) charge / -totalPower));
            }
            else if (totalPower > EPSILON)
            {
                return make_pair(
                    BatteryState::Available, 
                    VTIME_FROM_HOURS((double) (CAPACITY - this->charge) / totalPower)
                );
            }
            else
            {
                return make_pair(BatteryState::Available, VTime::Inf);
            }
            break;

        case BatteryState::Available:
            if(charge < EPSILON)
            {
                return make_pair(BatteryState::Empty, VTime::Inf);
            } else 
            if (charge > MAXIMUM_POWER - EPSILON)
            {
                assert(totalPower > EPSILON);
                return make_pair(BatteryState::Full, VTime::Inf);
            } else 
            {
                assert(false);
            }
            break;

        case  BatteryState::Full:
            assert(totalPower < -EPSILON);
            return make_pair(BatteryState::Available, VTIME_FROM_HOURS(charge / -totalPower));
            break;
    }
}