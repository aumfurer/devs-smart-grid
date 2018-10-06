[top]
components : bateria@bateria celda_solar@celda_solar windTurbine@generador_eolico controlador@controller

out : grid_out control_battery_demand

in : power_consumption battery_state

link : power_consumption load_demand@controlador
link : battery_state battery_state@controlador

link : battery_demand@controlador control_battery_demand 
link : grid_demand@controlador grid_out