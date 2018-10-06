#include "controller.h"

Controller::Controller(const string &name) :
    Atomic(name),

    // Input ports
    batteryStatePort(addInputPort(BATTERY_STATE_PORT)),
    loadDemand(addInputPort(LOAD_DEMAND_PORT)),

    // Output ports
    gridDemandPort(addOutputPort(GRID_DEMAND_PORT)),
    batteryDemandPort(addOutputPort(BATTERY_DEMAND_PORT)),

    // State variables
    batteryState(Bateria::EMPTY),
    currentLoadDemand(0.0)
    batteryDemand(0.0),
    gridDemand(0.0)
{}

Model& Controller::initFunction() {
	holdIn(AtomicState::active, VTime::Inf);
	return *this;
}

Model& Controller::externalFunction( const ExternalMessage &aMessage) {
    if (aMessage.port() == BATTERY_STATE_PORT) {
        int newBatteryState = (int) std::round(MessageValueAsDouble(aMessage));
        this->batteryState = newBatteryState;
    } else 
    if (aMessage.port() == LOAD_DEMAND_PORT) {
        this->currentLoadDemand = MessageValueAsDouble(aMessage);
    }
    updateGridConsumption();
    // Schedule an internal transition to propagate demand changes
    nextChange(VTime::Zero);
    return *this;
}

void Controller::updateGridConsumption() {
    this->gridDemand = this->currentLoadDemand - this->currentBatteryCharge;
    if (this->batteryState == Bateria::AVAILABLE || 
        this->batteryState == Bateria::FULL) {
        // Battery available to use as supply
        if (this->currentLoadDemand < Bateria::MAXIMUM_POWER) {
            // The whole load demand can be supplied by the battery
            this->batteryDemand = this->currentLoadDemand;
            this->gridDemand = 0;
        } else {
            this->batteryDemand = Bateria::MAXIMUM_POWER;
            this->gridDemand = this->currentLoadDemand - this->batteryDemand;
        }
    } else {
        this->batteryDemand = 0;
        this->gridDemand = this->currentLoadDemand;
    }
}

Model& Controller::internalFunction( const InternalMessage &aMessage) {
    passivate();
}

Model& Controller::outputFunction( const CollectMessage &aMessage) {
	sendOutput(aMessage.time(), this->gridDemandPort, this->gridDemand);
	sendOutput(aMessage.time(), this->batteryDemandPort, this->batteryDemand);
	return *this ;
}