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

void keys(int16_t x, int16_t y, int16_t w, int16_t h, int16_t font, uint16_t options,const char* s)
{
    //funcion que dibuja un teclado
    int largo = strlen(s);
    int contador_cadena, xaux;
    char aux;

    if(largo%2==0) xaux = HSIZE/2-(largo/2)*w;
    else xaux = HSIZE/2-((largo/2)*w+w/2);

    if(options==OPT_CENTER)
    {
        if(largo%2==0) xaux = HSIZE/2-(largo/2)*w;
        else xaux = HSIZE/2-((largo/2)*w+w/2);
    }
    else xaux=x;

    for(contador_cadena=0;contador_cadena<largo;contador_cadena++)
    {
        strncpy(&aux,s+contador_cadena,1);
        Boton(xaux+contador_cadena*w, y, w, h, font, &aux);//&aux[contador_cadena]);//*logico (pal 3 de la coord x)
        UARTprintf("%c", aux);
    }
}

void teclado(int16_t vm)
{
    int font;
    if(vm==35) font=26;
    else font=27;

    keys(3,VSIZE/2,HSIZE/10,VSIZE/8,font,OPT_CENTER,"QWERTYUIOP");
    keys(3,VSIZE/2+VSIZE/8+3,HSIZE/10,VSIZE/8,font,OPT_CENTER,"ASDFGHJKL");
    keys(3,VSIZE/2+2*VSIZE/8+6,HSIZE/10,VSIZE/8,font,OPT_CENTER,"ZXCVBNM");
    Boton(HSIZE/2-60,VSIZE/2+3*VSIZE/8+9,120,VSIZE/8,font,"");
}

void main(){

//------------------------------CONFIGURACIONES E INICIALIZACIONES----------------------------------
    RELOJ=SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);

    //iniciamos pantalla
    HAL_Init_SPI(1, RELOJ);
    Inicia_pantalla();
    //una espera para dar tiempo a la pantalla
    SysCtlDelay(RELOJ/3);

    //habilitacion y configuracion de la uart
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);//
    UARTStdioConfig(0, 115200, RELOJ);

    //habilitacion y configuracion de la uart
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);//
    UARTStdioConfig(0, 115200, RELOJ);

//-------------------------------PROGRAMA-------------------------------------------------------------

    while(1){
    //pintamos pantalla inicial
    //fondo
        Lee_pantalla();
        Nueva_pantalla(16,16,16);

        teclado(35);

        Dibuja();
    }
}
