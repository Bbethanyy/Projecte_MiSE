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


//MODO

uint8_t Modo;
uint8_t Number_moni;
uint8_t Mod_cursor;
uint8_t modu_cursor;
uint8_t  Menu_position;
uint8_t Menuu_position;

//CONFIG

uint8_t config_velocitat;
uint8_t config_RGB;


//MOTOR
uint8_t Array_motor[5]; //VARIABLE ENCARREGUADA DE ENVIA LES TRAMES I2C AL MOTOR
uint8_t a;


//ADC
uint16_t conversion_complete;
uint16_t resultat_ADC;
uint16_t ilum1;
uint16_t ilum2;
uint8_t update_ADC;

//JOYSTICK
uint16_t J_right;
uint16_t J_bot;


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
uint8_t update_ultrasound;
uint16_t comptador;


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

    //PORT P3 (P3.1 (BOTTOM)/P3.2(LEFT)/P3.5(BOTON)//)
    P3SEL0 &= ~0X3E;   //Especifica P3.1 a P3.5 com a GPIO
    P3SEL1 &= ~0X3E;
    P3DIR  &= ~0X3E;   //S'especifica com a input
    P3REN  |= 0X3E;    //Habilitar resistencies pull up
    P3OUT  |= 0X3E;    //pull up


    P3IES  |= 0x3E;
    P3IFG  &=~ 0x3E;   //Creal interrupt flags
    P3IE   = 0X26;    //Habilitar interruptors


    //PORT P4 (4.4//4.5)
    P5SEL0 &=~0X30;   //S'ESPECIFICA COM A GPIO EL PINT P5.2
    P5SEL1 &=~0X30;//S'ESPECIFICA COM A GPIO EL PINT P5.2
    P5DIR  |=0X30; //S'ESPECIFICA COM A OUTPUT
    P5OUT  =0X00;  // LES SORTIDES S'ESPECIFIQUEN A 0

    //PORT 2 (P2.0) TRIGGER/P2.2 BOT /P2.3 RIGHT;
    P2SEL0 &=~0X0D;
    P2SEL0 &=~0X0D;
    P2DIR |=0X0D;
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
    volatile uint16_t mesura_complete;
    conversion_complete=0;
    if (canal==4)
        ADCMCTL0 |= ADCINCH_4;


    if (canal==5)
        ADCMCTL0 |= ADCINCH_5;

    ADCCTL0 |= ADCENC | ADCSC;

    while(conversion_complete!=1){

    }
    mesura_complete=ADCMEM0;
    ADCCTL0 &= ~ADCENC; //Desahabilitem l’ADC per poder canviar de canal
    ADCMCTL0 &= ~0x0F; // Limpia los bits del canal

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


    //INICIALITZACIÓ TIMER TB1.2

    //PIN FUNCIó P6.0-> TB1.2 CCIA Capture

    P2SEL0 |= 0X02;
    P2SEL1 &= ~0X02;
    P2OUT=0X00;
    P6DIR  &= ~0X02;

    TB1CTL|=TBSSEL_1;
    TB1CCTL2|=CM_3|CCIS_0|CCIE_1|CAP_1|SCS_1; //S'OBSERVA EL VALOR DEL BIT DE CAPTURA
    capture_done=0;
    //INICIALITZACIO TIMER TB2

    TB2CTL|=TBSSEL_2; //S'agafa el rellotge SMCLK 16MHZ
    TB2CCTL0|=CCIE_1; //Habilita la interupcio CCIE
    TB2CCR0=16;    //CONFIGURA A 1mS CONVERSIO DE 16MHZ A 1MS ES EL NUMERO DE POLSOS PERQUE DURI 1ms
   //INICIALITZACIÓ TIMER TB3
    TB3CTL|=TBSSEL_2; //S'agafa el rellotge SMCLK 16MHZ
    TB3CCTL0|=CCIE_1; //Habilita la interupcio CCIE
    TB3CCR0=16000;    //CONFIGURA A 1mS CONVERSIO DE 16MHZ A 1MS ES EL NUMERO DE POLSOS PERQUE DURI 1ms


}

