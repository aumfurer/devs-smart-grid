[top]

components : bateria@bateria celda_solar@celda_solar

out : out_port

in : radiation 

link : radiation solar_change@celda_solar

link : out@celda_solar solar_panel@bateria 

link : battery_state@bateria out_port
