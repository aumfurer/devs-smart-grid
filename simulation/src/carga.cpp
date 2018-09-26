#include <random>
#include <string>

#include "message.h"
#include "parsimu.h"
#include "real.h"
#include "tuple_value.h"

#include "carga.h"

using namespace std;

Carga::Carga(const string &name) :
	Atomic(name),
	power_consumption(addInputPort("power_consumption")),
	out(addOutputPort("out")),
	energia_consumiendo(0)
{
}


Model &Carga::initFunction()
{
	holdIn(AtomicState::active, VTime::Inf);
	return *this;
}


Model &Carga::externalFunction(const ExternalMessage &msg)
{
	double consumo = Real::from_value(msg.value()).value();
	this->energia_consumiendo = consumo;
	holdIn(AtomicState::active, VTime::Zero);
	return *this;
}


Model &Carga::internalFunction(const InternalMessage &)
{
	holdIn(AtomicState::active, VTime::Inf);
	return *this;
}


Model &Carga::outputFunction(const CollectMessage &msg)
{
	auto delta_energia = Real(this->energia_consumiendo);
	sendOutput(msg.time(), out, delta_energia);
	return *this ;
}
