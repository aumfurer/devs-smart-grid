#include "controller.h"

Controller::Controller(const string &name) :
    Atomic(name),

    batteryState(addInputPort("battery_state")),
    loadDemand(addInputPort("load_demand")),

    // Output ports
    gridDemand(addOutputPort("grid_demand")),
    batteryDemand(addOutputPort("battery_demand"))
{}
