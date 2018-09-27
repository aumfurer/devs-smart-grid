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
  
    const double CAPACITY = 500.0;
    const double MIN_CAPACITY = 100.0;

    const int EMPTY = 0;
    const int AVAILABLE = 1;
    const int FULL = 2;

  protected:
    Model &initFunction();
    Model &externalFunction( const ExternalMessage & );
    Model &internalFunction( const InternalMessage & );
    Model &outputFunction( const CollectMessage & );

  private:
    const Port &energy_in;
    const Port &required_energy;
    Port &battery_state;

    double energy_from_generators;
    double energy_sending;
    double charge;
    VTime last_update;
    // int next_state;

    // void update_current_charge(const VTime &update_time);
    double new_current_charge(const VTime &update_time);
    void update_next_event();

    VTime to_VTime(double v);
};

#endif
