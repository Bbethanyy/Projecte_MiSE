
//LLIBRERIES QUE S'IMPORTEN
#include <msp430.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>


#include "clock.h"      //LLIBRERIA QUE CONFIGURA EL RELLOTGE SMCLK
#include "config_i2c.h" //LIBRERIA QUE CONFIGURA LA COMUNICACIÓ I2C
/**
 * main.c
 */

//VARIABLES

//MOTOR
uint8_t Array_motor[5]; //VARIABLE ENCARREGUADA DE ENVIA LES TRAMES I2C AL MOTOR

//LCD
uint8_t  Array_LCD[8];//VARIABLE ENCARREGUADA DE ENVIA LES TRAMES I2C PER CONFIGURAT LCD
char  test_LCD[18];   //CARACTERS A REPRESENTAR EN EL DISPLAY
uint8_t longitud;     //NUMERO DE CARACTERS QUE REPRESENTA EL DISPLAY

//LEDS RGB

uint8_t Array_led[2]; //VARIABLE ENCARREGUADA DE ENVIA LA TRAMA I2C DEL LCD


//TIMER
uint32_t captura_temporitzador;
uint8_t  capture_done;
uint32_t contador_ms;//NUMERO DE CICLES NECESSARIS PER GENERAR 1MS PER UN SMLCK 16MHZ

volatile uint8_t joystick_state = 0;


void init_GPIO(void){ //INICIALITZACIÓ DELS GPIOS

    //PUERTO 5 P5.2 (RESET de DISPLAY)

    P5SEL0 &= ~0X04;  //S'ESPECIFICA COM A GPIO EL PINT P5.2
    P5SEL1 &= ~0X04;  //S'ESPECIFICA COM A GPIO EL PINT P5.2
    P5DIR  |= 0X04;   //S'ESPECIFICA COM A OUTPUT
    P5OUT  =  0X00;    //LES SORTIDES S'ESPECIFIQUEN A 0

    //PORT P3 (P3.1 (BOTTOM)/P3.2(LEFT)/P3.3(TOP)/P3.4(RIGHT)/P3.5(BOTON)
    P3SEL0 &= ~0X3E;   //Especifica P3.1 a P3.5 com a GPIO
    P3SEL1 &= ~0X3E;
    P3DIR  &= ~0X3E;   //S'especifica com a input
    P3REN  |= 0X20;    //Habilitar resistencies pull up
    P3OUT  |= 0X20;    //pull up

    P3IE   |= 0X3E;    //Habilitar interruptors
    P3IES  |= 0x3E;
    P3IFG  &= ~0x3E;   //Creal interrupt flags


    //PORT P4 (4.4//4.5)
    P5SEL0 &=~0X30;   //S'ESPECIFICA COM A GPIO EL PINT P5.2
    P5SEL1 &=~0X30;//S'ESPECIFICA COM A GPIO EL PINT P5.2
    P5DIR  |=0X30; //S'ESPECIFICA COM A OUTPUT
    P5OUT  =0X00;  // LES SORTIDES S'ESPECIFIQUEN A 0

}


//TIMERS CONFIGURACIÓ

