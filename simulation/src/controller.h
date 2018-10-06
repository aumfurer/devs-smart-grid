#ifndef _CONTROLLER_H_
#define _CONTROLLER_H_

#include <random>

#include "atomic.h"
#include "VTime.h"
#include "bateria.h"

#define BATTERY_STATE_PORT "battery_state"
#define LOAD_DEMAND_PORT "load_demand"

    // Output ports
#define GRID_DEMAND_PORT "grid_demand"
#define BATTERY_DEMAND_PORT "battery_demand"

#define CONTROLLER_NAME "controller"

#define MessageValueAsDouble(aMessage) Real::from_value(aMessage.value()).value()

class Controller : public Atomic 
{
  public:

    Controller(const string &name = CONTROLLER_NAME );
    virtual string className() const {  return CONTROLLER_NAME ;}

  protected:
    Model &initFunction();
    Model &externalFunction( const ExternalMessage & );
    Model &internalFunction( const InternalMessage & );
    Model &outputFunction( const CollectMessage & );

  private:

    // Input ports
    const Port &batteryStatePort;
    const Port &loadDemand;

    // Output ports
    Port &gridDemandPort;
    Port &batteryDemandPort;

    // State variables
    int batteryState;
    double currentLoadDemand; 
    double batteryDemand;
    double gridDemand;

    // Helper methods
    void updateGridConsumption();
};

#endif
