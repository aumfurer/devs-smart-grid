#include <random>
#include <string>

#include "message.h"
#include "parsimu.h"
#include "real.h"
#include "tuple_value.h"

#include "celda_solar.h"

using namespace std;

CeldaSolar::CeldaSolar(const string &name) :
	Atomic(name),
	solar_change(addInputPort("solar_change")),
	out(addOutputPort("out")),
	on(true),
	factor(1.0),
	energia_produciendo(0),
	energia_produciendo_previo(0)
{
}


Model &CeldaSolar::initFunction()
{
	holdIn(AtomicState::active, VTime::Inf);
	return *this;
}


/* 
	Como "solar_change" es el úncio input_port de una CeldaSolar,
	si se produce una transición externa, se debe a que llegó un mensaje
	en dicho puerto
 */
Model &CeldaSolar::externalFunction(const ExternalMessage &msg)
{
	this->energia_produciendo_previo = this->energia_produciendo;
	double radiacion = Real::from_value(msg.value()).value();
	this->energia_produciendo = radiacion * this->factor;
	holdIn(AtomicState::active, VTime::Zero);
	return *this;
}


Model &CeldaSolar::internalFunction(const InternalMessage &)
{
	holdIn(AtomicState::active, VTime::Inf);
	return *this ;
}


Model &CeldaSolar::outputFunction(const CollectMessage &msg)
{
	auto delta_energia = Real(this->energia_produciendo - this->energia_produciendo_previo);
	sendOutput(msg.time(), out, delta_energia);
	return *this ;
}
