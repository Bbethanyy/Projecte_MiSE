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

uint8_t Modo;  //INDICA EL MODE DEL ROBOT
uint8_t Number_moni; //LIMITA A UNA VEGADA LA PANTALLA A GRAFICAR A CADA MODE
uint8_t Mod_cursor;  //INDICA DE FORMA VISUAL ON ES TROBA SITUAT DEL MENU INICIAL 
uint8_t modu_cursor;  //INDICA DE FORMA VISUAL ON ES TROBA SITUAT DEL MENU CONFIG
uint8_t Menu_position; //INDICA EL CURSOR ON ES TROBA SITUAT DEL MENU INICIAL 
uint8_t Menuu_position; //INDICA EL CURSOR ON ES TROBA SITUAT DEL MENU CONFIGURACIÓ

//CONFIG

uint8_t config_velocitat;//VALOR VELOCITAT ESPECIFICAT EN LA CONFIGURACIÓ
uint8_t config_RGB; //VALOR DEL LED ESPECIFICAT EN LA CONFIGURACIÓ


//MOTOR
uint8_t Array_motor[5]; //VARIABLE ENCARREGUADA DE ENVIA LES TRAMES I2C AL MOTOR
uint8_t a;


//ADC
uint16_t conversion_complete;
//DIFERENTS MESURES FETES EN EL ADC
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
uint8_t  capture_done;  //INDICA SI S'HA FET UNA CAPTURA
uint32_t contador_ms;//NUMERO DE CICLES NECESSARIS PER GENERAR 1MS PER UN SMLCK 16MHZ
uint16_t timer_capture;
uint32_t contador_us; //NUMERO DE CICLES NECESSARIS PER GENERAR 10us PER UN SMLCK 16MHZ
//ULTRASONS
uint16_t ultrasound; //DADE ON S'ALMACENA EL VALOR OBTINGUT DEL ULTRASONS
uint8_t update_ultrasound; //ACTUALITZA EL ULTRASONS
uint16_t comptador; //


//GPIO

uint8_t counter;
uint8_t inputGP;
uint8_t P3_Input;
volatile uint8_t joystick_state = 0; //INDICA I REPRESENTA EL MOVIMENT DEL MOTOR I SI S'ENVA EL MENU INICIAL


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
uint16_t mesura_ADC(uint8_t canal){ //REALITZA UNA MESURA DEL ADC DEL CANAL ESPECIFICAT SINGLE CHANNEL SINGLE CONVERSION
    volatile uint16_t mesura_complete;
    conversion_complete=0;
    if (canal==4)
        ADCMCTL0 |= ADCINCH_4;


    if (canal==5)
        ADCMCTL0 |= ADCINCH_5;

    ADCCTL0 |= ADCENC | ADCSC; //INICIA LA CONVERSIO ADC I EL MOSTREIG

    while(conversion_complete!=1){

    }
    mesura_complete=ADCMEM0; //VARIABLE ON S'ALMACENA EL VALOR DEL ADC
    ADCCTL0 &= ~ADCENC; //Desahabilitem l’ADC per poder canviar de canal
    ADCMCTL0 &= ~0x0F; // Limpia los bits del canal

    return (mesura_complete);

}

