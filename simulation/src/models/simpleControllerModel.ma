[top]
components : bateria@bateria celda_solar@celda_solar windTurbine@generador_eolico controlador@controller

out : grid_out controlBatteryState controlBatteryDemand

in : radiation wind_speed power_consumption

link : radiation solar_change@celda_solar
link : wind_speed wind_change@windTurbine
link : power_consumption load_demand@controlador

link : out@celda_solar solar_panel@bateria 
link : out@windTurbine wind_turbine@bateria 
link : battery_demand@controlador required_energy@bateria 

link : battery_state@bateria battery_state@controlador
link : grid_demand@controlador grid_out

link : battery_demand@controlador controlBatteryDemand 
link : battery_state@bateria controlBatteryState