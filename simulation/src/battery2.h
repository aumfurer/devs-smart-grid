#ifndef _BATERIA_H_
#define _BATERIA_H_

#include "atomic.h"
#include "VTime.h"
#include <cmath>
#include <iostream>
#include <utility>

#define CAPACITY 500
#define MAXIMUM_POWER 500
#define AVAILABE_CAPACITY 100

#define BATTERY_NAME "bateria"

#define EPSILON 1e-3

enum BatteryState
{
  Empty,
  Charging,
  Available,
  Full
};

class Battery2 : public Atomic {
  public:

    Battery2(const string &name = BATTERY_NAME );
    virtual string className() const {  return BATTERY_NAME ;}

  protected:
    Model &initFunction();
    Model &externalFunction( const ExternalMessage & );
    Model &internalFunction( const InternalMessage & );
    Model &outputFunction( const CollectMessage & );

  private:
    // Ports. They are the same to maintain back compatibility
    const Port &solarPanelEnergyIn;
    const Port &windTurbineEnergyIn;
    const Port &required_energy;
    Port &battery_state;
    Port &surplus_energy;

    // State variables
    double solarPanelPower;
    double windTurbinePower;
    double controllerDemand;
    double charge;
    BatteryState state;

    // Used to calculate the charge at each transition
    VTime lastChargeUpdate;
    bool stateHasChanged;

    double calculateNewCharge(VTime currentTime) const;
    void update_energy_producing(const ExternalMessage &aMessage);
    double energy_producing() const;
    pair<const BatteryState, VTime> calculate_next_state(double charge) const;
};

#endif
