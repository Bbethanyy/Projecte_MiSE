#include <msp430.h> 
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>


/**
 * main.c
 */

uint8_t Array_motor[5];

uint8_t *motor;
uint8_t *poinrt;


uint8_t *PTxData; // Pointer to TX data
uint8_t TXByteCtr;
uint8_t *PRxData; // Pointer to RX data
uint8_t RXByteCtr;

uint32_t contador_ms;

void init_GPIO(void){

    //TEST PORT 1 P1.1-> ES COMPROVA EL FUNCIONAMENT DELS DELAY

    P2SEL0 &=~0X04;
    P2SEL1 &=~0X04;
    P2DIR |=0X04;
    P2OUT  =0X00;
}


void init_clocks(void)
{ // Configure one FRAM waitstate as required by the device datasheet for MCLK operation beyond 8MHz before configuring the clock system
    FRCTL0 = FRCTLPW | NWAITS_1;
    P2SEL1 |= BIT6 | BIT7; // P2.6~P2.7: crystal pins
    do
    {
        CSCTL7 &= ~(XT1OFFG | DCOFFG); // Clear XT1 and DCO fault flag
        SFRIFG1 &= ~OFIFG;
    }
while (SFRIFG1 & OFIFG); // Test oscillator fault flag
__bis_SR_register(SCG0); // disable FLL
CSCTL3 |= SELREF__XT1CLK; // Set XT1 as FLL reference source
//CSCTL1 = DCOFTRIMEN_1 | DCOFTRIM0 | DCOFTRIM1 | DCORSEL_5; // DCOFTRIM=5, DCO Range = 16MHz**
CSCTL1 = DCORSEL_5; // DCOFTRIM=5, DCO Range = 16MHz
CSCTL2 = FLLD_0 + 487; // DCOCLKDIV = 16MHz
__delay_cycles(3);
__bic_SR_register(SCG0); // enable FLL
//Software_Trim(); // Software Trim to get the best DCOFTRIM value**
CSCTL4 = SELMS__DCOCLKDIV | SELA__XT1CLK; // set XT1 (~32768Hz) as ACLK source, ACLK = 32768Hz
// default DCOCLKDIV as MCLK and SMCLK source
 //P1DIR |= BIT0 | BIT1; // set SMCLK, ACLK pin as output
 //P1SEL1 |= BIT0 | BIT1; // set SMCLK and ACLK pin as second function
PM5CTL0 &= ~LOCKLPM5; // Disable the GPIO power-on default high-impedance mode to activate previously configured port settings
}




void init_timer(){ //timer de 1ms

TB0CTL|=TBSSEL_2; //S'agafa el rellotge SMCLK 16MHZ
TB0CCTL0|=CCIE_1; //Habilita la interupcio CCIE

TB0CCR0=16000;//CONFIGURA A 1MS

}

void delay(uint32_t temps){

    contador_ms=0;

    TB0CTL|=MC_1;     //S'habilita mode UP

    while(temps>contador_ms){

    }
    TB0CTL&=~MC_1; //PARA EL COMPTADOR
}



void i2c_init()
{
//P4SEL0 |= BIT7 + BIT6; * // P4.6 SDA i P4.7 SCL com a USCI si fem server USCI B1
P1SEL0 |= BIT3 + BIT2;  // P1.2 SDA i P1.3 SCL com a USCI si fem server USCI B0
UCB0CTLW0 |= UCSWRST; // Aturem el mòdul
//El configurem com a master, síncron i mode i2c, per defecte, està en single-master mode
UCB0CTLW0 |= UCMST + UCMODE_3 + UCSSEL_2; // Use SMCLK,
UCB0BR0 = 160; // fSCL = SMCLK(16MHz)/160 = ~100kHz
UCB0BR1 = 0;
UCB0CTLW0 &= ~UCSWRST; // Clear SW reset, resume operation
UCB0IE |= UCTXIE0 | UCRXIE0; // Habilita les interrupcions a TX i RX
}





