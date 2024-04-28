/*
 * config_i2c.h
 *
 *  Created on: 23 abr. 2024
 *      Author: USUARIO
 */

#ifndef LIBRERIAS_ROBOT_CONFIG_I2C_H_
#define LIBRERIAS_ROBOT_CONFIG_I2C_H_

uint8_t *PTxData; // Pointer to TX data
uint8_t TXByteCtr;
uint8_t *PRxData; // Pointer to RX data
uint8_t RXByteCtr;
void i2c_init();
void I2C_send(uint8_t addr, uint8_t *buffer, uint8_t n_dades);
void I2C_receive(uint8_t addr, uint8_t *buffer, uint8_t n_dades);
#pragma vector = USCI_B0_VECTOR;

#endif /* LIBRERIAS_ROBOT_CONFIG_I2C_H_ */
