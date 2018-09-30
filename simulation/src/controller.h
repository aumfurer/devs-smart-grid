#ifndef _CONTROLLER_H_
#define _CONTROLLER_H_

#include <random>

#include "atomic.h"
#include "VTime.h"


#define CONTROLLER_NAME "controller"

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
    const Port &batteryState;
    const Port &loadDemand;

    // Output ports
    Port &gridDemand;
    Port &batteryDemand;

};

#endif