void init_timer(){ //timer de 1ms


//INICIALITZACIÓ TIMER TB0
    TB0CTL|=TBSSEL_2; //S'agafa el rellotge SMCLK 16MHZ
    TB0CCTL0|=CCIE_1; //Habilita la interupcio CCIE
    TB0CCR0=16000;    //CONFIGURA A 1mS CONVERSIO DE 16MHZ A 1MS ES EL NUMERO DE POLSOS PERQUE DURI 1ms

    //INICIALITZACIÓ TIMER TB1

    //PIN FUNCIó P6.0-> TB3.1 CCIA Capture

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


void delay(uint32_t temps){   //Genera el temps de espera

    contador_ms=0;            //inicialment s'inicialitza a 0
    TB0CTL|=MC_1;             //S'habilita mode UP

    while(temps>contador_ms){ //fins que s'hagui fet mes que "temps" de  interupcions surt del bucle
    }
    TB0CTL&=~MC_1;            //PARA EL COMPTADOR
}

//CONFIGURACIO_LCD

void init_LCD(){

    P5OUT&=~0X04; //Es fa un reset del display LCD
    delay(10); //Temps de espera
    P5OUT|=0x04; //S'habilita a 1 activant el display LCD
    delay(10); //temps de espera



//INICIALITZACIO DISPLAY LCD
    Array_LCD[0] = 0x00; //
    Array_LCD[1] = 0x39; //
    Array_LCD[2] = 0x14;//Es configura la freqüencia OSC
    Array_LCD[3] = 0x74;//Es configura el contrast
    Array_LCD[4] = 0x54;//POWER ICON
    Array_LCD[5] = 0x6F;//Follower controller
    Array_LCD[6] = 0x0C;//S'activa el display
    Array_LCD[7] = 0x01;//Neteja lo que tenia escrit el display



    I2C_send(0x3E,Array_LCD,8);// S'envia la trama al LCD indicant la adress slave del LCD
    delay(10);

}


// ROBOT

//LEDS

leds_RGB(uint8_t izq_led,uint8_t der_led){ //Modifica el color dels LEDS RGB
    Array_led[0]=0x0B; //S'especifica  del robot el leds RGB
    Array_led[1]=izq_led; //El color del led esquerre
    Array_led[2]=der_led; //El color del led dreta
    I2C_send(0x10,Array_led,3); //S'envia les dades
    delay(10);
}


//MOTOR

//MOTOR: controlar sentiti velocitat de les rodes
mov_motor(uint8_t sentit_1,uint8_t velocitat_1,uint8_t sentit_2,uint8_t velocitat_2){

      Array_motor[0]=0x00;                //Començament per al motor
      Array_motor[1]=sentit_1;            //Assignem el valor sentit_1 a la segona posició de l'array
      Array_motor[2]=velocitat_1;         //la seva velocitat
      Array_motor[3]=sentit_2;            //Sentit_2 per a la quarta posició de l'array
      Array_motor[4]=velocitat_2;         //la seva velocitat
      I2C_send(0x10,Array_motor,5);       //Envia les dades de l'array del motor
      delay(10);
}

void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    //CONFIGURACIO INICIAL
       init_clocks();                        //Inicialitza els clocks
       init_timer();                         //el temporitzador
       init_GPIO();                          //els pins GPIOs
       i2c_init();                           //i el BUS I2C

    __enable_interrupt();


     init_LCD();                        //Inicialitza LCD
          delay(10);
          longitud=sprintf(test_LCD,"@HOLA y \n ");  // la paraula velocitat que volem mostrar en la pantalla
          I2C_send(0x3E,test_LCD,longitud);             //s'envia aquesta dada
          delay(10);
          longitud=sprintf(test_LCD,"@ ADIOS"); //Es fa visualitzar en el display LCD el missatge escrit
          delay(10);
          I2C_send(0x3E,test_LCD,longitud);
          delay(10);

    leds_RGB(1,1);
    //TEST ROBOT


    while (1) {
        // Processar l'estat del joystick
              switch (joystick_state) {
                  case 1: // BOTTOM
                      mov_motor(0x01, 0x20, 0x01, 0x20);
                      joystick_state = 0; // Reinicia l'estat després de processar-lo
                      break;
                  case 2: // LEFT
                      mov_motor(0x02, 0x20, 0x01, 0x00);
                      joystick_state = 0;
                      break;
                  case 3: // TOP
                      longitud = sprintf(test_LCD, "@HOLA \n ");
                      I2C_send(0x3E, test_LCD, longitud);
                      delay(10);
                      mov_motor(0x01, 0x20, 0x02, 0x20);
                      joystick_state = 0;
                      break;
                  case 4: // RIGHT
                      mov_motor(0x01, 0x00, 0x02, 0x20);
                      joystick_state = 0;
                      break;
                  case 5: // BUTTON
                      mov_motor(0x00, 0x00, 0x00, 0x00);
                      joystick_state = 0;
                      break;
                  default: break;
              }

                // Motor and LED test sequence
                leds_RGB(1, 1);
                mov_motor(0x01, 0x20, 0x01, 0x20);
                delay(2000);
                leds_RGB(2, 2);
                mov_motor(0x02, 0x20, 0x02, 0x20);
                delay(2000);
                leds_RGB(3, 3);
                mov_motor(0x01, 0x20, 0x00, 0x00);
                delay(2000);
                leds_RGB(4, 4);
                mov_motor(0x00, 0x00, 0x01, 0x20);
                delay(5000);
                leds_RGB(0, 0);
                mov_motor(0x00, 0x00, 0x01, 0x00);
                delay(100);


}

//    captura_timer(captura_temporitzador );
//    while(1){
//
//
//    }
//
//



    //CONFIGURACIO LED LCR


// INTERUPCIONS PORTS GPIOS

}

//PORT 3

#pragma vector=PORT3_VECTOR //interrupcions dels botons i Joystick.
__interrupt void port3_ISR(void)
{ //PORT P3 (P3.1 (BOTTOM)/P3.2(LEFT)/P3.3(TOP)/P3.4(RIGHT)/P3.5(BOTON)

//P3IE&=~0X3E;

uint8_t vector_flag=P3IV;

switch(vector_flag){

    case 0x00:break;
    case 0x02:break;

    case 0x04:                      //P3.1 BOTTOM
        delay(10);
        mov_motor(0x01,0x20,0x01,0x20);
        break;

    case 0x06:                     //P3.2 LEFT
        delay(20);
        mov_motor(0x02,0x20,0x01,0x00);
        break;

    case 0x08:                    // P3.3 TOP

        delay(10);
        longitud=sprintf(test_LCD,"@HOLA \n ");       // la paraula velocitat que volem mostrar en la pantalla
        I2C_send(0x3E,test_LCD,longitud);             //s'envia aquesta dada
        delay(10);
        mov_motor(0x01,0x20,0x02,0x20);
        break;

    case 0x0A:                    //P3.4 RIGHT
        delay(20);
        mov_motor(0x01,0x00,0x02,0x20);
        break;

    case 0x0C:                   // P3.5 BOTON
    delay(20);
    mov_motor(0x00,0x00,0x00,0x00);
    break;

    default :break;

}
P3IE|=0X3E;
P3IFG=0X3E;




}
//#pragma vector=PORT3_VECTOR
//__interrupt void port3_ISR(void) {
//    uint8_t vector_flag = P3IV;
//
//    switch (vector_flag) {
//        case 0x00: break;
//        case 0x02: break; // No utilitzat
//        case 0x04: // P3.1 BOTTOM
//            joystick_state = 1;
//            break;
//        case 0x06: // P3.2 LEFT
//            joystick_state = 2;
//            break;
//        case 0x08: // P3.3 TOP
//            joystick_state = 3;
//            break;
//        case 0x0A: // P3.4 RIGHT
//            joystick_state = 4;
//            break;
//        case 0x0C: // P3.5 BUTTON
//            joystick_state = 5;
//            break;
//        default: break;
//    }
//
//    P3IFG &= ~0x3E; // Esborra els flags d'interrupció
//}

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
