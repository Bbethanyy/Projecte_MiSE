#include <msp430.h> 
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>


/**
 * main.c
 */


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


void init_timer(){

TB0CTL|=TBSSEL_2; //S'agafa el rellotge SMCLK

TB0CCTL0|=CCIS_3; //rising
TB0CCTL0|=CAP_1;
TB0CCTL0|=CCIE_1;

}

void delay(uint32_t temps){

    volatile uint32_t temporitzador;
    temporitzador=16000*temps;
    TB0CTL|=MC_1;     //S'habilita mode UP
    TB0CCR0=temporitzador;

    while(1){
        if ()
    }
    TB0CTL|=MC_1;




}





void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    init_timer();
    while(1){
        init_clocks();
    }







}









