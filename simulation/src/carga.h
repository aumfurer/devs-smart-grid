#ifndef _CARGA_H_
#define _CARGA_H_

#include <random>

#include "atomic.h"
#include "VTime.h"


#define CARGA_NAME "carga"


class Carga : public Atomic {
  public:
    
    Carga(const string &name = CARGA_NAME );
    virtual string className() const {  return CARGA_NAME ;}
  
  protected:
    Model &initFunction();
    Model &externalFunction( const ExternalMessage & );
    Model &internalFunction( const InternalMessage & );
    Model &outputFunction( const CollectMessage & );

  private:
    const Port &power_consumption;
    Port &out;

    double energia_consumiendo;
};

#endif
