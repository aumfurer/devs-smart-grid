[top]
components : bateria@bateria celda_solar@celda_solar windTurbine@generador_eolico controlador@controller

out : grid_out controlBatteryState controlBatteryDemand solarPowerBeingGenerated windPowerBeingGenerated grid_sell

in : radiation wind_speed power_consumption

link : radiation solar_change@celda_solar
link : power_consumption load_demand@controlador

link : out@celda_solar solar_panel@bateria 
link : battery_demand@controlador required_energy@bateria 

link : battery_state@bateria battery_state@controlador
link : surplus_energy@bateria battery_surplus_energy@controlador
link : selling_energy@controlador grid_sell

link : grid_demand@controlador grid_out
link : out@celda_solar solarPowerBeingGenerated
link : out@windTurbine windPowerBeingGenerated 
link : battery_demand@controlador controlBatteryDemand 
link : battery_state@bateria controlBatteryState
