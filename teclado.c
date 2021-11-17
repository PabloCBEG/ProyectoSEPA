

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>


#include "driverlib2.h"
#include "utils/uartstdio.h"

//#include "HAL_I2C.h"
//#include "sensorlib2.h"
/***********************************
 * Ejemplo de manejo del PWM. Se configura para que un servo en PF1 cambie
 * Mientras se tenga pulsado un botón, cambia la consigna del servo
 * Se usa para ello el PWM1, con un periodo de 50Hz, y un duty cycle entre 1ms y 2ms aprox
 * Se usan variables globales para poder cambiarlo en GUiComposer
 *************************************/

#define MSEC 40000
#define MaxEst 10


#define B1_OFF GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0)
#define B1_ON !(GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0))
#define B2_OFF GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1)
#define B2_ON !(GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1))


volatile int Max_pos = 4200; //3750
volatile int Min_pos = 1300; //1875

int RELOJ, PeriodoPWM;
volatile int pos_100, pos_1000;
volatile int posicion;

int estado;
uint32_t periodo_global;
uint32_t periodo_global_100ms;




void IntTimer0(void)
{
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT); // Borra flag

    //SysCtlDelay(10*MSEC);
    estado = 3;
    //SysCtlDelay(10*MSEC);



}


int main(void)
{


    RELOJ=SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);




    //Configuracion del mperiferioc UART
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);



    UARTStdioConfig(0, 115200, RELOJ);


    IntMasterEnable();


    UARTprintf("Fin de la configuracion.\n");

    FILE *fp;
    fp = fopen("uno.txt","r");

    char buff[200];
    char texto[5][200];
    int i,j;
    UARTprintf("Fichero de texto abierto.\n");
    if (fp == NULL)
    {
        UARTprintf("Error en la apertura del fichero.\n");
        exit(1);
    }
    else
    {
        UARTprintf("Apertura realizada con exito.\n");
        i = 0; //Para ir contando
        while (feof(fp) == 0)
        {
            fgets(buff,200,fp);
            UARTprintf("%s",buff);
            strcpy(texto[i],buff);
            i++;
        }

        //fscanf(fp, "%s" ,buffer);
        //UARTprintf("%s\n",buffer);
        //UARTprintf("%s\n",buff);
    }
    UARTprintf("\n");
    UARTprintf("Fin de la lectura del archivo.\n");
    for(j=0;j<i;j++)
    {
        UARTprintf("%s\n",texto[j]);
    }
    fclose(fp);



    UARTprintf("Fin de la ejecucion.\n");
    return 0;
}


