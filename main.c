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
uint8_t a;


//ADC
uint16_t conversion_complete;
uint16_t resultat_ADC;
uint16_t resultat_ADC_promitg;
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
uint16_t timer_capture;
uint32_t contador_us;
uint16_t ultrasound;


//GPIO

uint8_t counter;
uint8_t inputGP;
uint8_t P3_Input;
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
    P3REN  |= 0X3E;    //Habilitar resistencies pull up
    P3OUT  |= 0X3E;    //pull up

    P3IES  |= 0x3E;
    P3IFG  &=~ 0x3E;   //Creal interrupt flags
    P3IE   = 0X3E;    //Habilitar interruptors


    //PORT P4 (4.4//4.5)
    P5SEL0 &=~0X30;   //S'ESPECIFICA COM A GPIO EL PINT P5.2
    P5SEL1 &=~0X30;//S'ESPECIFICA COM A GPIO EL PINT P5.2
    P5DIR  |=0X30; //S'ESPECIFICA COM A OUTPUT
    P5OUT  =0X00;  // LES SORTIDES S'ESPECIFIQUEN A 0

    //PORT 2 (P2.0) TRIGGER;
    P2SEL0 &=~0X01;
    P2SEL0 &=~0X01;
    P2DIR |=0X01;
    P2OUT=0X00;

}

void init_ADC(){

    PM5CTL0 &= ~LOCKLPM5; // Disable the GPIO power-on default high-impedance mode
    // to activate previously configured port settings
    P1SEL0 |= BIT5 |BIT4; // Enable A/D channel inputs
    P1SEL1 &= ~(BIT4 |BIT5); // Enable A/D channel inputs
    ADCCTL0 |= ADCON+ADCSHT_2; // Turn on ADC12, S&H=16 clks //En aquet registre actives la conversio //S'haura de activar ADCENC +ADCSC
    ADCCTL1 |= ADCSHP; // ADCLK=MODOSC, //rellotge no extern
    ADCCTL2 &= ~ADCRES; // clear ADCRES in ADCTL
    ADCCTL2 |= ADCRES_2; // 12 bits resolution
    ADCIE = ADCIE0; // Habilita interrupció


}
uint16_t mesura_ADC(uint8_t canal){
    uint16_t mesura_complete;
    conversion_complete=0;
    if (canal==4)
        ADCMCTL0 |= ADCINCH_4;
        ADCCTL0 |= ADCENC | ADCSC;

    if (canal==5)
        ADCMCTL0 |= ADCINCH_5;
        ADCCTL0 |= ADCENC | ADCSC;

    while(conversion_complete==1){

    }
    mesura_complete=ADCMEM0;
    ADCCTL0 &= ~ADCENC; //Desahabilitem l’ADC per poder canviar de canal
    return (mesura_complete);

}

uint16_t meas_ADC(uint8_t canal,uint8_t N_valors){
    uint16_t resultat;
    uint8_t i;
    i=0;
    resultat=0;
    for(i=0;i<N_valors;i++){
        resultat=resultat+mesura_ADC(canal)/N_valors;
    }
    return(resultat);

}

//TIMERS CONFIGURACIÓ

void init_timer(){ //timer de 1ms


//INICIALITZACIÓ TIMER TB0
    TB0CTL|=TBSSEL_2; //S'agafa el rellotge SMCLK 16MHZ
    TB0CCTL0|=CCIE_1; //Habilita la interupcio CCIE

    TB0CCR0=16000;    //CONFIGURA A 1mS CONVERSIO DE 16MHZ A 1MS ES EL NUMERO DE POLSOS PERQUE DURI 1ms

//INICIALITZACIO TIMER TB2

       TB2CTL|=TBSSEL_2; //S'agafa el rellotge SMCLK 16MHZ
       TB2CCTL0|=CCIE_1; //Habilita la interupcio CCIE
       TB2CCR0=16;    //CONFIGURA A 1mS CONVERSIO DE 16MHZ A 1MS ES EL NUMERO DE POLSOS PERQUE DURI 1ms

    //INICIALITZACIÓ TIMER TB3.1

    //PIN FUNCIó P6.0-> TB1.2 CCIA Capture

    P2SEL0 |= 0X02;
    P2SEL1 &= ~0X02;
    P6DIR  &= ~0X02;

    TB1CTL|=TBSSEL_1;
    TB1CCTL2|=CM_3|CCIS_0|CCIE_1|CAP_1|SCS_1; //S'OBSERVA EL VALOR DEL BIT DE CAPTURA
    capture_done=0;

}

