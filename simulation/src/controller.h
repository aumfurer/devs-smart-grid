#ifndef _CONTROLLER_H_
#define _CONTROLLER_H_

#include <random>

#include "atomic.h"
#include "VTime.h"
#include <cmath>

#define BATTERY_STATE_PORT "battery_state"
#define LOAD_DEMAND_PORT "load_demand"
#define BATTERY_SURPLUS_ENERGY_PORT "battery_surplus_energy"
#define SELLING_ENERGY_PORT "selling_energy"

    // Output ports
#define GRID_DEMAND_PORT "grid_demand"
#define BATTERY_DEMAND_PORT "battery_demand"

#define CONTROLLER_NAME "controller"

#define MessageValueAsDouble(aMessage) Real::from_value(aMessage.value()).value()

enum ControllerState {
  AllGrid,
  GridAndBattery
};

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
    const Port &batterySurplusEnergy;
    const Port &loadDemand;

    // Output ports
    Port &gridDemandPort;
    Port &batteryDemandPort;
    Port &sellingEnergy;

    // State variables
    int batteryState;
    double currentLoadDemand; 
    double batteryDemand;
    double gridDemand;

    ControllerState state;
    bool notifyBattery;
    bool notifyExtraEnergy;
    double extraEnergy;

    // Helper methods
    void updateGridConsumption();
    void updateControllerState();
};

#endif
