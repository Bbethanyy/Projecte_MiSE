#include <msp430.h> 
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>


#include "clock.h" //LLIBRERIA QUE CONFIGURA EL RELLOTGE


#include "config_i2c.h" //LIBRERIA QUE CONFIGURA LA COMUNICACION I2C
/**
 * main.c
 */

//VARIABLES

//MOTOR
uint8_t Array_motor[5];

//LCD
uint8_t  Array_LCD[8];
char  test_LCD[17];
uint8_t longitud;

//LEDS RGB

uint8_t Array_led[2];


//TIMER
uint32_t captura_temporitzador;
uint8_t  capture_done;
uint32_t contador_ms;



void init_GPIO(void){

    //TEST PORT 2 //P2.1/P2.2/P2.3/P2.4/P2.5

    P2SEL0 &=~0X04;
    P2SEL1 &=~0X04;
    P2DIR |=0X04;
    P2OUT  =0X00;

    //INICIALIZACION GPIOS

    //PORT P3 //P3.0//P3.3//P3.4//P3.5-> INPUTS  JOYSTICKS
              ////P3.2 ->OUTPUT HABILITACIÓ JOYSTICK

//    P3SEL0 &=~0X3D; // > MASCARA (0011 1101)
//    P3SEL1 &=~0X3D;// > MASCARA (0011 1101)  (1100 0010)
//    P3DIR  |=0X04;// "1" OUTPUT I "0" INPUT (XX00 01X0) -> 1100 0110
//    P3REN  |=0X39; //HABILITACIÓ PULL "1" O "0" NO
//    P3OUT  |=0X3D; // "1" PULL UP //"0" PULL DOWN
//    P3IE   |=0X39; //MASCARA (0011 1001)
//    P3OUT=0X00;
//    P3OUT  |=0X3D;






    //PUERTO 5 P5.2 (RESET de DISPLAY)

    P5SEL0 &=~0X04;
    P5SEL1 &=~0X04;
    P5DIR  |=0X04;
    P5OUT  =0X00;



}


//TIMERS CONFIGURACIÓ

void init_timer(){ //timer de 1ms


//INICIALITZACIÓ TIMER TB0
TB0CTL|=TBSSEL_2; //S'agafa el rellotge SMCLK 16MHZ
TB0CCTL0|=CCIE_1; //Habilita la interupcio CCIE

TB0CCR0=16000;//CONFIGURA A 1MS

//INICIALITZACIÓ TIMER TB1

//PIN FUNCION P6.0-> TB3.1 CCIA

P6SEL0 |= 0X01;
P6SEL1 &= ~0X01;
P6DIR  &= ~0X01;


TB3CTL|=TBSSEL_2;
TB3CCTL1|=CCIE_1|CAP_1|SCS_1; //S'OBSERVA EL VALOR DEL BIT DE CAPTURA
capture_done=0;


}

void captura_timer(uint32_t timer ){
    capture_done=0;
    TB3CTL|=MC_1;
    TB3CCTL1|=CM_1;

    while(capture_done==0){

    }
    timer=TB3CCR1;
    TB3CTL|=~MC_1;
}


void delay(uint32_t temps){

    contador_ms=0;

    TB0CTL|=MC_1;     //S'habilita mode UP

    while(temps>contador_ms){

    }
    TB0CTL&=~MC_1; //PARA EL COMPTADOR
}

//CONFIGURACIO_LCD

void init_LCD(){

    P5OUT&=~0X04;
    delay(10);
    P5OUT|=0x04;
    delay(10);




    Array_LCD[0] = 0x00;
    Array_LCD[1] = 0x39;
    Array_LCD[2] = 0x14;
    Array_LCD[3] = 0x74;
    Array_LCD[4] = 0x54;
    Array_LCD[5] = 0x6F;
    Array_LCD[6] = 0x0C;
    Array_LCD[7] = 0x01;



    I2C_send(0x3E,Array_LCD,8);
    delay(10);

}



// ROBOT

//LEDS

leds_RGB(uint8_t izq_led,uint8_t der_led){
    Array_led[0]=0x0B;
    Array_led[1]=izq_led;
    Array_led[2]=der_led;
    I2C_send(0x10,Array_led,3);
    delay(10);
}

//MOTOR

mov_motor(uint8_t sentit_1,uint8_t velocitat_1,uint8_t sentit_2,uint8_t velocitat_2){

      Array_motor[0]=0x00;
      Array_motor[1]=sentit_1;
      Array_motor[2]=velocitat_1;
      Array_motor[3]=sentit_2;
      Array_motor[4]=velocitat_2;
      I2C_send(0x10,Array_motor,5);
      delay(10);



}







void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    //CONFIGURACIO INICIAL
    init_clocks();
    init_timer();
    init_GPIO();
    i2c_init();


    __enable_interrupt();

       init_LCD();
       delay(10);
       longitud=sprintf(test_LCD,"@VELOCITAT \n ");
       I2C_send(0x3E,test_LCD,longitud);
       delay(10);
       longitud=sprintf(test_LCD,"@ VELOCIIITAT");
       delay(10);
       I2C_send(0x3E,test_LCD,longitud);
       delay(10);

    //TEST ROBOT
    while(1){

    }

    leds_RGB(1,1);
    mov_motor(0x01,0x20,0x01,0x20);
    delay(2000);
    leds_RGB(2,2);
    mov_motor(0x02,0x20,0x02,0x20);
    delay(2000);
    leds_RGB(3,3);
    mov_motor(0x01,0x20,0x00,0x00);
    delay(2000);
    leds_RGB(4,4);
    mov_motor(0x00,0x00,0x01,0x20);
    delay(5000);
    leds_RGB(0,0);
    mov_motor(0x00,0x00,0x01,0x00);

    delay(100);


//    captura_timer(captura_temporitzador );
    while(1){


    }





    //CONFIGURACIO LED LCR



}



#pragma vector=TIMER0_B0_VECTOR //Aquest és el nom important
__interrupt void PORT0_ISR (void)
{

    contador_ms=contador_ms+1;
    TB0CCTL0&=~CCIFG;



/* El que volem fer a la rutina d’atenció d’Interrupció */
}

#pragma vector=TIMER3_B1_VECTOR //Aquest és el nom important
__interrupt void PORT3_ISR (void)
{

    capture_done=1;
    TB3CCTL1&=~CCIFG;



/* El que volem fer a la rutina d’atenció d’Interrupció */
}




