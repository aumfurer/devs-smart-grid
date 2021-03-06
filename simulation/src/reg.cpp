#include "pmodeladm.h"
#include "register.h"

// TODO: cambiar este include por el header del modelo atómico
// que se desee utilizar.
// #include "mi_header.h"
#include "celda_solar.h"
#include "generador_eolico.h"
#include "battery.h"
#include "controller.h"
#include "carga.h"


void register_atomics_on(ParallelModelAdmin &admin)
{
	// TODO: cambiar 'MiModelo' por el nombre de la clase que representa
	// el modelo atómico y "Nombre del modelo" por el string utilizado en
	// dicha clase como valor de retorno del método className().
	
	admin.registerAtomic(NewAtomicFunction<CeldaSolar>(), CELDA_SOLAR_NAME);
	admin.registerAtomic(NewAtomicFunction<GeneradorEolico>(), GENERADOR_EOLICO_NAME);

	// New battery model
	admin.registerAtomic(NewAtomicFunction<Battery>(), BATTERY_NAME);

	admin.registerAtomic(NewAtomicFunction<Controller>(), CONTROLLER_NAME);
	admin.registerAtomic(NewAtomicFunction<Carga>(), CARGA_NAME);

}
