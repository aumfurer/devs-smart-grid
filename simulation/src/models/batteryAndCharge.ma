[top]

components : bateria@bateria celda_solar@celda_solar windTurbine@generador_eolico controlador@controller carga@carga

out : grid_out

in : radiation wind_speed power_consumption

link : radiation solar_change@celda_solar
link : wind_speed wind_change@windTurbine
link : power_consumption power_consumption@carga

link : out@celda_solar solar_panel@bateria 
link : out@windTurbine wind_turbine@bateria 
link : battery_demand@controlador required_energy@bateria 

link : battery_state@bateria battery_state@controladorw
link : out@carga load_demand@controlador
link : grid_demand@controlador grid_out

link : power_consumption required_energy@bateria

link : grid_demand@controlador grid_out
