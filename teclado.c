//VAMOS A HACER UNA FUNCION QUE SAQUE POR PANTALLA UN TECLADO

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "driverlib2.h"
#include "utils/uartstdio.h"
#include "sensorlib2.h"
#include "FT800_TIVA.h"

#define VM800B35
#define B1_OFF GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0)
#define B1_ON !(GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0))
#define B2_OFF GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1)
#define B2_ON !(GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1))

char chipid = 0;                        // Holds value of Chip ID read from the FT800

unsigned long cmdBufferRd = 0x00000000;         // Store the value read from the REG_CMD_READ register
unsigned long cmdBufferWr = 0x00000000;         // Store the value read from the REG_CMD_WRITE register
unsigned int t=0;

//inicializacion del display de FT800
unsigned long POSX, POSY, BufferXY;
unsigned long POSYANT=0;
unsigned int CMD_Offset = 0;
unsigned long REG_TT[6];
const int32_t REG_CAL[6]={21696,-78,-614558,498,-17021,15755638};
const int32_t REG_CAL5[6]={32146, -1428, -331110, -40, -18930, 18321010};

#define NUM_SSI_DATA 3
#define MSEC 40000

int RELOJ;
int RELOJ, bkspc, letra;

void keys(int16_t x, int16_t y, int16_t w, int16_t h, int16_t font, uint16_t options,const char* s);
//----------------------------------------DECLARACION DE FUNCIONES-------------------------------------------------------------------

char *texto_introducido;
int flag_pulsado; //Para letras del teclado
int flag_espacio_pulsado; //Para el espacio
int flag_delete_pulsado; //Para el delete
char tecla_pulsada;
int primera_tecla_pulsada = 0;
int cont_texto_introducido = 0;

void keys(int16_t x, int16_t y, int16_t w, int16_t h, int16_t font, uint16_t options, const char* s)
{
    //funcion que dibuja un teclado
    int largo = strlen(s);

    int contador_cadena, xaux;
    char aux;

    if(largo%2==0)
    {
        xaux = HSIZE/2-(largo/2)*w;
    }
    else
    {
        xaux = HSIZE/2-((largo/2)*w+w/2);
    }

    if(options==OPT_CENTER)
    {
        if(largo%2==0) xaux = HSIZE/2-(largo/2)*w;
        else xaux = HSIZE/2-((largo/2)*w+w/2);
    }
    else xaux=x;
    {
        if(largo%2==0)
        {
            xaux = HSIZE/2-(largo/2)*(w+3);         //3 pixels es el espacio que queremos dejar entre tecla y tecla. Lo ideal seria que
        }
        else
        {
            xaux = HSIZE/2-((largo/2)*(w+3)+w/2);   //se lo pasaramos como parametro y la funcion hiciera sus calculos. Pendiente
        }                                                           //de incorporarlo en la version definitiva
    }



    for(contador_cadena=0;contador_cadena<largo;contador_cadena++)
    {
        strncpy(&aux,s+contador_cadena,1);
        //aux es el caracter que estamos evaluando
        if(Boton(xaux+contador_cadena*w, y, w, h, font, &aux))//&aux[contador_cadena]);//*logico (pal 3 de la coord x)
        {

            //UARTprintf("%c", aux);
            flag_pulsado = 1;
            strncpy(&tecla_pulsada,s+contador_cadena,1);
            //guardamos en el caracter tecla_pulsada el valor de la tecla que se ha pulsado
        }
        else
        {
            if(flag_pulsado == 1 && tecla_pulsada == aux)
            {
                if(primera_tecla_pulsada == 0)
                {
                    strcpy(texto_introducido,&tecla_pulsada);
                    primera_tecla_pulsada = 1;
                }
                else
                {
                    strcat(texto_introducido,&tecla_pulsada);
                }
                flag_pulsado = 0;
                UARTprintf("%c", tecla_pulsada);
                cont_texto_introducido++;
            }
        }

//        logico = 0;
//        if (contador_cadena > 0) logico=1;
//        Boton(xaux+contador_cadena*logico*(w+3), y, w, h, font, &aux);
//        logico = 1;
//        UARTprintf("%c", aux);
    }
}

void teclado(int16_t vm)
{
    int font;
    if(vm==35) font=26;
    else font=27;


    keys(3,VSIZE/2,(HSIZE-30)/10,(VSIZE/2-12)/4,font,OPT_CENTER,"QWERTYUIOP");
    keys(3,VSIZE/2+(VSIZE/2-12)/4+3,(HSIZE-30)/10,(VSIZE/2-12)/4,font,OPT_CENTER,"ASDFGHJKL");
    keys(3,VSIZE/2+2*(VSIZE/2-12)/4+6,(HSIZE-30)/10,(VSIZE/2-12)/4,font,OPT_CENTER,"ZXCVBNM");
    if(Boton(HSIZE/2-60,VSIZE/2+3*(VSIZE/2-12)/4+9,120,(VSIZE/2-12)/4,font," "))
    {
        if(flag_espacio_pulsado == 0)
        {
            flag_espacio_pulsado = 1;
        }

    }
    else
    {
        if(flag_espacio_pulsado == 1)
        {
            int len = strlen(texto_introducido);
            memset( texto_introducido+len, ' ', 1);
            texto_introducido[len + 1] = '\0';
            flag_espacio_pulsado = 0;
        }
    }
    if(Boton(HSIZE-46,VSIZE/2+2*(VSIZE/2-12)/4+6,43,(VSIZE/2-12)/4,font,"<---"))
    {
        if(flag_delete_pulsado == 0)
        {
            flag_delete_pulsado = 1;
        }

    }
    else
    {
        if(flag_delete_pulsado == 1)
        {
            //int long_cadena = strlen(texto_introducido);
            //texto_introducido[long_cadena] = '\0';
            texto_introducido[strlen(texto_introducido)-1] = 0;
            flag_delete_pulsado = 0;
        }
    }


}

