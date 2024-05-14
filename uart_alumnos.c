/*
 * uart.c
 *
 *  Created on:
 *      Author:
 */
#include <msp430.h>
#include <stdint.h>
#include "uart_alumnos.h"
void init_uart_wifi(void) //UART configuration: UCA0, 115200bps INICIALITZACIO DE LA UART
{


    UCA1CTLW0&=~UCSWRST_1;
    UCA1CTLW0|=UCSSEL_3; //DEFINEIX EL CLOCK DE REFERENCIA
    //CONFIGURACIO DELS BAUDIOS
    UCA1BRW=8;   //n_baud =SMCLK/baudrate
    UCA1MCTLW|=UCOS16; //OVERSAMPLING MODE
    UCA1MCTLW |= (10 << 4); // Justifiqueu aquesta instrucció
    UCA1MCTLW |= (0xF7 << 8); // Justifiqueu aquesta instrucció


    UCA1CTLW0|=UCSWRST_1;
    //HABILITACIÓ INTERRUPCIÓ
    UCA1IE|=UCTXIE|UCRXIE;  //ACEPTA UN NOU CARACTER ESCRIURE //EL VALOR ESCRIT ES ACCEPTABLE
    P4SEL0 |= BIT2 | BIT3; // I/O funció: P1.7 = UART_TX, P1.6 = UART_RX
    P4SEL1 &= ~(BIT2 | BIT3); // I/O funció: P1.7 = UART_TX, P1.6 = UART_RX
    UCA1IFG &= ~UCRXIFGE; // Per si de cas, esborrem qualsevol activació d’interrupció fins ara






}


/*
RxPacket() read data from UART buffer (received from Wifi module).
RxPacket() need a timeout parameter, in order to abort if no response.
RxPacket() return struct containing Received Packet.
 */
RxReturn RxPacket(uint32_t time_out) //
{

}

/*
TxPacket() send data to Wifi module (UART).
TxPacket() needs 2 parameters:
     Length of parameters (number of characters to be sent)
     Pointer to parameters to send.
 */
uint8_t TxPacket(uint8_t bParameterLength, const uint8_t *Parameters){

}

//interrupcion de recepcion en la uart UCA0:
#pragma vector = EUSCI_A0_VECTOR
__interrupt void EUSCIA0_IRQHandler(void)
{

}