uint16_t captura_timer(){
    uint16_t timer;
    capture_done=0;
    TB1R=0X0000;
    TB1CCTL2|=CM_3;
    TB1CTL|=MC_2;

    while(capture_done==0){

    }
    timer=TB1CCR2;
    TB1CTL &=~MC_2;
    TB1CCTL2&=~CM_3;


    return(timer);
}

uint16_t ultrasons(){
    uint16_t trimer1;
    uint16_t trimer2;
    uint16_t temps_obstacle;
    uint16_t distancia;



    //INICIALITZACIO ULTRASONS
    P2OUT=0X00;
    delay_us(1);
    P2OUT^=0X01;
    delay_us(1);
    P2OUT^=0X01;

    trimer1=captura_timer();
    trimer2=captura_timer();
    temps_obstacle=trimer2;
//    temps_obstacle=temps_obstacle/2*32000;
//    distancia=331*temps_obstacle;
//    return(distancia);
      return(temps_obstacle);




}


void delay(uint32_t temps){   //Genera el temps de espera

    contador_ms=0;            //inicialment s'inicialitza a 0
    TB0CTL |= TBCLR; // Limpiar el contador
    TB0CTL|=MC_1;             //S'habilita mode UP

    while(temps>contador_ms){ //fins que s'hagui fet mes que "temps" de  interupcions surt del bucle
    }
    TB0CTL&=~MC_1;            //PARA EL COMPTADOR

}

