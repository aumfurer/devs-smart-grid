#include <random>
#include <string>

#include "message.h"
#include "parsimu.h"
#include "real.h"
#include "tuple_value.h"

#include "generador_eolico.h"

using namespace std;


GeneradorEolico::GeneradorEolico(const string &name) :
	Atomic(name),
	wind_charge(addInputPort("solar_change")),
	out(addOutputPort("out")),
	on(true),
	factor(1.0),
	energia_produciendo(0),
	energia_produciendo_previo(0)
{
}

Model &GeneradorEolico::initFunction()
{
	holdIn(AtomicState::active, VTime::Inf);
	return *this;
}


Model &GeneradorEolico::externalFunction(const ExternalMessage &msg)
{
	this->energia_produciendo_previo = this->energia_produciendo;
	double radiacion = Real::from_value(msg.value()).value();
	this->energia_produciendo = radiacion * this->factor;
	holdIn(AtomicState::active, VTime::Zero);
	return *this;
}


Model &GeneradorEolico::internalFunction(const InternalMessage &)
{
	holdIn(AtomicState::active, VTime::Inf);
	return *this ;
}


Model &GeneradorEolico::outputFunction(const CollectMessage &msg)
{
	auto delta_energia = Real(this->energia_produciendo - this->energia_produciendo_previo);
	sendOutput(msg.time(), out, delta_energia);
	return *this ;
}
