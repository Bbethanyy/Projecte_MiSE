#include <msp430.h> 
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>


/**
 * main.c
 */
uint32_t contador_ms;


void init_GPIO(void){

    //TEST PORT 1 P1.1-> ES COMPROVA EL FUNCIONAMENT DELS DELAY

    P1SEL0&=~0X04;
    P1SEL1&=~0X04;
    P1DIR&=0X04;
    P1OUT=0X00;




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

void i2c_init()
{
//P4SEL0 |= BIT7 + BIT6; * // P4.6 SDA i P4.7 SCL com a USCI si fem server USCI B1
//P1SEL0 |= BIT3 + BIT2; * // P1.2 SDA i P1.3 SCL com a USCI si fem server USCI B0
UCBxCTLW0 |= UCSWRST; // Aturem el mòdul
//El configurem com a master, síncron i mode i2c, per defecte, està en single-master mode
UCBxCTLW0 |= UCMST + UCMODE_3 + UCSSEL_2; // Use SMCLK,
UCBxBR0 = 160; // fSCL = SMCLK(16MHz)/160 = ~100kHz
UCBxBR1 = 0;
UCBxCTLW0 &= ~UCSWRST; // Clear SW reset, resume operation
UCBxIE |= UCTXIE0 | UCRXIE0; // Habilita les interrupcions a TX i RX
}





void init_timer(){ //timer de 1ms

TB0CTL|=TBSSEL_2; //S'agafa el rellotge SMCLK 16MHZ
TB0CCTL0|=CCIE_1; //Habilita la interupcio CCIE

TB0CCR0=16000;//CONFIGURA A 1MS

}

void delay(uint32_t temps){

    contador_ms=0;
    ;
    TB0CTL|=MC_1;     //S'habilita mode UP

    while(temps>contador_ms){

    }



    TB0CTL&=~MC_1; //PARA EL COMPTADOR








}





void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    //CONFIGURACIO INICIAL
    init_clocks();
    init_timer();
    init_GPIO();

    __enable_interrupt();

    while(1){
    P1OUT&=~0X02;
    delay(1000);
    P1OUT|=0X04;
    delay(1000);
    }


    }



#pragma vector=TIMER0_B0_VECTOR //Aquest és el nom important
__interrupt void PORT0_ISR (void)
{

    contador_ms=contador_ms+1;
    TB0CCTL0&=~CCIFG;



/* El que volem fer a la rutina d’atenció d’Interrupció */
}