void crono_timer(){
    TB3CTL|=MC_1;             //S'habilita mode UP

}

uint16_t captura_timer(){
    uint16_t timer;
    capture_done=0;
//    TB1R=0X0000;
//    TB1CCTL2|=CM_3|CAP_1;
    TB1CTL|=MC_2;


    while(capture_done==0){
       if((TB1CCTL2&COV_1)==COV_1){
           capture_done=1;
       }
    }
    timer=TB1CCR2;

    TB1CTL|=TBCLR;
    TB1CCTL2&=~COV_1;


//    TB1CCTL2&=CM_0;




    return(timer);
}



uint16_t ultrasons(){
    uint16_t trimer1;
    uint16_t trimer2;
    uint16_t temps_obstacle;
    uint16_t distancia;

    //CHECK CAPTURA


    //INICIALITZACIO ULTRASONS COMPROVACIO RESULTAT
    P2OUT|=0X01;
    delay_us(1);
    P2OUT^=0X01;

    trimer1=captura_timer();

    trimer2=captura_timer();

//
    distancia=33100/32000;
    distancia=distancia*trimer2/2;
//    distancia=distancia*343/2;
//    temps_obstacle=temps_obstacle/2*32000;
//    distancia=331*temps_obstacle;
//    return(distancia);
    return(distancia);





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

void Menu_moniterització(){
    move_cursor(0x00);

    delay(10);
    longitud = sprintf(test_LCD, "@ DIR=N   VEL=%d",Array_motor[2]);
    I2C_send(0x3E, test_LCD, longitud);
    delay(10);
    move_cursor(0x28);
    delay(10);
    longitud = sprintf(test_LCD, "@I=       Dis= ",Array_motor[2]);
    I2C_send(0x3E, test_LCD, longitud);
    move_cursor(0x00);
}

void Menu_inicial(){
    move_cursor(0x00);
    delay(10);
      longitud = sprintf(test_LCD, "@ CONFIG  MANUAL");
      I2C_send(0x3E, test_LCD, longitud);
      delay(10);
      move_cursor(0x28);
      delay(10);
      longitud = sprintf(test_LCD, "@ ILUM     DIST ");
      I2C_send(0x3E, test_LCD, longitud);
      move_cursor(0x00);
}

void Menu_config(){
    move_cursor(0x00);
    delay(10);
      longitud = sprintf(test_LCD, "@ VELOCITAT=%d",config_velocitat);
      I2C_send(0x3E, test_LCD, longitud);
      delay(10);
      move_cursor(0x28);
      delay(10);
      longitud = sprintf(test_LCD, "@ LED RGB=%d",config_RGB);
      I2C_send(0x3E, test_LCD, longitud);
      move_cursor(0x00);
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
       joystick_state=0;
       update_ultrasound=0;
       comptador=0;
       Modo=0;
       Number_moni=0;
       Mod_cursor=0;
       Menu_position=1;
       Menuu_position=1;
       config_RGB=0;
       config_velocitat=0;
       update_ultrasound=1;
       update_ADC=1;

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

//    while(1){

//          delay(100);
  //        resultat_ADC=meas_ADC(5,10);;
//          resultat_ADC_promitg=meas_ADC(5,10);
//          resultat_ADC= mesura_ADC(4);
//          delay(100);



//      }

//    delay(1000);
//    while(1){
//        delay(100);
//        resultat_ADC=ultrasons();
//        delay(100);



//    }//


    //ES CREA EL MENU DE MONITERITZACIÓ
    init_LCD();

    crono_timer();



//
    while (1){


        switch(Modo){


        case 0x00:
            if(Number_moni==0){
                joystick_state=0;
                delete_LCD();
                Menu_inicial();
                delay(1000);
                Number_moni=1;
            }


            delay(100);

            if(joystick_state==2){

                move_cursor(Mod_cursor);
                delay(10);
                longitud = sprintf(test_LCD, "@ ");
                I2C_send(0x3E, test_LCD, longitud);
                delay(10);



                switch(Menu_position){

                case 1:
                    Mod_cursor=0x31;
                    break;
                case 2:
                    Mod_cursor=0x28;
                    break;
                case 3:
                    Mod_cursor=0x08;
                    break;
                case 4:
                    Mod_cursor=0x00;
                    Menu_position=0;
                    break;
                default: break;



                }

                move_cursor(Mod_cursor);
                delay(10);
                longitud = sprintf(test_LCD, "@-");
                I2C_send(0x3E, test_LCD, longitud);
                delay(10);


                Menu_position=Menu_position+1;


                }
            else if(joystick_state==5){
                joystick_state=0;
                Number_moni=0;

                switch(Menu_position){

                                case 1:
                                    Modo=1;
                                    break;
                                case 2:
                                    Modo=4;
                                    break;
                                case 3:
                                    Modo=3;
                                    break;
                                case 4:
                                    Modo=2;
                                    break;
                                default: break;



                                }
            }
        break;


        case 1:
                if(Number_moni==0){
                            joystick_state=0;
                            delete_LCD();
                            Menu_config();
                            delay(1000);
                            Number_moni=1;
                                    }
                delay(100);
                if (joystick_state==1){

                            move_cursor(Mod_cursor);
                            delay(10);
                            longitud = sprintf(test_LCD, "@ ");
                            I2C_send(0x3E, test_LCD, longitud);
                            delay(10);
                            switch(Menuu_position){
                                case 1:
                                    Mod_cursor=0x00;
                                    break;
                                case 2:
                                    Mod_cursor=0x28;
                                    Menuu_position=0;
                                    break;
                                }
                            move_cursor(Mod_cursor);
                            delay(10);
                            longitud = sprintf(test_LCD, "@-");
                            I2C_send(0x3E, test_LCD, longitud);
                            delay(10);
                            Menuu_position=Menuu_position+1;


                        }
                if(joystick_state==2){


                    switch(Menuu_position){
                        case 1:
                            modu_cursor=0x31;

                            config_RGB=config_RGB+1;
                            if(config_RGB==8){
                                config_RGB=0;
                            }
                            move_cursor(modu_cursor);
                            delay(10);
                            longitud = sprintf(test_LCD, "@    ");
                            I2C_send(0x3E, test_LCD, longitud);
                            delay(10);
                            move_cursor(modu_cursor);
                            delay(10);
                            longitud = sprintf(test_LCD, "@%d    ",config_RGB);
                            I2C_send(0x3E, test_LCD, longitud);
                            delay(10);

                            break;
                        case 2:

                            modu_cursor=0x0B;
                            config_velocitat=config_velocitat+10;
                            if(config_velocitat==240){
                                config_velocitat=0;
                            }
                            move_cursor(modu_cursor);
                            delay(10);
                            longitud = sprintf(test_LCD, "@%d    ",config_velocitat);
                            I2C_send(0x3E, test_LCD, longitud);
                            delay(10);

                            break;
                            }

                    }
                if(joystick_state==5){
                    Menu_position=0;
                    Modo=0;
                    joystick_state=0;
                    Number_moni=0;
                }

        break;







        default:

            if(Number_moni==0){
                joystick_state=0;
                delete_LCD();
                delay(10);
                Menu_moniterització();
                delay(1000);
                Number_moni=1;
            }


            break;


        }


        if(Modo>1){


        switch (joystick_state) {
        case 0:
            delay(20);
            mov_motor(0x01, 0x00, 0x01, 0x00);

            move_cursor(0x0E);
            delay(10);
            longitud = sprintf(test_LCD, "@%d",Array_motor[2]);
            I2C_send(0x3E, test_LCD, longitud);
            delay(10);
            move_cursor(0x05);
            delay(10);
            longitud = sprintf(test_LCD,"@N");
            I2C_send(0x3E, test_LCD, longitud);
            delay(10);
            move_cursor(0x00);
            break;


            case 1: // TOP
                delay(20);
                mov_motor(0x01, config_velocitat, 0x01, config_velocitat);
                delay(10);
                move_cursor(0x0E);
                delay(10);
                longitud = sprintf(test_LCD, "@%d",Array_motor[1]);
                I2C_send(0x3E, test_LCD, longitud);
                delay(10);
                move_cursor(0x05);
                delay(10);
                longitud = sprintf(test_LCD,"@T");
                I2C_send(0x3E, test_LCD, longitud);
                delay(10);
                move_cursor(0x00);

                break;
            case 2: // LEFT
                    delay(20);
                    mov_motor(0x01, 0x00, 0x01, config_velocitat);
                    move_cursor(0x0E);
                    delay(10);
                    longitud = sprintf(test_LCD, "@%d",Array_motor[2]);
                    I2C_send(0x3E, test_LCD, longitud);
                    delay(10);
                    move_cursor(0x05);
                    delay(10);
                    longitud = sprintf(test_LCD,"@L");
                    I2C_send(0x3E, test_LCD, longitud);
                    delay(10);
                    move_cursor(0x00);


                break;
            case 3: // BOT
                delay(20);
                mov_motor(0x02, config_velocitat, 0x02, Array_motor[2]);
                move_cursor(0x0E);
                delay(10);
                longitud = sprintf(test_LCD, "@%d",Array_motor[1]);
                I2C_send(0x3E, test_LCD, longitud);
                delay(10);
                move_cursor(0x05);
                delay(10);
                longitud = sprintf(test_LCD,"@B");
                I2C_send(0x3E, test_LCD, longitud);
                delay(10);
                move_cursor(0x00);

                break;
            case 4: // RIGHT
                delay(20);
                mov_motor(0x01, config_velocitat, 0x01, 0x00);
                move_cursor(0x0E);
                delay(10);
                longitud = sprintf(test_LCD, "@%d",Array_motor[1]);
                I2C_send(0x3E, test_LCD, longitud);
                delay(10);
                move_cursor(0x05);
                delay(10);
                longitud = sprintf(test_LCD,"@R");
                I2C_send(0x3E, test_LCD, longitud);
                delay(10);
                move_cursor(0x00);

                break;
            case 5: // BUTTON
                delay(10);
                longitud = sprintf(test_LCD, "@BOTTOM \n ");
                I2C_send(0x3E, test_LCD, longitud);
                delay(10);
                Menu_position=0;
                Modo=0;
                joystick_state=0;

                Number_moni=0;


                break;
            default:
                break;
        }


        if (update_ADC){
            J_right=mesura_ADC(5);

            ilum1=meas_ADC(5,10)/10;

            J_bot=mesura_ADC(4);
            ilum2=meas_ADC(4,10)/10;

            J_right=J_right*3;
            J_right=J_right/4096;

            J_bot=J_bot*3;
            J_bot=J_bot/4096;

            if(Modo==2){

                if(J_right>1)
                     {
                    joystick_state=4;
                     }
                else if(J_bot>1)
                     {
                        joystick_state=3;
                     }
                else if((J_bot && J_right)==1)
                   {
                       joystick_state=0;
                   }
                else
                {
                   joystick_state=joystick_state;
                }
            }
            if(Modo==3){
                if (ilum1>350){
                    joystick_state=1;
                    leds_RGB(config_RGB,0);
                }
                if(ilum2>350){
                    joystick_state=2;
                    leds_RGB(0,config_RGB);
                }
                else if((ilum1<350)& (ilum2<350)){
                    joystick_state=0;
                    leds_RGB(0,0);
                }
            }

            if((Modo==3)|(Modo==4)){
                delay(10);
                move_cursor(0x3A);
                delay(10);
                longitud = sprintf(test_LCD, "@ ");
                I2C_send(0x3E, test_LCD, longitud);
                delay(100);
                move_cursor(0x3A);
                delay(10);
                longitud = sprintf(test_LCD, "@%d/%d",ilum1,ilum2);
                I2C_send(0x3E, test_LCD, longitud);
                delay(10);


        }

        }



        if(update_ultrasound){
            delay(10);
            ultrasound=ultrasons();
            delay(10);
            move_cursor(0x35);
            delay(10);
            longitud = sprintf(test_LCD, "@   ",ultrasound);
            I2C_send(0x3E, test_LCD, longitud);
            delay(10);
            move_cursor(0x35);
            delay(10);
            longitud = sprintf(test_LCD, "@%d",ultrasound);
            I2C_send(0x3E, test_LCD, longitud);
            delay(10);
            if(Modo==4){
                if(ultrasound>5){
                    joystick_state=1;
                    leds_RGB(config_RGB,config_RGB);
                }
                else{
                    joystick_state=2;
                }
            }

        }

    }
    }


//



    //CONFIGURACIO LED LCR


// INTERUPCIONS PORTS GPIOS

}

//PORT 3

#pragma vector=PORT3_VECTOR //interrupcions dels botons i Joystick.
__interrupt void port3_ISR(void)
{ //PORT P3 (P3.1 (BOTTOM)/P3.2(LEFT)/P3.3(TOP)/P3.4(RIGHT)/P3.5(BOTON)



uint16_t vector_flag=P3IV;
P3IE&=~0X26;


switch(vector_flag){

    case 0x00:break;
    case 0x02:break;

    case 0x04:                      //P3.1 TOP
        P3_Input=P3IN & BIT1;
             if(P3_Input==0){

                        joystick_state=1;
                        P3IES&=~BIT1;
                    }
                    else {
                        joystick_state=0;
                        P3IES|=BIT1;
                    }


        break;

    case 0x06:
        P3_Input=P3IN & BIT2; //P3.2 LEFT
        if(P3_Input==0){

                   joystick_state=2;
                   P3IES&=~BIT2;
               }
               else {
                   joystick_state=0;
                   P3IES|=BIT2;
               }


        break;

    case 0x08:                    // P3.3 TOP

        break;

    case 0x0A:                    //P3.4 RIGHT

             break;

    case 0x0C:                   // P3.5 BOTON
        P3_Input=P3IN & BIT5;
                 if(P3_Input==0){
                     P3IES&=~BIT5;
                     joystick_state=5;
                 }
                 else {
                     P3IES|=BIT5;
                     joystick_state=0;
                 }
                 break;

    default :break;

}

P3IFG=0X00;
P3IE|=0X26;




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


    if( TB1CCTL2&CCIFG==1){
        if(TB1CCR2==1){
            capture_done=0;
        }
        else{
            capture_done=1;
        }

        TB1CCTL2&=~CCIFG;

    }






/* El que volem fer a la rutina datenció dInterrupció */
}

#pragma vector=TIMER3_B0_VECTOR //Aquest és el nom important
__interrupt void TB30_ISR (void)
{
uint16_t comptador2;
    if((Modo==1)|(Modo==0)){
        comptador=0;
        comptador2=0;
    }


    if(comptador==60){
        if(Modo>1){
//            update_ADC=1;
            comptador=0;
//            update_ultrasound=1;

        }
    if (comptador2==1000){
        if(Modo>1){
        comptador2=0;
//        update_ultrasound=1;
        }
    }



}

comptador ++;
comptador2 ++;
TB2CCTL0&=~CCIFG;
/* El que volem fer a la rutina datenció dInterrupció */
}

#pragma vector=ADC_VECTOR
__interrupt void ADC_ISR(void)
{
    uint16_t vector_flag=ADCIV;

    switch(vector_flag){

    case 0x0000: break;
    case 0x0002:break;
    case 0x0004:break;
    case 0x0006: break;
    case 0x0008: break;
    case 0x000A: break;
    case 0x000C:
        conversion_complete=1;

        break;
    default :break;








}

ADCIFG&=0x0000;
}

