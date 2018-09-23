#ifndef _SOLAR_H_
#define _SOLAR_H_

#include <random>

#include "atomic.h"
#include "VTime.h"


#define CELDA_SOLAR_NAME "celda_solar"


class CeldaSolar : public Atomic {
  public:
    
    CeldaSolar(const string &name = CELDA_SOLAR_NAME );
    virtual string className() const {  return CELDA_SOLAR_NAME ;}
  
  protected:
    Model &initFunction();
    Model &externalFunction( const ExternalMessage & );
    Model &internalFunction( const InternalMessage & );
    Model &outputFunction( const CollectMessage & );

  private:
    const Port &solar_change;
    Port &out;

    bool on;

    const float factor;

    double energia_produciendo;
    double energia_produciendo_previo;
};

#endif
