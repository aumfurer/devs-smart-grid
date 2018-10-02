[top]

components : bateria@bateria celda_solar@celda_solar windTurbine@generador_eolico

out : out_port

in : radiation wind_speed power_consumption

link : radiation solar_change@celda_solar
link : wind_speed wind_change@windTurbine

link : out@celda_solar solar_panel@bateria 
link : out@windTurbine wind_turbine@bateria 
link : power_consumption required_energy@bateria

link : battery_state@bateria out_port
