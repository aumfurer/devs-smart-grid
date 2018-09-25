#include <random>
#include <string>
#include <cassert>
#include <cmath>

#include "message.h"
#include "parsimu.h"
#include "real.h"
#include "tuple_value.h"

#include "bateria.h"

using namespace std;


Bateria::Bateria(const string &name) :
	Atomic(name),

	energy_in(addInputPort("energy_in")),
	required_energy(addInputPort("required_energy")),
	battery_state(addOutputPort("battery_state")),

	energy_from_generators(0),
    energy_sending(0),
    charge(0),
    last_update(0),
	next_state(Bateria::AVAILABLE)
{
}

Model &Bateria::initFunction()
{
	holdIn(AtomicState::active, VTime::Inf);
	return *this;
}


Model &Bateria::externalFunction(const ExternalMessage &msg)
{
	this->update_current_charge(msg.time());
	double value = Real::from_value(msg.value()).value();
	if(msg.port() == this->energy_in){
		this->energy_from_generators += value;
	} else if(msg.port() == this->required_energy){
		this->energy_sending = this->next_state == Bateria::AVAILABLE ? 0: value;
	}
	this->update_next_event();
	return *this;
}


Model &Bateria::internalFunction(const InternalMessage &msg)
{
	VTime update_time = msg.time();

	if (this->next_state == Bateria::EMPTY){
		// Se termino la bateria
		this->energy_sending = 0;
		this->charge = 0;
		if(this->energy_from_generators > 0){
			nextChange(to_VTime(Bateria::MIN_CAPACITY / this->energy_from_generators));			 
		} else {
			nextChange(VTime::Inf);
		}
	} else if (this->next_state == Bateria::MIN_CAPACITY){
		// Ya esta disponible la bateria: recalculo el tiempo para que se llene
		this->next_state = Bateria::FULL;
		const float delta = this->energy_from_generators - this->energy_sending;
		if (delta > 0){
			const float time_to_full =  (Bateria::CAPACITY - this->charge) / delta;
			nextChange(to_VTime(time_to_full));
		} else {
			nextChange(VTime::Inf);
		}
	} else {
		// Se lleno la bateria (no hago nada?)
		assert(this->energy_from_generators > this->energy_sending);
		nextChange(VTime::Inf);
	}
	return *this;
}


Model &Bateria::outputFunction(const CollectMessage &msg)
{
	// estoy aca porque ocurrio un evento
	sendOutput(msg.time(), this->battery_state, Real(this->next_state));
	
	return *this ;
}

void Bateria::update_current_charge(const VTime &update_time)
{
	const float delta = this->energy_from_generators - this->energy_sending;
	this->charge += delta * (update_time - this->last_update).asMsecs();
	this->last_update = update_time;
}

void Bateria::update_next_event()
{
	const float delta = this->energy_from_generators - this->energy_sending;
	if(delta == 0){
		nextChange(VTime::Inf); 
	} else if (delta < 0){
		const float remaining_hours = this->charge / -delta;
		this->next_state = Bateria::EMPTY;
		nextChange(to_VTime(remaining_hours));
	} else if (this->charge < Bateria::MIN_CAPACITY){
		const float time_to_availability = (Bateria::MIN_CAPACITY - this->charge) / delta;
		this->next_state = Bateria::AVAILABLE;
		nextChange(to_VTime(time_to_availability));
	} else {
		const float time_to_full =  (Bateria::CAPACITY - this->charge) / delta;
		this->next_state = Bateria::FULL;
		nextChange(to_VTime(time_to_full));
	}
}

VTime Bateria::to_VTime(double v){
	return VTime((int)ceil(60 * 60 * 1000 * v));
}