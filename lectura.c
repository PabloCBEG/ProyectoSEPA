#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "driverlib2.h"
#include "utils/uartstdio.h"
#include "sensorlib2.h"
//asqui lo que hace falta es pasar por serie lectura teclado pa indicar paso de renglon, y sacar por serie UART el renglon indicado
//

FILE *fopen( const char * filename, const char * mode );//DEFINICION DE TIPO FILE
int RELOJ;

void main(){
    RELOJ=SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);

    //habilitacion y configuracion de la uart
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);//
    UARTStdioConfig(0, 115200, RELOJ);

        int i;
        FILE *fp;
        char buff[255];
        char texto[255][255];
        fp = fopen("uno.txt","r");
    //    linea = *fgets();
        i=0;
        while(fgets(buff, sizeof(buff), (FILE*)fp)){
            //printf("%s\n", buff); //para imprimir todas las lineas del txt
            strcpy(texto[i],buff);
            i++;
        }
        for(i=0;i<8;i++)
        {
        UARTprintf("%s\n", texto[i]);
        }
        fclose(fp);
}
