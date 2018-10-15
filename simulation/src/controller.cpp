#include "battery.h"
#include "controller.h"

Controller::Controller(const string &name) :
    Atomic(name),

    // Input ports
    batteryStatePort(addInputPort(BATTERY_STATE_PORT)),
    batterySurplusEnergy(addInputPort(BATTERY_SURPLUS_ENERGY_PORT)),
    loadDemand(addInputPort(LOAD_DEMAND_PORT)),
    
    // Output ports
    gridDemandPort(addOutputPort(GRID_DEMAND_PORT)),
    batteryDemandPort(addOutputPort(BATTERY_DEMAND_PORT)),
    sellingEnergy(addOutputPort(SELLING_ENERGY_PORT)),

    // State variables
    batteryState(BatteryState::Empty),
    currentLoadDemand(0.0),
    batteryDemand(0.0),
    gridDemand(0.0),

    notifyBattery(false),
    notifyExtraEnergy(false),
    extraEnergy(0.0),
    state(ControllerState::AllGrid)
{}

Model& Controller::initFunction() {
	holdIn(AtomicState::active, VTime::Inf);
	return *this;
}

Model& Controller::externalFunction( const ExternalMessage &aMessage) {

    if (aMessage.port() == this->batterySurplusEnergy){
        this->notifyExtraEnergy = true;
        this->extraEnergy = MessageValueAsDouble(aMessage);
        nextChange(VTime::Zero);
        return *this;
    }

    // New battery state
    if (aMessage.port() == this->batteryStatePort) {
        this->batteryState = (int) std::round(MessageValueAsDouble(aMessage));

        // Since a new battery state arrived, update controler state
        updateControllerState();

    // New load demand
    } else if (aMessage.port() == this->loadDemand) {
        this->currentLoadDemand = MessageValueAsDouble(aMessage);
    }

    updateGridConsumption();

    nextChange(VTime::Zero);
    return *this;
}

void Controller::updateControllerState() {
    switch(this->state) {
        case ControllerState::AllGrid:
            if (this->batteryState == BatteryState::Available) {
                this->state = ControllerState::GridAndBattery;
            }
            break;

        case ControllerState::GridAndBattery:
            if (this->batteryState == BatteryState::Empty) {
                this->state = ControllerState::AllGrid;
                this->notifyBattery = true;
            }
            break;

        default:
            cerr << "Battery state " << this->batteryState << " arrived. No ControllerState change produced" << endl;
            break;
    }
}

void Controller::updateGridConsumption() {
    switch(this->state) {
        case ControllerState::AllGrid:
            this->batteryDemand = 0;
            this->gridDemand = this->currentLoadDemand;
            break;

        case ControllerState::GridAndBattery:
            this->batteryDemand = std::min((double) MAXIMUM_POWER, this->currentLoadDemand);
            this->gridDemand = this->currentLoadDemand - this->batteryDemand;
            break;
    }
}

Model& Controller::internalFunction( const InternalMessage &aMessage) {
    this->notifyExtraEnergy = false;
    nextChange(VTime::Inf);
    return *this;
}

Model& Controller::outputFunction( const CollectMessage &aMessage) {
    if(this->notifyExtraEnergy){
        sendOutput(aMessage.time(), this->sellingEnergy, this->extraEnergy);
        return *this;
    }

	sendOutput(aMessage.time(), this->gridDemandPort, this->gridDemand);

    if (this->state == ControllerState::GridAndBattery || this->notifyBattery)
	    sendOutput(aMessage.time(), this->batteryDemandPort, this->batteryDemand);

    if (this->state == ControllerState::AllGrid) {
        this->notifyBattery = false;
    }

	return *this;
}