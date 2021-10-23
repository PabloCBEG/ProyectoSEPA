/*
 * main.c
 */


#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include <string.h>


#include "driverlib2.h"
#include "utils/uartstdio.h"

#include "HAL_I2C.h"
#include "sensorlib2.h"

#define MSEC 40000 //Valor para 1ms con SysCtlDelay()
#define B1_OFF GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0)
#define B1_ON !(GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0))
#define B2_OFF GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1)
#define B2_ON !(GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1))



// =======================================================================
// Function Declarations
// =======================================================================


int RELOJ;


void Timer0IntHandler(void);
void checkeo_sensores(void);
void rutina_interrupcion(void);


char Cambia=0;

float lux;
char string[80];
int DevID=0;

int16_t T_amb, T_obj;

 float Tf_obj, Tf_amb;
 int lux_i, T_amb_i, T_obj_i;

 // BME280
 int returnRslt;
 int g_s32ActualTemp   = 0;
 unsigned int g_u32ActualPress  = 0;
 unsigned int g_u32ActualHumity = 0;
// struct bme280_t bme280;

 // BMI160/BMM150
 int8_t returnValue;
 struct bmi160_gyro_t        s_gyroXYZ;
 struct bmi160_accel_t       s_accelXYZ;
 struct bmi160_mag_xyz_s32_t s_magcompXYZ;


 //Calibration off-sets
 int8_t accel_off_x;
 int8_t accel_off_y;
 int8_t accel_off_z;
 int16_t gyro_off_x;
 int16_t gyro_off_y;
 int16_t gyro_off_z;
 float T_act,P_act,H_act;
 bool BME_on = true;

 int T_uncomp,T_comp;
char mode;
long int inicio, tiempo;

int estado; //VARIABLE PARA DEFINIR EN QUE ESTADO NOS ENCONTRAMOS DE LA MAQUINA DE ESTADO

volatile long int ticks=0;
uint8_t Sensor_OK=0;
#define BP 2
uint8_t Opt_OK, Tmp_OK, Bme_OK, Bmi_OK;

void IntTick(void){
    ticks++;
}


void rutina_interrupcion(void){
    int boton_activado = GPIOIntStatus(GPIO_PORTJ_BASE, true);

    if (boton_activado == 1){ //SE HA PULSADO EL BOTON B1
        while(B1_ON){
            //ESPERAMOS A QUE DEJE DE SER ACTIVADO
        }
        SysCtlDelay(10*MSEC);//ESPERAMOS 10 MS PARA EVITAR EFECTOS DE BOUNCING
        estado++;
        if (estado == 5){
            estado = 1;
        }
        GPIOIntClear(GPIO_PORTJ_BASE, GPIO_PIN_0);//LIMPIAMOS EL PIN DE LA INTERRUPCION
    }
    else if(boton_activado == 2){ //SE HA PULSADO EL BOTON B2
        while(B2_ON){
                    //ESPERAMOS A QUE DEJE DE SER ACTIVADO
        }
        SysCtlDelay(10*MSEC);//ESPERAMOS 10 MS PARA EVITAR EFECTOS DE BOUNCING
        estado--;
        if (estado == 0){
            estado = 4;
        }
        GPIOIntClear(GPIO_PORTJ_BASE, GPIO_PIN_1);//LIMPIAMOS EL PIN DE LA INTERRUPCION
    }
}

