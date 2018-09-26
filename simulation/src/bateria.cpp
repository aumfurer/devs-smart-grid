#include <random>
#include <string>
#include <cassert>
#include <cmath>

#include "message.h"
#include "parsimu.h"
#include "real.h"
#include "tuple_value.h"

#include "bateria.h"

#define eps 1e-5

using namespace std;


Bateria::Bateria(const string &name) :
	Atomic(name),

	energy_in(addInputPort("energy_in")),
	required_energy(addInputPort("required_energy")),
	battery_state(addOutputPort("battery_state")),

	energy_from_generators(0),
    energy_sending(0),
    charge(0),
    last_update(0)
{
}

Model &Bateria::initFunction()
{
	this->charge = 0;
	holdIn(AtomicState::active, VTime::Inf);
	return *this;
}


Model &Bateria::externalFunction(const ExternalMessage &msg)
{
	VTime update_time = msg.time();
	this->charge = this->new_current_charge(update_time);
	this->last_update = update_time;
	
	double value = Real::from_value(msg.value()).value();
	if(msg.port() == this->energy_in){
		this->energy_from_generators += value;
	} else if(msg.port() == this->required_energy){

		bool still_waiting_for_available = 
			this->energy_sending == 0 &&
			this->charge < Bateria::MIN_CAPACITY;
			
		this->energy_sending = still_waiting_for_available ? 0: value;
	}
	this->update_next_event();
	return *this;
}


Model &Bateria::internalFunction(const InternalMessage &msg)
{
	VTime update_time = msg.time();

	this->charge = this->new_current_charge(msg.time());
	this->last_update = update_time;
	

	if (this->charge < eps){
		// Se termino la bateria
		this->energy_sending = 0;
		this->charge = 0;
		if(this->energy_from_generators > 0){
			nextChange(to_VTime(Bateria::MIN_CAPACITY / this->energy_from_generators));			 
		} else {
			nextChange(VTime::Inf);
		}
	} else if (abs(this->charge - Bateria::MIN_CAPACITY) < eps){
		const double delta = this->energy_from_generators - this->energy_sending;
		if (delta > 0){
			const double time_to_full = (Bateria::CAPACITY - this->charge) / delta;
			nextChange(to_VTime(time_to_full));
		} else {
			nextChange(VTime::Inf);
		}
	} else {
		// Se lleno la bateria (no hago nada?)
		assert(this->energy_from_generators > this->energy_sending);
		assert(abs(this->charge - Bateria::CAPACITY) < eps);
		nextChange(VTime::Inf);
	}
	return *this;
}


Model &Bateria::outputFunction(const CollectMessage &msg)
{

	double new_charge = this->new_current_charge(msg.time());
	// estoy aca porque ocurrio un evento
	sendOutput(msg.time(), this->battery_state, 
		new_charge < eps ? Bateria::EMPTY :
		abs(new_charge - Bateria::MIN_CAPACITY) ? Bateria::AVAILABLE:
		Bateria::FULL
	);
	
	return *this ;
}

double Bateria::new_current_charge(const VTime &update_time)
{
	const double delta = this->energy_from_generators - this->energy_sending;
	double res = this->charge + delta * (update_time - this->last_update).asMsecs() / 1000;
	res = min(res, Bateria::CAPACITY);
	return res;
}

void Bateria::update_next_event()
{
	const double delta = this->energy_from_generators - this->energy_sending;
	if(delta == 0){
		nextChange(VTime::Inf); 
	} else if (delta < 0){
		const double remaining_seconds = this->charge / -delta;
		nextChange(to_VTime(remaining_seconds));
	} else if (this->charge < Bateria::MIN_CAPACITY){
		const double time_to_availability = (Bateria::MIN_CAPACITY - this->charge) / delta;
		nextChange(to_VTime(time_to_availability));
	} else {
		const double time_to_full =  (Bateria::CAPACITY - this->charge) / delta;
		nextChange(to_VTime(time_to_full));
	}
}

VTime Bateria::to_VTime(double v){
	VTime res = VTime(0,0,0,0, (float)(v)*1000);
	return res;
}