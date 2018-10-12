#include "battery.h"
#define DEBUG

VTime VTimeFromHours(double hours)
{
    // Convert through using hours in ms
	return VTime( 0, 0, 0, hours * 3600 * 1000, 0);
}

Battery::Battery(const string &name) :
	Atomic(name),

    // Old ports remained for back compatibility
	// Input ports
	windTurbineEnergyIn(addInputPort("wind_turbine")),
	solarPanelEnergyIn(addInputPort("solar_panel")),
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

    lastChargeUpdate(VTime::Zero),
    stateHasChanged(false)
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
    calculateNewCharge(aMessage.time());
    this->lastChargeUpdate = aMessage.time();

	double messageValue = Real::from_value(aMessage.value()).value();

    // Update new state variables
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

    // Since this is not an external transition, or it should 
    // have happend before the one that was scheduled. That means
    // that no state change will happeds here, but the time till the 
    // next one might have changed due to the generated powet update.

    if (this->state == BatteryState::Charging)
    {
        if (this->solarPanelPower + this->windTurbinePower > EPSILON)
        {
            // Time to reach an available energy level
            nextChange(VTimeFromHours((AVAILABE_CAPACITY + EPSILON) / (this->solarPanelPower + this->windTurbinePower)));
        } 
        else
        {
            // No power being generated and not in an available level yet
            passivate();
        }
    }
    else if (this->state == BatteryState::Available)
    {
        double totalPower = this->solarPanelPower + this->windTurbinePower - this->controllerDemand;
        if (totalPower > EPSILON)
        {
            // Time to fill battery
            nextChange(VTimeFromHours(CAPACITY / totalPower));
        }
        else if (totalPower < -EPSILON)
        {
            // Time to empty stored energy
            nextChange(VTimeFromHours(this->charge / -totalPower));
        }
        else
        {
            // Absolute energy being consumed or generated is zero,
            // nothing will happend until further news
            passivate();
        }
    }
    // In both Empty and Full states, a change will occur if certain conditions are given in external transitions
    else if (this->state == BatteryState::Empty && (this->solarPanelPower + this->windTurbinePower) > EPSILON)
    {
        nextChange(VTime::Zero);
    }
    else if (this->state == BatteryState::Full && (this->solarPanelPower + this->windTurbinePower - this->controllerDemand) < -EPSILON)
    {
        nextChange(VTime::Zero);
    }
    // In the case no branch is reached, it means that the battery is in Full or Empty state, and
    // with no transition condition
    else {
        passivate();
    }

	return *this;
}

Model &Battery::internalFunction(const InternalMessage &aMessage)
{
    // Update charge and reset last update time
    calculateNewCharge(aMessage.time());
    this->lastChargeUpdate = aMessage.time();

    double totalPower = this->solarPanelPower + this->windTurbinePower - this->controllerDemand;

    // If an internal transition is reached, state should change
    if (this->state == BatteryState::Empty && (this->solarPanelPower + this->windTurbinePower) > EPSILON)
    {
        this->stateHasChanged = true;
        this->state = BatteryState::Charging;
        nextChange(VTimeFromHours((double)(AVAILABE_CAPACITY + EPSILON) / (this->solarPanelPower + this->windTurbinePower)));
    }
    else if (this->state == BatteryState::Charging 
        && this->charge > AVAILABE_CAPACITY 
        && (this->solarPanelPower + this->windTurbinePower) > EPSILON)
    {
        this->stateHasChanged = true;
        this->state = BatteryState::Available;
        // TODO: I've already done this check. Refactor this
        // Consider case in which I already have a demand from the controller, so
        // totalPower could be negative
        if (totalPower < -EPSILON)
        {
            nextChange(VTimeFromHours((double) this->charge / -totalPower));
        }
        else if (totalPower > EPSILON)
        {
            nextChange(VTimeFromHours((double) CAPACITY / totalPower));
        }
        else
        {
            nextChange(VTime::Inf);
        }
    }
    else if (this->state == BatteryState::Available
        && this->charge < EPSILON)
    {
        this->stateHasChanged = true;
        this->state = BatteryState::Empty;
        nextChange(VTime::Inf);
    }
    else if (this->state == BatteryState::Available
        && this->charge > (MAXIMUM_POWER - EPSILON)
        && (this->solarPanelPower + this->windTurbinePower - this->controllerDemand) > EPSILON)
    {
        this->stateHasChanged = true;
        this->state = BatteryState::Full;
        nextChange(VTime::Inf);
    }
    else if (this->state == BatteryState::Full
        && (this->solarPanelPower + this->windTurbinePower - this->controllerDemand) < -EPSILON)
    {
        this->stateHasChanged = true;
        this->state = BatteryState::Available;
        nextChange(VTimeFromHours(this->charge / -totalPower));
    }
    else
    {
        std::cerr << "No state transition happend" << std::endl;
        // Fail fast
        exit(1);
    }

	return *this;
}

Model &Battery::outputFunction(const CollectMessage &aMessage)
{
    // If state has changed, notify port consumers
    if (this->stateHasChanged)
    {
	    sendOutput(aMessage.time(), this->battery_state, this->state);
    }

    // Clear state change flag
    this->stateHasChanged = false;

	return *this ;
}


void Battery::calculateNewCharge(VTime currentTime)
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

    this->charge += chargingRate * elapsedTimeBetweenUpdates;
    // Keep charge between 0 and CAPACITY
    this->charge = std::max(0.0, std::min((double) CAPACITY, this->charge));
}