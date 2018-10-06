#ifndef _BATERIA_H_
#define _BATERIA_H_

#include <random>

#include "atomic.h"
#include "VTime.h"


#define BATERIA_NAME "bateria"

class Bateria : public Atomic {
  public:

    Bateria(const string &name = BATERIA_NAME );
    virtual string className() const {  return BATERIA_NAME ;}
  
    static double CAPACITY;
    static double MAXIMUM_POWER;
    static double MIN_CAPACITY;

    const static int EMPTY = 0;
    const static int AVAILABLE = 1;
    const static int FULL = 2;

  protected:
    Model &initFunction();
    Model &externalFunction( const ExternalMessage & );
    Model &internalFunction( const InternalMessage & );
    Model &outputFunction( const CollectMessage & );

  private:
    const Port &solarPanelEnergyIn;
    const Port &windTurbineEnergyIn;
    const Port &required_energy;
    Port &battery_state;

    double solarPanelPower;
    double windTurbinePower;

    double energy_from_generators;
    double energy_sending;
    double charge;
    double energyRequiredByLoad;
    VTime last_update;
    // int next_state;

    // void update_current_charge(const VTime &update_time);
    double new_current_charge(const VTime &update_time);
    void update_next_event();
    void updateGeneratedPower();

    double WattsToWattsPerMsecond(double);

    VTime to_VTime(double v);
};

#endif