void rutina_interrupcion(void)
{
    int boton_activado = GPIOIntStatus(GPIO_PORTJ_BASE, true);

    if (boton_activado == 1){ //Se ha activado el pin 0, es decir, B1
        //La mascara de bits es 00...0001
        while(B1_ON){
            //Esperamos hasta que el boton deje de ser pulsado
        }
        SysCtlDelay(10*MSEC);//Esperamos 10 ms para evitar efectos de bouncing
        UARTprintf("\n");
        UARTprintf(texto_introducido);
        GPIOIntClear(GPIO_PORTJ_BASE, GPIO_PIN_0);//Limpiamos el pin de la interrupcin
    }
    else if (boton_activado== 2){ //Se ha activado el pin 1, es decir, B2
        //La mascarad de bits es 00...0010
        while(B2_ON){
            //Esperamos hasta que el boton deje de ser pulsado
        }
        SysCtlDelay(10*MSEC);//Esperamos 10 ms para evitar efectos de bouncing
        free(texto_introducido);
        texto_introducido = malloc(sizeof(char)*200);
        GPIOIntClear(GPIO_PORTJ_BASE, GPIO_PIN_1);//Limpiamos el pin de la interrupcion
    }
}

void main(){

//------------------------------CONFIGURACIONES E INICIALIZACIONES----------------------------------
    RELOJ=SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
    GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0|GPIO_PIN_1);   //J0 y J1: entradas
    GPIOPadConfigSet(GPIO_PORTJ_BASE,GPIO_PIN_0|GPIO_PIN_1,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU); //Pullup en J0 y J1
    GPIOIntTypeSet(GPIO_PORTJ_BASE,GPIO_PIN_0|GPIO_PIN_1,GPIO_FALLING_EDGE); // Definimos el  tipo de interrupcion: flanco bajada
    //Es decir, que el boton de valor alto a bajo, OJO, NO CUANDO LA VARIABLE PASE DE ALTO A BAJO
    GPIOIntEnable(GPIO_PORTJ_BASE, GPIO_PIN_0|GPIO_PIN_1);  // Habilitar pines de interrupción J0, J1
    GPIOIntRegister(GPIO_PORTJ_BASE, rutina_interrupcion);  //Registramos la funcion que sera llamada cada vez que se provoque una interrupcion en ese puerto
    IntEnable(INT_GPIOJ);                                   //Habilitamos la interrupción del puerto J
    IntMasterEnable();                                      // Habilitar globalmente las interrupciones
    //Configuracion del mperiferioc UART
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);



    UARTStdioConfig(0, 115200, RELOJ);
    //iniciamos pantalla
    HAL_Init_SPI(1, RELOJ);
    Inicia_pantalla();
    //una espera para dar tiempo a la pantalla
    SysCtlDelay(RELOJ/3);
    int i;
#ifdef VM800B35
    for(i=0;i<6;i++)    Esc_Reg(REG_TOUCH_TRANSFORM_A+4*i, REG_CAL[i]);
#endif
#ifdef VM800B50
    for(i=0;i<6;i++)    Esc_Reg(REG_TOUCH_TRANSFORM_A+4*i, REG_CAL5[i]);
#endif
    //Pantalla inicial
    Nueva_pantalla(16,16,16);
    ComColor(21,160,6);
    ComLineWidth(5);
    ComRect(10, 10, HSIZE-10, VSIZE-10, true);
    ComColor(65,202,42);
    ComRect(12, 12, HSIZE-12, VSIZE-12, true);

    ComColor(255,255,255);
    ComTXT(HSIZE/2,VSIZE/5, 22, OPT_CENTERX,"Ejercicio 1 Lab 4");
    ComTXT(HSIZE/2,50+VSIZE/5, 22, OPT_CENTERX," SEPA GIERM. 2021 ");
    ComTXT(HSIZE/2,100+VSIZE/5, 20, OPT_CENTERX,"A.T.M.");

    ComRect(40,40, HSIZE-40, VSIZE-40, false);
    Dibuja();
    Espera_pant();






//-------------------------------PROGRAMA-------------------------------------------------------------

    texto_introducido = malloc(sizeof(char)*200);
    while(1)
    {

    //pintamos pantalla inicial
    //fondo
        Lee_pantalla();
        Nueva_pantalla(16,16,16);

        ComFgcolor(0,122,116);
        ComColor(255, 255, 255);
        teclado(35);

        Dibuja();
    }
}


