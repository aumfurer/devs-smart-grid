#include "controller.h"

Controller::Controller(const string &name) :
    Atomic(name),

    // Input ports
    batteryState(addInputPort("battery_state")),
    loadDemand(addInputPort("load_demand")),

    // Output ports
    gridDemand(addOutputPort("grid_demand")),
    batteryDemand(addOutputPort("battery_demand"))
{}

Model& Controller::initFunction() {

}

Model& Controller::externalFunction( const ExternalMessage &aMessage) {

}

Model& Controller::internalFunction( const InternalMessage &aMessage) {

}

Model& Controller::outputFunction( const CollectMessage &aMessage) {

}