void I2C_send(uint8_t addr, uint8_t *buffer, uint8_t n_dades)
{
UCB0I2CSA = addr; //Coloquem l’adreça de slave
PTxData = buffer; //adreça del bloc de dades a transmetre
TXByteCtr = n_dades; //carreguem el número de dades a transmetre;
UCB0CTLW0 |= UCTR + UCTXSTT; //I2C en mode TX, enviem la condició de start
__bis_SR_register(LPM0_bits + GIE); //Entrem a mode LPM0, enable interrupts
__no_operation(); //Resta en mode LPM0 fins que es trasmetin les dades
while (UCB0CTLW0 & UCTXSTP); //Ens assegurem que s'ha enviat la condició de stop
}

void I2C_receive(uint8_t addr, uint8_t *buffer, uint8_t n_dades)
{
PRxData = buffer; //adreça del buffer on ficarem les dades rebudes
RXByteCtr = n_dades; //carreguem el número de dades a rebre
UCB0I2CSA = addr; //Coloquem l’adreça de slave
UCB0CTLW0 &= ~UCTR; //I2C en mode Recepció
while (UCB0CTLW0 & UCTXSTP); //Ens assegurem que el bus està en stop
UCB0CTLW0 |= UCTXSTT; //I2C start condition en recepció
__bis_SR_register(LPM0_bits + GIE); //Entrem en mode LPM0, enable interrupts
__no_operation(); // Resta en mode LPM0 fins que es rebin totes les dades
}


void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    //CONFIGURACIO INICIAL
    init_clocks();
    init_timer();
    init_GPIO();
    i2c_init();






    Array_motor[0]=0x00;
    Array_motor[1]=0x02;
    Array_motor[2]=50;
    Array_motor[3]=0x01;
    Array_motor[4]=20;








    __enable_interrupt();





    I2C_send(0x10,Array_motor,5);



    }



#pragma vector=TIMER0_B0_VECTOR //Aquest és el nom important
__interrupt void PORT0_ISR (void)
{

    contador_ms=contador_ms+1;
    TB0CCTL0&=~CCIFG;



/* El que volem fer a la rutina d’atenció d’Interrupció */
}

#pragma vector = USCI_B0_VECTOR
__interrupt void ISR_USCI_I2C(void)
{
switch(__even_in_range(UCB0IV,12))
{
case USCI_NONE: break; // Vector 0: No interrupts
case USCI_I2C_UCALIFG: break; // Vector 2: ALIFG
case USCI_I2C_UCNACKIFG: break; // Vector 4: NACKIFG
case USCI_I2C_UCSTTIFG: break; // Vector 6: STTIFG
case USCI_I2C_UCSTPIFG: break; // Vector 8: STPIFG
case USCI_I2C_UCRXIFG0: // Vector 10: RXIFG
if (RXByteCtr)
{
*PRxData++ = UCB0RXBUF; // Mou la dada rebuda a l’adreça PRxData
if (RXByteCtr == 1) // Queda només una?
UCB0CTLW0 |= UCTXSTP; // Genera I2C stop condition
}
else {
*PRxData = UCB0RXBUF; // Mou la dada rebuda a l’adreça PRxData
__bic_SR_register_on_exit(LPM0_bits); // Exit del mode baix consum LPM0, activa la CPU
}
RXByteCtr--; // Decrement RX byte counter
break;
case USCI_I2C_UCTXIFG0: // Vector 12: TXIFG
if (TXByteCtr) // Check TX byte counter
{
UCB0TXBUF = *PTxData++; // Carrega el TX buffer amb la dada a enviar
TXByteCtr--; // Decrementa TX byte counter
}
else
{
UCB0CTLW0 |= UCTXSTP; // I2C stop condition
UCB0IFG &= ~UCTXIFG; // Clear USCI_B1 TX int flag
__bic_SR_register_on_exit(LPM0_bits); // Exit del mode baix consum LPM0, activa la CPU
}
default: break;
}
}





