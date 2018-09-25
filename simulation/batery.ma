[top]

components : bateria@bateria celda_solar@celda_solar generador_eolico@generador_eolico

out : out_port

in : radiation wind_speed power_consumption

link : radiation solar_change@celda_solar
link : wind_speed wind_change@generador_eolico

link : out@celda_solar energy_in@bateria 
link : out@generador_eolico energy_in@bateria 

link : battery_state@bateria out_port
