#include <random>
#include <string>
#include <cassert>
#include <cmath>
#include <iostream>

#include "message.h"
#include "parsimu.h"
#include "real.h"
#include "tuple_value.h"

#include "bateria.h"

#define eps 1e-7
#define eps_charge 1

using namespace std;

#define dbg(x) cout << #x << " is " << (x) << endl


double Bateria::CAPACITY = 500.0;
double Bateria::MAXIMUM_POWER = 500.0;
double Bateria::MIN_CAPACITY = 100.0;

Bateria::Bateria(const string &name) :
	Atomic(name),

	// Input ports
	windTurbineEnergyIn(addInputPort("wind_turbine")),
	solarPanelEnergyIn(addInputPort("solar_panel")),
	required_energy(addInputPort("required_energy")),

	// Output ports
	battery_state(addOutputPort("battery_state")),


    solarPanelPower(0),
    windTurbinePower(0),
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
	if(msg.port() == this->solarPanelEnergyIn){
		// Message comes from generators
		// Changed this to take just the new amount of power being generated, instead of delta
		// Easier to interpret in messages later
		this->solarPanelPower = value;
		this->updateGeneratedPower();
	} else if(msg.port() == this->windTurbineEnergyIn){
		// Message comes from generators
		// Changed this to take just the new amount of power being generated, instead of delta
		// Easier to interpret in messages later
		this->windTurbinePower = value;
		this->updateGeneratedPower();
	} else if(msg.port() == this->required_energy){
		// Message comes from controller

		bool still_waiting_for_available = 
			this->energy_sending == 0 &&
			this->charge < Bateria::MIN_CAPACITY;
			
		// Required energy received from controller is converted into Watts/ms
		this->energy_sending = still_waiting_for_available ? 0: value / (3600 * 1000);
	}
	this->update_next_event();
	cout << msg.time() << " " << this->charge << " (ext)" << endl;
	return *this;
}

Model &Bateria::internalFunction(const InternalMessage &msg)
{
	VTime update_time = msg.time();

	this->charge = this->new_current_charge(msg.time());
	this->last_update = update_time;

	if (this->charge < eps_charge){
		// Se termino la bateria
		this->energy_sending = 0;
		this->charge = 0;
		if(this->energy_from_generators > 0){
			nextChange(to_VTime(Bateria::MIN_CAPACITY / this->energy_from_generators));			 
		} else {
			nextChange(VTime::Inf);
		}
	} else if (abs(this->charge - Bateria::MIN_CAPACITY) < eps_charge){
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
		assert(abs(this->charge - Bateria::CAPACITY) < eps_charge);
		nextChange(VTime::Inf);
	}

	cout << msg.time() << " " << this->charge << " (int)" << endl;
	return *this;
}


Model &Bateria::outputFunction(const CollectMessage &msg)
{

	double new_charge = this->new_current_charge(msg.time());
	// estoy aca porque ocurrio un evento
	sendOutput(msg.time(), this->battery_state, 
		new_charge < Bateria::MIN_CAPACITY ? Bateria::EMPTY :
		new_charge >= Bateria::MIN_CAPACITY && new_charge < this->CAPACITY ? Bateria::AVAILABLE:
		Bateria::FULL
	);
	
	return *this ;
}

double Bateria::new_current_charge(const VTime &update_time)
{
	const double chargingRate = this->energy_from_generators - this->energy_sending;
	// Convert time difference back to hours
	// Refer to https://answers.energysage.com/question/102/if-a-solar-panel-is-rated-at-300w-how-much-power-will-it-produce/
	double timeSinceLastUpdate = (update_time - this->last_update).asMsecs();
	double res = this->charge + chargingRate * timeSinceLastUpdate;

	// TODO: Considerar tomar el max(0, posible valor en el que se consume mucho, y qued negativo res)
	// RES: no deberia pasar nunca si no hubieran errores de precision
	res = max(0.0, min(res, Bateria::CAPACITY));
	return res;
}

void Bateria::update_next_event()
{
	const double delta = this->energy_from_generators - this->energy_sending;
	// Maybe use delta < EPSILON, for a little number EPSILON
	dbg(delta);
	if (abs(delta) < eps) {
		passivate();
	} else if (delta < -eps) {
		// Consuming more than the energy being generated, battery discharging
		const double remaining_seconds = this->charge / -delta;
		nextChange(to_VTime(remaining_seconds));
	} else {
		// delta > eps
		if (this->charge < Bateria::MIN_CAPACITY) {
			// To to be able to provide power to controller
			const double time_to_availability = (Bateria::MIN_CAPACITY - this->charge) / delta;
			nextChange(to_VTime(time_to_availability));
		} else if (abs(this->charge - Bateria::CAPACITY) < eps) {
			// Full charge and still charging
			passivate();
		} else {
			// Charging. delta > 0
			// TODO: Consider case in which te battery is fully charged
			const double time_to_full =  (Bateria::CAPACITY - this->charge) / delta;
			nextChange(to_VTime(time_to_full));
		}
	}
}

/*
	Converts a value of v hours, into a VTime struct.
*/
VTime Bateria::to_VTime(double v){
	// The last parameter in VTime constructor is miliSeconds, and since
	// power over time mesasures are in hours, should use it instead of secs.
	// 3600 sec == 1 hour
	// float hoursIntoMiliseconds = (float) v * 3600.0f * 1000.0f;
	// return VTime(0,0,0,0, hoursIntoMiliseconds);
	return VTime(0,0,0,0, (float)v);
}

// Updates the energy being generated towards the battery, in a rate of Watts over miliseconds.
void Bateria::updateGeneratedPower() {
	this->energy_from_generators = (this->solarPanelPower + this->windTurbinePower) / (1000 * 3600);
}