void delay_us(uint32_t temps){
    contador_us=0;            //inicialment s'inicialitza a 0
    TB2CTL |= TBCLR; // Limpiar el contador
    TB2CTL|=MC_1;             //S'habilita mode UP

    while(temps>contador_us){ //fins que s'hagui fet mes que "temps" de  interupcions surt del bucle
    }
    TB2CTL&=~MC_1;            //PARA EL COMPTADOR


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
void delete_LCD(){
   delay(10);
   Array_LCD[0] = 0x00; //
   Array_LCD[1] = 0x01; //
   Array_LCD[2] = 0x20; //
   Array_LCD[3] = 0x00; //
   delay(10);
   I2C_send(0x3E,Array_LCD,4);// S'envia la trama al LCD indicant la adress slave del LCD
   delay(10);
}
void move_line(){
  delay(10);
  Array_LCD[0] = 0x00; //
  Array_LCD[1] = 0x80|0X28; //




  Array_LCD[2]=0x00;
  delay(10);
  I2C_send(0x3E, Array_LCD, 3);
  delay(10);
  }

void move_cursor(uint8_t position){ //1r linia va de 0x00//0x0F  2n linia //0X28//0x37

      delay(10);
      Array_LCD[0] = 0x00; //
      Array_LCD[1] = 0x80|position; //




      Array_LCD[2]=0x00;
      delay(10);
      I2C_send(0x3E, Array_LCD, 3);
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
       init_ADC();
       resultat_ADC=0;
       counter=0;

    __enable_interrupt();
//    while(1){
//        delay(1000);
//      resultat_ADC=mesura_ADC(4);
//      init_LCD();                        //Inicialitza LCD
//                delay(10);
//                longitud=sprintf(test_LCD,"@%d y \n ",resultat_ADC);  // la paraula velocitat que volem mostrar en la pantalla
//                I2C_send(0x3E,test_LCD,longitud);             //s'envia aquesta dada
//                delay(10);

//    }
//     init_LCD();                        //Inicialitza LCD
//          delay(10);
//          longitud=sprintf(test_LCD,"@HOLA y \n ");  // la paraula velocitat que volem mostrar en la pantalla
//          I2C_send(0x3E,test_LCD,longitud);             //s'envia aquesta dada
//          delay(10);
//          longitud=sprintf(test_LCD,"@ ADIOS"); //Es fa visualitzar en el display LCD el missatge escrit
//          delay(10);
//          I2C_send(0x3E,test_LCD,longitud);
//          delay(10);

//    leds_RGB(1,1);
    //TEST ROBOT


    while(1){

        resultat_ADC=ultrasons();


    }
    joystick_state=50;
    init_LCD();
    delay(10);
    longitud = sprintf(test_LCD, "@DIR=N VEL=%d",Array_motor[2]);
    I2C_send(0x3E, test_LCD, longitud);




    while(1){

        delay(1000);
//        resultat_ADC=meas_ADC(5,10);;
//        resultat_ADC_promitg=meas_ADC(4,10);
          resultat_ADC_promitg=mesura_ADC(4);
          resultat_ADC= mesura_ADC(5);
        delay(1000);



    }

    while (1){


        switch (joystick_state) {
        case 0:
            delay(20);
            mov_motor(0x01, 0x00, 0x01, 0x00);
            move_cursor(0x0A);
            delay(10);
            longitud = sprintf(test_LCD, "@%d",Array_motor[2]);
            I2C_send(0x3E, test_LCD, longitud);
            delay(10);
            move_cursor(0x04);
            delay(10);
            longitud = sprintf(test_LCD,"@N");
            I2C_send(0x3E, test_LCD, longitud);
            delay(10);
            move_cursor(0x00);

            break;


            case 1: // TOP
                delay(20);
                mov_motor(0x01, 0x20, 0x01, 0x20);
                delay(10);
                move_cursor(0x0A);
                delay(10);
                longitud = sprintf(test_LCD, "@%d",Array_motor[2]);
                I2C_send(0x3E, test_LCD, longitud);
                delay(10);
                move_cursor(0x04);
                delay(10);
                longitud = sprintf(test_LCD,"@T");
                I2C_send(0x3E, test_LCD, longitud);
                delay(10);
                move_cursor(0x00);

                break;
            case 2: // LEFT
                                        delay(20);
                                        mov_motor(0x01, 0x00, 0x01, 0x20);
                                         move_cursor(0x0A);
                                         delay(10);
                                         longitud = sprintf(test_LCD, "@%d",Array_motor[2]);
                                         I2C_send(0x3E, test_LCD, longitud);
                                         delay(10);
                                         move_cursor(0x04);
                                         delay(10);
                                         longitud = sprintf(test_LCD,"@L");
                                         I2C_send(0x3E, test_LCD, longitud);
                                         delay(10);
                                         move_cursor(0x00);


                break;
            case 3: // TOP
                move_cursor(0x0B);
                                                     delay(20);
                                                     longitud = sprintf(test_LCD, "%d",Array_motor[2]);
                                                     I2C_send(0x3E, test_LCD, longitud);
                                                     delay(10);
                                                     move_cursor(0x04);
                                                     delay(10);
                                                     longitud = sprintf(test_LCD,"T");
                                                     I2C_send(0x3E, test_LCD, longitud);
                                                     delay(10);
                                                     move_cursor(0x00);

                break;
            case 4: // RIGHT
                delay(10);
                longitud = sprintf(test_LCD, "@RIGHT \n ");
                I2C_send(0x3E, test_LCD, longitud);
                delay(10);

                break;
            case 5: // BUTTON
                delay(10);
                longitud = sprintf(test_LCD, "@BOTTOM \n ");
                I2C_send(0x3E, test_LCD, longitud);
                delay(10);

                break;
            default:
                break;
        }

    }

    while (1) {
        // Processar l'estat del joystick



              switch (joystick_state) {
              case 0:
                  delay(10);
                  longitud = sprintf(test_LCD, "@HOLA COMO VA \n ");
                  I2C_send(0x3E, test_LCD, longitud);
                  delay(10);
                  mov_motor(0x01, 0x00, 0x01, 0x00);
                  joystick_state = 50; // Reinicia l'estat després de processar-lo
                  break;


                  case 1: // BOTTOM
                      delay(10);
                      longitud = sprintf(test_LCD, "@TOP \n ");
                      I2C_send(0x3E, test_LCD, longitud);
                      delay(10);
                      mov_motor(0x01, 0x20, 0x01, 0x20);
                      joystick_state = 50; // Reinicia l'estat després de processar-lo
                      break;
                  case 2: // LEFT
                      delay(10);
                      longitud = sprintf(test_LCD, "@LEFT \n ");
                      I2C_send(0x3E, test_LCD, longitud);
                      delay(10);
                      mov_motor(0x01, 0x20, 0x01, 0x00);
                      joystick_state = 50;
                      break;
                  case 3: // TOP
                      delay(10);
                      longitud = sprintf(test_LCD, "@BOTTOM \n ");
                      I2C_send(0x3E, test_LCD, longitud);
                      delay(10);
                      mov_motor(0x02, 0x20, 0x02, 0x20);
                      joystick_state = 50;
                      break;
                  case 4: // RIGHT
                      delay(10);
                      longitud = sprintf(test_LCD, "@RIGHT \n ");
                      I2C_send(0x3E, test_LCD, longitud);
                      delay(10);
                      mov_motor(0x01, 0x00, 0x01, 0x20);
                      joystick_state = 50;
                      break;
                  case 5: // BUTTON
                      delay(10);
                      longitud = sprintf(test_LCD, "@BUTTON \n ");
                      I2C_send(0x3E, test_LCD, longitud);
                      delay(10);
                      mov_motor(0x00, 0x00, 0x00, 0x00);
                      joystick_state = 50;
                      break;
                  default:
                      break;
              }




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


joystick_state=0x00;
uint8_t pulsed;
uint16_t vector_flag=P3IV;
P3IE=0X00;
P3IES^=0X3E;
joystick_state=0x00;



switch(vector_flag){

    case 0x00:break;
    case 0x02:break;

    case 0x04:                      //P3.1 BOTTOM
        P3_Input=P3IN & BIT1;
             if(P3_Input==0){
                        joystick_state=1;
                    }
                    else {
                        joystick_state=0;
                    }

        break;

    case 0x06:
        P3_Input=P3IN & BIT2; //P3.2 LEFT
        if(P3_Input==0){
                   joystick_state=2;
               }
               else {
                   joystick_state=0;
               }

        break;

    case 0x08:                    // P3.3 TOP

        break;

    case 0x0A:                    //P3.4 RIGHT

             break;

    case 0x0C:                   // P3.5 BOTON
        P3_Input=P3IN & BIT5;
                 if(P3_Input==0){
                     joystick_state=5;
                 }
                 else {
                     joystick_state=0;
                 }
                 break;

    default :break;

}

P3IFG=0X00;
P3IE    =0X3E;




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
/* El que volem fer a la rutina datenció dInterrupció */
}

#pragma vector=TIMER2_B0_VECTOR //Aquest és el nom important
__interrupt void PORT1_ISR (void)
{

        contador_us=contador_us+1;






    TB2CCTL0&=~CCIFG;
/* El que volem fer a la rutina datenció dInterrupció */
}
#pragma vector=TIMER1_B1_VECTOR //Aquest és el nom important
__interrupt void TIMER0_B1_ISR (void)
{

    capture_done=1;
    TB1CCTL2&=~CCIFG;



/* El que volem fer a la rutina datenció dInterrupció */
}

#pragma vector=ADC_VECTOR
__interrupt void ADC_ISR(void)
{
    uint8_t vector_flag=ADCIV0;

    switch(vector_flag){

    case 0x00: break;
    case 0x02:break;
    case 0x04:break;
    case 0x06: break;
    case 0x08: break;
    case 0x0A: break;
    case 0x0C:conversion_complete=1;







}
}

