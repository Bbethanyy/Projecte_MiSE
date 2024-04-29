#!/usr/bin/env python
# coding: utf-8

# 
# |Slave adress | Dispositiu | D1 | D2 | D3 | D4 |
# |-------------|------------|----|----|----|----|
# |0X10|MOTOR 0X00|DIRECCIÓ LEFT(0,1,2)|VELOCITAT LEFT|DIRECCIÓ LEFT |
# |0X10|LED RGB 0X0B|
# 
# 
# 
# 
# |NOM FUNCIÓ|LLIBRERIA|FUNCIONALITAT|INPUTS|
# |-------|----------|-------------|------|
# |init_clocks|clock.h|Generació del SMCLK|void|
# |I2c_init|config_i2c.h|Inicialització del I2C|void|
# |I2c_send|config_i2c.h|Genera Transferencia d'escriptura|(adreça del slave,array de dades,numero de dades)|
# |I2C_receive|config_I2c.h|Genera transferencia de lectura|(adreça del slave,array de dades d electura,numero de dades)|
# 
# 