void checkeo_sensores(void){



         int numero_sensores = 4;
         int fallo_sensores[numero_sensores];
         int i;
         for (i=0;i<numero_sensores;i++){
             fallo_sensores[i] = 0;
         }

         Sensor_OK=Test_I2C_Dir(OPT3001_SLAVE_ADDRESS);//PARA COMPORBAR SI EL SENSOR DE LUZ FUNCIONA CORRECTAMENTE
         if(!Sensor_OK)
         {
             fallo_sensores[0] = 1;
         }
         else
         {
             OPT3001_init(); //INICIALIZAS EL SENSOR DE LUZ
             //DevID=OPT3001_readDeviceId();
             //UARTprintf("DevID= 0X%x \n", DevID); //MUESTRA LA DIRECCION EN HEXADECIMAL

         }
         Sensor_OK=Test_I2C_Dir(TMP007_I2C_ADDRESS);
         if(!Sensor_OK)
         {
             fallo_sensores[1] = 1;
         }
         else
         {
             sensorTmp007Init();
             /*UARTprintf("Hecho! \nLeyendo DevID... ");
             DevID=sensorTmp007DevID();
             UARTprintf("DevID= 0X%x \n", DevID);
             sensorTmp007Enable(true);
             */
         }

         Sensor_OK=Test_I2C_Dir(BME280_I2C_ADDRESS2);
         if(!Sensor_OK)
         {
             fallo_sensores[2] = 1;

         }
         else
         {
             bme280_data_readout_template();
             bme280_set_power_mode(BME280_NORMAL_MODE);

             /*UARTprintf("Hecho! \nLeyendo DevID... ");
             readI2C(BME280_I2C_ADDRESS2,BME280_CHIP_ID_REG, &DevID, 1);
             UARTprintf("DevID= 0X%x \n", DevID);*/

         }
         Sensor_OK=Test_I2C_Dir(BMI160_I2C_ADDR2);
         if(!Sensor_OK)
         {
             fallo_sensores[3] = 1;

         }
         else
         {

             bmi160_initialize_sensor();
             bmi160_config_running_mode(APPLICATION_NAVIGATION);

             /*UARTprintf("Hecho! \nLeyendo DevID... ");
             readI2C(BMI160_I2C_ADDR2,BMI160_USER_CHIP_ID_ADDR, &DevID, 1);
             UARTprintf("DevID= 0X%x \n", DevID);*/
         }


         char nombres_sensores[4][100];
         strcpy(nombres_sensores[0],"OPT3001");
         strcpy(nombres_sensores[1],"TMP007");
         strcpy(nombres_sensores[2],"BME280");
         strcpy(nombres_sensores[3],"BMI160-BMM150");

         for (i=0;i<numero_sensores;i++){
             if(fallo_sensores[i] == 1){
                 UARTprintf("Ha fallado el sensor: %s\n",&nombres_sensores[i]);
             }
         }

}
int main(void) {


    //DEFINICION DEL RELOJ
    RELOJ=SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);

    //BOTONES
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ); //DECLARAMOS EL PUERTO DE LOS BOTONES DE ENTRADA
    GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0|GPIO_PIN_1);   //J0 y J1: ENTRADAS
    GPIOPadConfigSet(GPIO_PORTJ_BASE,GPIO_PIN_0|GPIO_PIN_1,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU); //Pullup en J0 y J1
       //Por tanto, dichas entradas estaran forzadas a un nivel alto. Por tanto, cuando el boton sea pulsado, la tension de esa entrada tendra un valor bajo
       //Por tanto, cuando leamos esa entrada, si leemos un valor false, significa que el boton esta siendo pulsado
       //Si leemos un valor true, significa que no esta pulsado y sigue en valor alto


    estado = 0; //PARTIMOS DEL ESTADO INICIAL 0

    GPIOIntTypeSet(GPIO_PORTJ_BASE,GPIO_PIN_0|GPIO_PIN_1,GPIO_FALLING_EDGE); // DEFINIMOS EL TIMPO DE INTERRUPCION, POR FLANCO DE BAJADA
    GPIOIntEnable(GPIO_PORTJ_BASE, GPIO_PIN_0|GPIO_PIN_1);  // Habilitar pines de interrupción J0, J1
    GPIOIntRegister(GPIO_PORTJ_BASE, rutina_interrupcion);  //REGISTRAMOS LA FUNCION QUE SERA LLAMADA EN LA INTERRUPCION
    IntEnable(INT_GPIOJ);                                   //HABILITAMOS LAS INTERRUPCIONES DEL PUERTO J

    //LEDS
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0 |GPIO_PIN_4); //F0 y F4: salidas
    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0 |GPIO_PIN_1); //N0 y N1: salidas


    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);//HABILITA EL PERIFERICO TIMER 0
    TimerClockSourceSet(TIMER0_BASE, TIMER_CLOCK_SYSTEM);
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
    TimerLoadSet(TIMER0_BASE, TIMER_A, RELOJ/2 -1);
    TimerIntRegister(TIMER0_BASE, TIMER_A,Timer0IntHandler);
    IntEnable(INT_TIMER0A);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);


    IntMasterEnable();
    TimerEnable(TIMER0_BASE, TIMER_A);
    //HABILITA TIMERS E INTERRUPCIONES

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);//HABILITA EL PUERTO A
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);//HABILITA EL PERIFERICO UART 0

    GPIOPinConfigure(GPIO_PA0_U0RX); //DEFINE EL PIN 0 DEL PUERTO A COMO RX
    GPIOPinConfigure(GPIO_PA1_U0TX); //DEFINE EL PIN 1 DEL PUERTO A COMO TX
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    UARTStdioConfig(0, 115200, RELOJ);


    //COMIENZA LA MAQUINA DE ESTADOS

    //PARA DETECTAR EN QUE BOOSTERPACK HEMOS CONECTADO
        if(Detecta_BP(1))
                 {
                 UARTprintf("\n--------------------------------------");
                 UARTprintf("\n  BOOSTERPACK detectado en posicion 1");
                 UARTprintf("\n   Configurando puerto I2C0");
                 UARTprintf("\n--------------------------------------");
                 Conf_Boosterpack(1, RELOJ);
                 }
         else if(Detecta_BP(2))
                 {
                 UARTprintf("\n--------------------------------------");
                 UARTprintf("\n  BOOSTERPACK detectado en posicion 2");
                 UARTprintf("\n   Configurando puerto I2C2");
                 UARTprintf("\n--------------------------------------");
                 Conf_Boosterpack(2, RELOJ);
                 }
         else
                 {
                 UARTprintf("\n--------------------------------------");
                 UARTprintf("\n  Ningun BOOSTERPACK detectado   :-/  ");
                 UARTprintf("\n              Saliendo");
                 UARTprintf("\n--------------------------------------");
                 return 0;
                 }








    //SysTickIntRegister(IntTick);
    //SysTickPeriodSet(12000);
    //SysTickIntEnable();
    //SysTickEnable();

    /*while(1)
    {
        switch (estado){
                case 0:
                    GPIOPinWrite(GPIO_PORTN_BASE,GPIO_PIN_0,1);
                    GPIOPinWrite(GPIO_PORTN_BASE,GPIO_PIN_1,1);
                    GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_0,1);
                    GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_4,1);
                    SysCtlDelay(1000*MSEC);

                    GPIOPinWrite(GPIO_PORTN_BASE,GPIO_PIN_0,GPIO_PIN_0*0);
                    GPIOPinWrite(GPIO_PORTN_BASE,GPIO_PIN_1,GPIO_PIN_1*0);
                    GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_0,GPIO_PIN_1*0);
                    GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_4,GPIO_PIN_4*0);

                    SysCtlDelay(10*MSEC);
                    checkeo_sensores();
                    break;
                case 1:
                    break;
                case 2:
                    break;
                case 3:
                    break;
                case 4:
                    break;
                case 5:
                    break;

            }


    }
    */



}


void Timer0IntHandler(void)
{
	TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT); //Borra flag
	Cambia=1;
	SysCtlDelay(100);
}

