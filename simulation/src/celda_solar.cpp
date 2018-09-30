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

	// Input ports
	solar_change(		addInputPort("solar_change")),
	// Output ports
	out(				addOutputPort("out")),

	// Variables de estado
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

	// TODO: Cambiar esta forma de calcular la energía producida, y extraerla a un método
	this->energia_produciendo = radiacion * this->factor;

	// Hack para hacer una transición interna luego de recibir una novedad
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
	// Enviar novedad por outport indicando la nueva cantidad de energía siendo generada
	auto produccionDeEnergiaActual = Real(this->energia_produciendo);
	sendOutput(msg.time(), out, produccionDeEnergiaActual);
	return *this ;
}
