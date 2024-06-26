/*
 * config_i2c.c
 *
 *  Created on: 23 abr. 2024
 *      Author: USUARIO
 */
/*
 * config_i2c.c
 *
 *  Created on: 23 abr. 2024
 *      Author: USUARIO
 */
#include <msp430.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>


uint8_t *PTxData; // Pointer to TX data
uint8_t TXByteCtr;
uint8_t *PRxData; // Pointer to RX data
uint8_t RXByteCtr;

void i2c_init()
{
P4SEL0 |= BIT7 + BIT6;  // P4.6 SDA i P4.7 SCL com a USCI si fem server USCI B1
//P1SEL0 |= BIT3 + BIT2;  // P1.2 SDA i P1.3 SCL com a USCI si fem server USCI B0
UCB1CTLW0 |= UCSWRST; // Aturem el m�dul
//El configurem com a master, s�ncron i mode i2c, per defecte, est� en single-master mode
UCB1CTLW0 |= UCMST + UCMODE_3 + UCSSEL_2; // Use SMCLK,
UCB1BR0 = 160; // fSCL = SMCLK(16MHz)/160 = ~100kHz
UCB1BR1 = 0;
UCB1CTLW0 &= ~UCSWRST; // Clear SW reset, resume operation
UCB1IE |= UCTXIE0 | UCRXIE0; // Habilita les interrupcions a TX i RX
}





void I2C_send(uint8_t addr, uint8_t *buffer, uint8_t n_dades)
{
UCB1I2CSA = addr; //Coloquem l�adre�a de slave
PTxData = buffer; //adre�a del bloc de dades a transmetre
TXByteCtr = n_dades; //carreguem el n�mero de dades a transmetre;
UCB1CTLW0 |= UCTR + UCTXSTT; //I2C en mode TX, enviem la condici� de start
__bis_SR_register(LPM0_bits + GIE); //Entrem a mode LPM0, enable interrupts
__no_operation(); //Resta en mode LPM0 fins que es trasmetin les dades
while (UCB1CTLW0 & UCTXSTP); //Ens assegurem que s'ha enviat la condici� de stop
}

void I2C_receive(uint8_t addr, uint8_t *buffer, uint8_t n_dades)
{
PRxData = buffer; //adre�a del buffer on ficarem les dades rebudes
RXByteCtr = n_dades; //carreguem el n�mero de dades a rebre
UCB1I2CSA = addr; //Coloquem l�adre�a de slave
UCB1CTLW0 &= ~UCTR; //I2C en mode Recepci�
while (UCB1CTLW0 & UCTXSTP); //Ens assegurem que el bus est� en stop
UCB1CTLW0 |= UCTXSTT; //I2C start condition en recepci�
__bis_SR_register(LPM0_bits + GIE); //Entrem en mode LPM0, enable interrupts
__no_operation(); // Resta en mode LPM0 fins que es rebin totes les dades
}





#pragma vector = USCI_B1_VECTOR
__interrupt void ISR_USCI_I2C(void)
{
switch(__even_in_range(UCB1IV,12))
{
case USCI_NONE: break; // Vector 0: No interrupts
case USCI_I2C_UCALIFG: break; // Vector 2: ALIFG
case USCI_I2C_UCNACKIFG: break; // Vector 4: NACKIFG
case USCI_I2C_UCSTTIFG: break; // Vector 6: STTIFG
case USCI_I2C_UCSTPIFG: break; // Vector 8: STPIFG
case USCI_I2C_UCRXIFG0: // Vector 10: RXIFG
if (RXByteCtr)
{
*PRxData++ = UCB1RXBUF; // Mou la dada rebuda a l�adre�a PRxData
if (RXByteCtr == 1) // Queda nom�s una?
UCB1CTLW0 |= UCTXSTP; // Genera I2C stop condition
}
else {
*PRxData = UCB1RXBUF; // Mou la dada rebuda a l�adre�a PRxData
__bic_SR_register_on_exit(LPM0_bits); // Exit del mode baix consum LPM0, activa la CPU
}
RXByteCtr--; // Decrement RX byte counter
break;
case USCI_I2C_UCTXIFG0: // Vector 12: TXIFG
if (TXByteCtr) // Check TX byte counter
{
UCB1TXBUF = *PTxData++; // Carrega el TX buffer amb la dada a enviar
TXByteCtr--; // Decrementa TX byte counter
}
else
{
UCB1CTLW0 |= UCTXSTP; // I2C stop condition
UCB1IFG &= ~UCTXIFG; // Clear USCI_B1 TX int flag
__bic_SR_register_on_exit(LPM0_bits); // Exit del mode baix consum LPM0, activa la CPU
}
default: break;
}
}
