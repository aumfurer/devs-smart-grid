#ifndef _EOLICO_H_
#define _EOLICO_H_

#include <random>

#include "atomic.h"
#include "VTime.h"


#define GENERADOR_EOLICO_NAME "generador_eolico"


class GeneradorEolico : public Atomic {
  public:
    
    GeneradorEolico(const string &name = GENERADOR_EOLICO_NAME );
    virtual string className() const {  return GENERADOR_EOLICO_NAME ;}
  
  protected:
    Model &initFunction();
    Model &externalFunction( const ExternalMessage & );
    Model &internalFunction( const InternalMessage & );
    Model &outputFunction( const CollectMessage & );

  private:
    const Port &wind_charge;
    Port &out;

    bool on;

    const float factor;

    double energia_produciendo;
    double energia_produciendo_previo;
};

#endif