uint16_t meas_ADC(uint8_t canal,uint8_t N_valors){ //VALOR PROMITG DE N MESURES DEL ADC
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

    TB1CTL|=TBSSEL_1; //ACLK CLOCK
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

uint16_t captura_timer(){ //CAPTURA EL FLANC DE PUJADA I DE BAIXADA CONTINUAMENT EL TIMER ESTA ACTIVAT MODE CONTINOUS
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

    TB1CTL|=TBCLR; //ES RESETEJA EL VALOR INICIAL DLE COMPTATGE
    TB1CCTL2&=~COV_1;


//    TB1CCTL2&=CM_0;




    return(timer);
}



uint16_t ultrasons(){ //VARIABLE ENCARREGUADA DE OBTINDRE LA DISTANCIA ALS OBSTACLES
    uint16_t trimer1;
    uint16_t trimer2;
    uint16_t temps_obstacle;
    uint16_t distancia;

    //CHECK CAPTURA


    //INICIALITZACIO ULTRASONS TRIGGER
    P2OUT|=0X01;               //Activa el pin 0 del port 2
    delay_us(1);
    P2OUT^=0X01;

    trimer1=captura_timer();   //Captura del temps del primer timer
    trimer2=captura_timer();   //I del segon timer

//
    distancia=33100/32000;                  //    distancia=distancia*343/2;
    distancia=distancia*trimer2/2;          //    temps_obstacle=temps_obstacle/2*32000;
                                            //    distancia=331*temps_obstacle;
    return(distancia);                      //    return(distancia);

}


void delay(uint32_t temps){   //Genera el temps de espera

    contador_ms=0;            //inicialment s'inicialitza a 0
    TB0CTL |= TBCLR;          // Limpiar el contador
    TB0CTL|=MC_1;             //S'habilita mode UP

    while(temps>contador_ms){ //fins que s'hagui fet mes que "temps" de  interupcions surt del bucle
    }
    TB0CTL&=~MC_1;            //PARA EL COMPTADOR

}

void delay_us(uint32_t temps){
    contador_us=0;            //inicialment s'inicialitza a 0
    TB2CTL |= TBCLR;          //Limpiar el contador
    TB2CTL|=MC_1;             //S'habilita mode UP


    while(temps>contador_us){ //fins que s'hagui fet mes que "temps" de  interupcions surt del bucle
    }
    TB2CTL&=~MC_1;            //PARA EL COMPTADOR


}

//CONFIGURACIO_LCD

void init_LCD(){

    P5OUT&=~0X04;        //Es fa un reset del display LCD
    delay(10);           //Temps de espera
    P5OUT|=0x04;         //S'habilita a 1 activant el display LCD
    delay(10);           //temps de espera



//INICIALITZACIO DISPLAY LCD
    Array_LCD[0] = 0x00;       //
    Array_LCD[1] = 0x39;       //
    Array_LCD[2] = 0x14;       //Es configura la freqüencia OSC
    Array_LCD[3] = 0x74;       //Es configura el contrast
    Array_LCD[4] = 0x54;       //POWER ICON
    Array_LCD[5] = 0x6F;       //Follower controller
    Array_LCD[6] = 0x0C;       //S'activa el display
    Array_LCD[7] = 0x01;       //Neteja lo que tenia escrit el display



    I2C_send(0x3E,Array_LCD,8); //S'envia la trama al LCD indicant la adress slave del LCD
    delay(10);

}
void delete_LCD(){ //ELIMINA CONTINGUT DEL LCD
   delay(10);
   Array_LCD[0] = 0x00;       //Initcialitza els primers valors de l'array
   Array_LCD[1] = 0x01;       //
   Array_LCD[2] = 0x20;       //
   Array_LCD[3] = 0x00;       //
   delay(10);
   I2C_send(0x3E,Array_LCD,4);//S'envia la trama al LCD indicant la adress slave del LCD
   delay(10);
}
void move_line(){
  delay(10);
  Array_LCD[0] = 0x00;      //
  Array_LCD[1] = 0x80|0X28; //

  Array_LCD[2]=0x00;
  delay(10);
  I2C_send(0x3E, Array_LCD, 3);
  delay(10);
  }

void move_cursor(uint8_t position){ //1ra linia va de 0x00 a 0x0F  2n linia //0X28//0x37
                                    //2na linia de 0X28 a 0x37
      delay(10);
      Array_LCD[0] = 0x00;          //
      Array_LCD[1] = 0x80|position; //
      Array_LCD[2]=0x00;
      delay(10);
      I2C_send(0x3E, Array_LCD, 3);// Envia la trama al LCD indicant l'adreça slave del LCD
      delay(10);
}

void Menu_moniterització(){                                          //Funció per a visualitzar la monetorització
    move_cursor(0x00);                                               //Mou el cursor a la primera posició
    delay(10);
    longitud = sprintf(test_LCD, "@ DIR=N   VEL=%d",Array_motor[2]); //Escriu la direcció i velocitat al LCD
    I2C_send(0x3E, test_LCD, longitud);                              //Envia al LCD
    delay(10);
    move_cursor(0x28);                                               //Mou el cursor a la 2na linia
    delay(10);
    longitud = sprintf(test_LCD, "@I=       Dis= ",Array_motor[2]);  //Ecriu la Intensitat i Distancia
    I2C_send(0x3E, test_LCD, longitud);                              //Envia al LCD
    move_cursor(0x00);                                               //Cursor a la primera posició
}

void Menu_inicial(){                                                 //Funcio per a la pantalla inicial
    move_cursor(0x00);
    delay(10);
    longitud = sprintf(test_LCD, "@ CONFIG  MANUAL");                //1ra linia Configuració i manual
    I2C_send(0x3E, test_LCD, longitud);
    delay(10);
    move_cursor(0x28);
    delay(10);
    longitud = sprintf(test_LCD, "@ ILUM     DIST ");                //2na linia LDRs i ultrasons
    I2C_send(0x3E, test_LCD, longitud);
    move_cursor(0x00);
}

void Menu_config(){                                                  //Funció per a la configuració inicial
    move_cursor(0x00);
    delay(10);
    longitud = sprintf(test_LCD, "@ VELOCITAT=%d",config_velocitat); //Escriu la velocitat escollida
    I2C_send(0x3E, test_LCD, longitud);
    delay(10);
    move_cursor(0x28);
    delay(10);
    longitud = sprintf(test_LCD, "@ LED RGB=%d",config_RGB);        //Escriu els RGBs escollits
    I2C_send(0x3E, test_LCD, longitud);
    move_cursor(0x00);
}
// ROBOT

//LEDS

leds_RGB(uint8_t izq_led,uint8_t der_led){                         //Modifica el color dels LEDS RGB
    Array_led[0]=0x0B;                                             //S'especifica  del robot el leds RGB
    Array_led[1]=izq_led;                                          //El color del led esquerre
    Array_led[2]=der_led;                                          //El color del led dreta
    I2C_send(0x10,Array_led,3);                                    //S'envia les dades
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
    WDTCTL = WDTPW | WDTHOLD;               // stop watchdog timer

    //CONFIGURACIO INICIAL
       init_clocks();                        //Inicialitza els clocks
       init_timer();                         //el temporitzador
       init_GPIO();                          //els pins GPIOs
       i2c_init();                           //i el BUS I2C
       init_ADC();                           //Inicialitza ADC
       resultat_ADC=0;                       // Inicialitza la variable resultat_ADC a 0
       counter = 0;                          // Inicialitza el comptador a 0
       joystick_state = 0;                   // Inicialitza l'estat del joystick a 0
       update_ultrasound = 0;                // Inicialitza la variable de control d'actualització d'ultrasons a 0
       comptador = 0;                        // Inicialitza el comptador a 0
       Modo = 0;                             // Inicialitza el mode de funcionament a 0
       Number_moni = 0;                      // Inicialitza la variable Number_moni a 0
       Mod_cursor = 0;                       // Inicialitza la posició del cursor a 0
       Menu_position = 1;                    // Inicialitza la posició del menú a 1
       Menuu_position = 1;                   // Inicialitza la posició del submenú a 1
       config_RGB = 0;                       // Inicialitza la configuració RGB a 0
       config_velocitat = 0;                 // Inicialitza la configuració de velocitat a 0
       update_ultrasound = 1;                // Activa l'actualització d'ultrasons
       update_ADC = 1;                       // Activa l'actualització de l'ADC

    __enable_interrupt();                   //Habilita interrupcions
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

    init_LCD();     //Inicialitza LCD
    crono_timer();  //Inicialitza el cronometre

//
    while (1){

        switch(Modo){ //ESPECIFICA QUIN MODE DEL ROBOT S'ESTA TREBALLANT

        case 0x00:                  //Mode inicial
            if(Number_moni==0) //CADA VEGADA QUE CANVIA EL MODE AQUESTA VARIABLE LIMITA 1 VEGADA LA QUANTITAT DE VEGADES QUE ES REPRESENTA EL MODE LCD
            {
                joystick_state=0;
                delete_LCD();      //Esborra contingut LCD
                Menu_inicial();    //Mostra la pantalla inicial
                delay(1000);
                Number_moni=1;
            }


            delay(100);

            if(joystick_state==2){ //MOVIMENT ESQUERRE

                move_cursor(Mod_cursor);            //Mou el cursor a la posició escollida
                delay(10);
                longitud = sprintf(test_LCD, "@ "); //ELIMINA EL PUNTER DEL ANTERIOR POSICIO
                I2C_send(0x3E, test_LCD, longitud); //Envia a aquesta posició
                delay(10);


                switch(Menu_position){ //EN FUNCIÓ DEL NOMBRE DE VEGADES QUE ES PRESIONA EL JOYSTICK ESQUERRE DETERMINA LA POSICIO DEL MENU

                case 1:
                    Mod_cursor=0x31;                //Canvia la posició del cursor
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

                move_cursor(Mod_cursor);            //Mou el cursor a la nova posició
                delay(10);
                longitud = sprintf(test_LCD, "@-"); //Possa el caràcter -
                I2C_send(0x3E, test_LCD, longitud); //Envia aquest caracter a la posició cursor
                delay(10);


                Menu_position=Menu_position+1;      //Incrementa la posició -


                }
            else if(joystick_state==5){ //SI ES PRESIONA EL BOTO  SELECIONARA EL MODE EN QUE TREBALLARA EL ROBOT
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


        case 1: //CONFIGURACIÓ
                if(Number_moni==0){ //CADA VEGADA QUE CANVIA EL MODE AQUESTA VARIABLE LIMITA 1 VEGADA LA QUANTITAT DE VEGADES QUE ES REPRESENTA EL MODE LCD
                            joystick_state=0;
                            delete_LCD();
                            Menu_config();
                            delay(1000);
                            Number_moni=1;
                                    }
                delay(100);
                if (joystick_state==1){ //QUAN ES PRESIONA CAP ADALT CANVIAR ENTRE VELOCITAT I LED A CONFIGURAR

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
                if(joystick_state==2){ //INDICA EL VALOR QUE ES VOL DE VELOCITAT I DEL COLOR RGB


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
                if(joystick_state==5){ //TORNA AL MENU INICIAL
                    Menu_position=0;
                    Modo=0;
                    joystick_state=0;
                    Number_moni=0;
                }

        break;







        default: //ELS DEMES MODES FAN SERVIR EL MATEIX MENU DE MONITERITZACIÓ

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


        switch (joystick_state) { //INDICA EN FUNCIÓ DE LA VARIABLE EL MOVIMENT QUE SEGUIRA EL ROBOT HI HO MOSTRA CONJUNTAMENT AMB LA DIRECCIÓ
        case 0:
            delay(20);
            mov_motor(0x01, 0x00, 0x01, 0x00);                  //En aquest cas para el motor
            move_cursor(0x0E);                                  //Mou el cursor a la posició 0x0E
            delay(10);
            longitud = sprintf(test_LCD, "@%d",Array_motor[2]); //Posa el valor del motor al `test_LCD`
            I2C_send(0x3E, test_LCD, longitud);                 //Ho envia
            delay(10);
            move_cursor(0x05);                                  //Mou el cursor a la posició 0x05
            delay(10);
            longitud = sprintf(test_LCD,"@N");
            I2C_send(0x3E, test_LCD, longitud);                 //Envia valor "N" al LCD
            delay(10);
            move_cursor(0x00);                                  //Mou el cursor a la posició inicial
            break;


            case 1: // TOP
                delay(20);
                mov_motor(0x01, config_velocitat, 0x01, config_velocitat); //Mou el motor en TOP amb la velocitat configurada
                delay(10);
                move_cursor(0x0E);                                         //Mou el cursor a la posició 0x0E
                delay(10);
                longitud = sprintf(test_LCD, "@%d",Array_motor[1]);        //Posa el valor motor en `test_LCD`
                I2C_send(0x3E, test_LCD, longitud);                        //Envia aquest valor de la direcció
                delay(10);
                move_cursor(0x05);
                delay(10);
                longitud = sprintf(test_LCD,"@T");                         //Posa el valor T de TOP en el LCD
                I2C_send(0x3E, test_LCD, longitud);                        //Ho envia
                delay(10);
                move_cursor(0x00);                                         //Mou el cursor a la posició inicial

                break;
            case 2: // LEFT
                    delay(20);
                    mov_motor(0x01, 0x00, 0x01, config_velocitat);         //Mou el motor a LEFT amb la velocitat configurada
                    move_cursor(0x0E);                                     //Mou el cursor a la posició 0x0E
                    delay(10);                                             //Posa el valor del motor en `test_LCD`
                    longitud = sprintf(test_LCD, "@%d",Array_motor[2]);    //Ho Envia
                    I2C_send(0x3E, test_LCD, longitud);
                    delay(10);
                    move_cursor(0x05);
                    delay(10);
                    longitud = sprintf(test_LCD,"@L");                     //Posa el valor L de LEFT en el LCD
                    I2C_send(0x3E, test_LCD, longitud);                    //Ho envia
                    delay(10);
                    move_cursor(0x00);


                break;
            case 3: // BOT
                delay(20);
                mov_motor(0x02, config_velocitat, 0x02, Array_motor[2]); //Mou el motor a LEFT amb la velocitat configurada
                move_cursor(0x0E);                                       //Mou el cursor a la posició 0x0E
                delay(10);
                longitud = sprintf(test_LCD, "@%d",Array_motor[1]);      //Posa el valor del motor en `test_LCD`
                I2C_send(0x3E, test_LCD, longitud);                      //Ho Envia
                delay(10);
                move_cursor(0x05);
                delay(10);
                longitud = sprintf(test_LCD,"@B");                       //Posa el valor L de LEFT en el LCD
                I2C_send(0x3E, test_LCD, longitud);                      //Ho envia
                delay(10);
                move_cursor(0x00);

                break;
            case 4: // RIGHT
                delay(20);
                mov_motor(0x01, config_velocitat, 0x01, 0x00);          //Mou el motor a RIGHT amb la velocitat configurada
                move_cursor(0x0E);                                      //Mou el cursor a la posició 0x0E
                delay(10);
                longitud = sprintf(test_LCD, "@%d",Array_motor[1]);     //Posa el valor del motor en `test_LCD`
                I2C_send(0x3E, test_LCD, longitud);                     //Ho Envia
                delay(10);
                move_cursor(0x05);
                delay(10);
                longitud = sprintf(test_LCD,"@R");                      //Posa el valor R de RIGHT en el LCD
                I2C_send(0x3E, test_LCD, longitud);                     //Ho envia
                delay(10);
                move_cursor(0x00);

                break;
            case 5: // BUTTON
                delay(10);
                longitud = sprintf(test_LCD, "@BOTTOM \n ");            //Possa BOTTOM en el LCD
                I2C_send(0x3E, test_LCD, longitud);                     //Ho envia
                delay(10);
                Menu_position=0;                                        //Restableix la posició del menú
                Modo=0;                                                 //Torna al mode inicial
                joystick_state=0;                                       //Restableix l'estat del joystick
                Number_moni=0;                                          //Restableix la variable de monitorització


                break;
            default:
                break;
        }
	
	//EN FUNCIÓ DELS MODES ES MODIFICARA EL VALOR DEL JOYSTICK STATE MODO=2 HO DETERMINARA EL JOYSTICK MODO=3 HO DETERMINARA LA LLUM I MODE 4 LA DISTANCIA OBSTACLE

        if (update_ADC){
            J_right = mesura_ADC(5);      //Llegeix el valor del canal 5 de l'ADC
           ilum1 = meas_ADC(5, 10) / 10;  //Llegeix el valor del canal 5 de l'ADC i el divideix per 10
           J_bot = mesura_ADC(4);         //Llegeix el valor del canal 4 de l'ADC
           ilum2 = meas_ADC(4, 10) / 10;  //Llegeix el valor del canal 4 de l'ADC i el divideix per 10

            J_right=J_right*3;
            J_right=J_right/4096;

            J_bot=J_bot*3;
            J_bot=J_bot/4096;

            if(Modo==2){

                if(J_right>1)
                     {
                    joystick_state=4;            //Canvia l'estat del joystick a 4 (Right)
                     }
                else if(J_bot>1)
                     {
                        joystick_state=3;        //Canvia l'estat del joystick a 3 (BOT)
                     }
                else if((J_bot && J_right)==1)
                   {
                       joystick_state=0;         //Canvia l'estat del joystick a 0
                   }
                else
                {
                   joystick_state=joystick_state;//Manté l'estat actual del joystick
                }
            }
            if(Modo==3){
                if (ilum1>350){
                    joystick_state=1;            //Canvia l'estat del joystick a 1 (Top)
                    leds_RGB(config_RGB,0);      //Actualitza els LEDs RGB
                }
                if(ilum2>350){
                    joystick_state=2;            //Canvia l'estat del joystick a 2 (esquerra)
                    leds_RGB(0,config_RGB);      //Actualitza els LEDs RGB
                }
                else if((ilum1<350)& (ilum2<350)){
                    joystick_state=0;           //Canvia l'estat del joystick a 0
                    leds_RGB(0,0);              //Apaga els LEDs
                }
            }

            if((Modo==3)|(Modo==4)){ //VISUALITZA EL DETECTOR DE LLUM PER MODE DE COLISIO I DE LLUMINOSITAT
                delay(10);
                move_cursor(0x3A);                                  //Mou el cursor a la posició 0x3A
                delay(10);
                longitud = sprintf(test_LCD, "@ ");                 //Envia un espai en blanc al LCD per després escriure
                I2C_send(0x3E, test_LCD, longitud);
                delay(100);
                move_cursor(0x3A);
                delay(10);
                longitud = sprintf(test_LCD, "@%d/%d",ilum1,ilum2); //Posa els valors de `ilum1` i `ilum2`
                I2C_send(0x3E, test_LCD, longitud);                 //Envia aquests valors al LCD
                delay(10);
        }

        }

        if(update_ultrasound){
            delay(10);
            ultrasound=ultrasons();                          //Llegeix el valor del sensor d'ultrasons
            delay(10);
            move_cursor(0x35);
            delay(10);
            longitud = sprintf(test_LCD, "@   ",ultrasound);//Posa espais en blanc en el LCD per després utilitzarlos
            I2C_send(0x3E, test_LCD, longitud);
            delay(10);
            move_cursor(0x35);
            delay(10);
            longitud = sprintf(test_LCD, "@%d",ultrasound); //Posa el valor de ultrasound
            I2C_send(0x3E, test_LCD, longitud);             //Envia aquest valor al LCD
            delay(10);
            if(Modo==4){
                if(ultrasound>10){
                    joystick_state=1;                       //Canvia l'estat del joystick a 1 (top)
                    leds_RGB(config_RGB,config_RGB);        //Actualitza els LEDs RGB
                }
                else{
                    joystick_state=2;                       //Canvia l'estat del joystick a 2 (LEFT)
                    leds_RGB(1,1);                          //actualitza els LEDs

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

