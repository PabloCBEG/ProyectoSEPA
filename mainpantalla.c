/*
 * main.c
 */


#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>


#include "driverlib2.h"
#include "utils/uartstdio.h"

#include "HAL_I2C.h"
#include "sensorlib2.h"

#include "FT800_TIVA.h"



// =======================================================================
// Function Declarations
// =======================================================================
#define dword long
#define byte char



#define PosMin 750
#define PosMax 1000

#define XpMax 286
#define XpMin 224
#define YpMax 186
#define YpMin 54

unsigned int Yp=120, Xp=245;

//Macros para facilitar la lectura de los botones
#define B1_OFF GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0)
#define B1_ON !(GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0))
#define B2_OFF GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1)
#define B2_ON !(GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1))

#define BOOSTERPACK_PANTALLA 1
#define BOOSTERPACK_SENSORES 2


// =======================================================================
// Variable Declarations
// =======================================================================

char chipid = 0;                        // Holds value of Chip ID read from the FT800

unsigned long cmdBufferRd = 0x00000000;         // Store the value read from the REG_CMD_READ register
unsigned long cmdBufferWr = 0x00000000;         // Store the value read from the REG_CMD_WRITE register
unsigned int t=0;
// ############################################################################################################
// User Application - Initialization of MCU / FT800 / Display
// ############################################################################################################

unsigned long POSX, POSY, BufferXY;
unsigned long POSYANT=0;
unsigned int CMD_Offset = 0;
unsigned long REG_TT[6];
const int32_t REG_CAL[6]={21696,-78,-614558,498,-17021,15755638};
const int32_t REG_CAL5[6]={32146, -1428, -331110, -40, -18930, 18321010};



#define NUM_SSI_DATA            3
#define MSEC 40000

int RELOJ;
int estado;


uint32_t Puerto[]={
        GPIO_PORTN_BASE,
        GPIO_PORTN_BASE,
        GPIO_PORTF_BASE,
        GPIO_PORTF_BASE, //Puertos de los leds

};
uint32_t Pin[]={
        GPIO_PIN_1,
        GPIO_PIN_0,
        GPIO_PIN_4,
        GPIO_PIN_0,
        }; //Pines de los leds

//Periodos para los timers
uint32_t periodo0_global; //Timer 0
uint32_t periodo1_global; //Timer 1
uint32_t periodo2_global; //Timer 2
uint32_t periodo3_global; //Timer 3

//Para el numero de monedas que se han de devolver
int numero_monedas_20;
int numero_monedas_5;
//Para el modulo PWM
volatile int Max_pos = 4200; //3750
volatile int Min_pos = 1300; //1875
int PeriodoPWM;
volatile int pos_100;
volatile int pos_1000;
int posicion; //Posicion del servo -> Ancho de pulso del PWM


//VARIABLES GLOBALES

//PARA EL ESTADO 1

int cuadro_seleccionado_estado_1 = 0; // 0 -> Campo nombre, 1 -> Campo contraseña
int parpadeo_cuadrado_seleccionado_estado_1 = 0; //Para que el cuadro seleccionado haga un parpadeo



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

volatile long int ticks=0;
uint8_t Sensor_OK=0;
uint8_t Opt_OK, Tmp_OK, Bme_OK, Bmi_OK;

void IntTick(void){
    ticks++;
}

//Para definir un widget
//Mirar libreria de programacion FT800
//ft800.tiva.c



//FUNCIONES

//TIMERS
void Timer0IntHandler(void)
{
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT); //Borra flag
    Cambia=1;
    SysCtlDelay(100);
}
void Timer1IntHandler(void)
{
    TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
    if(parpadeo_cuadrado_seleccionado_estado_1 == 0)
    {
        parpadeo_cuadrado_seleccionado_estado_1 = 1;
    }
    else
    {
        parpadeo_cuadrado_seleccionado_estado_1 = 0;
    }
    SysCtlDelay(100);
}

int pinta_boton( int16_t x,int16_t y,int16_t w,int16_t h,int16_t font,char* s)
{

    /*
     * x -> Coord. x centro del boton
     * y -> Coord. y centro del boton
     * w -> Ancho
     * h -> Alto
     * font -> Fuente
     * options ->Opciones posicionamento cadena
     * s -> Cadena
     *
     */
    int16_t x_esquina_sup_izquierda = x - (w/2);
    int16_t y_esquina_sup_izquierda = y - (h/2);


    int resul;
    Lee_pantalla();
    if(POSX>x_esquina_sup_izquierda && POSX<(x_esquina_sup_izquierda+w) && POSY>y_esquina_sup_izquierda && POSY<(y_esquina_sup_izquierda+h)){
        ComButton(x_esquina_sup_izquierda,y_esquina_sup_izquierda,w,h,font,OPT_FLAT,s);
        resul=1;
    }else{
        ComButton(x_esquina_sup_izquierda,y_esquina_sup_izquierda,w,h,font,0,s);
        resul=0;
    }
    return resul;
}

void interrupcion_botones(void)
{
    int boton_activado = GPIOIntStatus(GPIO_PORTJ_BASE, true);
    if(boton_activado == 1)
    {

        while(B1_ON)
        {

        }
        SysCtlDelay(10*MSEC);
        if(estado == 1) //Estado de creacion de usuario
        {
            if(cuadro_seleccionado_estado_1 == 0)
            {
                cuadro_seleccionado_estado_1 = 1;
            }
            else
            {
                cuadro_seleccionado_estado_1 = 0;
            }
        }

        GPIOIntClear(GPIO_PORTJ_BASE, GPIO_PIN_0);

    }
    else if(boton_activado == 2)
    {

        while(B2_ON){
            //Esperamos hasta que el boton deje de ser pulsado
        }
        SysCtlDelay(10*MSEC);//Esperamos 10 ms para evitar efectos de bouncing
        GPIOIntClear(GPIO_PORTJ_BASE,GPIO_PIN_1);
    }
}





int main(void) {

    //RELOJ
    RELOJ=SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);

    //BOTONES DE ENTRADA
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ); //DECLARAMOS EL PUERTO DE LOS BOTONES DE ENTRADA
    GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0|GPIO_PIN_1);   //J0 y J1: ENTRADAS
    GPIOPadConfigSet(GPIO_PORTJ_BASE,GPIO_PIN_0|GPIO_PIN_1,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU); //Pullup en J0 y J1
    //Con sus interrupciones
    GPIOIntTypeSet(GPIO_PORTJ_BASE,GPIO_PIN_0|GPIO_PIN_1, GPIO_FALLING_EDGE);
    //Por flanco de bajada
    GPIOIntEnable(GPIO_PORTJ_BASE, GPIO_PIN_0|GPIO_PIN_1); // Habilitamos las interrupciones en esos pines
    GPIOIntRegister(GPIO_PORTJ_BASE, interrupcion_botones); //La funcion que se lanza


    int i;
    //TIMERS
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0); //Timer 0
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1); //Timer 1
    TimerClockSourceSet(TIMER0_BASE, TIMER_CLOCK_SYSTEM);
    TimerClockSourceSet(TIMER1_BASE, TIMER_CLOCK_SYSTEM);
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
    TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC);
    TimerLoadSet(TIMER0_BASE, TIMER_A, RELOJ/20 -1); //Se actualizara el estado de la pantalla cada 0.05 segundos
    TimerLoadSet(TIMER1_BASE, TIMER_A, RELOJ/2-1  ); //Periodo de 0.5 segundos
    TimerIntRegister(TIMER0_BASE, TIMER_A,Timer0IntHandler);
    TimerIntRegister(TIMER1_BASE, TIMER_A,Timer1IntHandler);
    IntEnable(INT_TIMER0A);
    IntEnable(INT_TIMER1A);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
    IntMasterEnable();
    TimerEnable(TIMER0_BASE, TIMER_A);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    UARTStdioConfig(0, 115200, RELOJ);

    //Configuracion del BoosterPack de los sensores
    Conf_Boosterpack(BOOSTERPACK_SENSORES, RELOJ); //Funcion definida en la libreria HAL_I2C.c

    //Comprobacion de los sensores disponibles
    Sensor_OK=Test_I2C_Dir(OPT3001_SLAVE_ADDRESS);
         if(!Sensor_OK)
         {
             UARTprintf("Error en OPT3001\n");
             Opt_OK=0;

         }
         else
         {
             OPT3001_init();
             UARTprintf("Hecho!\n");
             UARTprintf("Leyendo DevID... ");
             DevID=OPT3001_readDeviceId();
             UARTprintf("DevID= 0X%x \n", DevID);
             Opt_OK=1;
         }
         UARTprintf("Inicializando ahora el TMP007...");
         Sensor_OK=Test_I2C_Dir(TMP007_I2C_ADDRESS);
         if(!Sensor_OK)
         {
             UARTprintf("Error  en TMP007\n");
             Tmp_OK=0;
         }
         else
         {
             sensorTmp007Init();
             UARTprintf("Hecho! \nLeyendo DevID... ");
             DevID=sensorTmp007DevID();
             UARTprintf("DevID= 0X%x \n", DevID);
             sensorTmp007Enable(true);
             Tmp_OK=1;
         }
         UARTprintf("Inicializando BME280... ");
         Sensor_OK=Test_I2C_Dir(BME280_I2C_ADDRESS2);
         if(!Sensor_OK)
         {
             UARTprintf("Error en BME280\n");
             Bme_OK=0;
         }
         else
         {
             bme280_data_readout_template();
             bme280_set_power_mode(BME280_NORMAL_MODE);
             UARTprintf("Hecho! \nLeyendo DevID... ");
             readI2C(BME280_I2C_ADDRESS2,BME280_CHIP_ID_REG, &DevID, 1);
             UARTprintf("DevID= 0X%x \n", DevID);
             Bme_OK=1;
         }
         Sensor_OK=Test_I2C_Dir(BMI160_I2C_ADDR2);
         if(!Sensor_OK)
         {
             UARTprintf("Error en BMI160\n");
             Bmi_OK=0;
         }
         else
         {
             UARTprintf("Inicializando BMI160, modo NAVIGATION... ");
             bmi160_initialize_sensor();
             bmi160_config_running_mode(APPLICATION_NAVIGATION);
             UARTprintf("Hecho! \nLeyendo DevID... ");
             readI2C(BMI160_I2C_ADDR2,BMI160_USER_CHIP_ID_ADDR, &DevID, 1);
             UARTprintf("DevID= 0X%x \n", DevID);
             Bmi_OK=1;
         }


    //Inicializacion de la pantalla
    HAL_Init_SPI(BOOSTERPACK_PANTALLA,RELOJ);
    Inicia_pantalla();
    SysCtlDelay(RELOJ/3);

    //Pantalla inicial
    Nueva_pantalla(16,16,16);
    ComColor(21,160,6);
    ComLineWidth(5);
    ComRect(10, 10, HSIZE-10, VSIZE-10, true);
    ComColor(65,202,42);
    ComRect(12, 12, HSIZE-12, VSIZE-12, true);

    ComColor(255,255,255);
    ComTXT(HSIZE/2,VSIZE/5, 22, OPT_CENTERX,"Asistente de Recetas");
    ComTXT(HSIZE/2,50+VSIZE/5, 22, OPT_CENTERX," SEPA GIERM. 2021 ");
    ComTXT(HSIZE/2,100+VSIZE/5, 20, OPT_CENTERX,"Por favor, pulse la pantalla");

    ComRect(40,40, HSIZE-40, VSIZE-40, false);
    Dibuja();
    Espera_pant();

#ifdef VM800B35
    for(i=0;i<6;i++)    Esc_Reg(REG_TOUCH_TRANSFORM_A+4*i, REG_CAL[i]);
#endif
#ifdef VM800B50
    for(i=0;i<6;i++)    Esc_Reg(REG_TOUCH_TRANSFORM_A+4*i, REG_CAL5[i]);
#endif


    SysTickIntRegister(IntTick);
    SysTickPeriodSet(12000);
    SysTickIntEnable();
    SysTickEnable();

    estado = 0;
    while(1)
    {
        if(Cambia==1){
            Cambia=0;

            ticks=0;

            Lee_pantalla();

            /*
            if(Opt_OK)
            {
                lux=OPT3001_getLux();
                lux_i=(int)round(lux);
            }
            if(Tmp_OK)
            {
                sensorTmp007Read(&T_amb, &T_obj);
                sensorTmp007Convert(T_amb, T_obj, &Tf_obj, &Tf_amb);
                T_amb_i=(short)round(Tf_amb);
                T_obj_i=(short)round(Tf_obj);
            }
            if(Bme_OK)
            {
                returnRslt = bme280_read_pressure_temperature_humidity(
                        &g_u32ActualPress, &g_s32ActualTemp, &g_u32ActualHumity);
                T_act=(float)g_s32ActualTemp/100.0;
                P_act=(float)g_u32ActualPress/100.0;
                H_act=(float)g_u32ActualHumity/1000.0;
            }
            if(Bmi_OK)
            {
                bmi160_bmm150_mag_compensate_xyz(&s_magcompXYZ);
                bmi160_read_accel_xyz(&s_accelXYZ);
                bmi160_read_gyro_xyz(&s_gyroXYZ);
            }
            tiempo=ticks;
            */
            Nueva_pantalla(16,16,16);
            ComColor(12, 183, 242); //Valores RGB del celeste del fondo de la pantalla
            ComLineWidth(5);
            ComRect(1, 1, HSIZE-1, VSIZE-1, true);

            switch(estado)
            {
                case 0:
                    //Pantalla inicial. Creacion de usuario o registro
                    ComColor(255,255,255); // Su texto estara en blanco
                    ComTXT(HSIZE/2, VSIZE/4, 22, OPT_CENTER, "Asistente de recetas");
                    ComTXT(HSIZE/2, VSIZE/2, 22, OPT_CENTER, "Seleccione una opcion");

                    if(pinta_boton(HSIZE/4, VSIZE*3/4, 120, 25, 18,"Registrarse"))
                    {
                        estado = 1;
                        TimerEnable(TIMER1_BASE, TIMER_A); //Lanzamos el timer de periodo de 0.5 segundos, para el parpadeo
                    }
                    if(pinta_boton(HSIZE*3/4, VSIZE*3/4, 120, 25, 18,"Iniciar sesion"))
                    {
                        estado = 2;
                    }

                    break;
                case 1:
                    //Pantalla de registro de usuario
                    ComColor(255,255,255);
                    ComTXT(HSIZE/4, VSIZE/8,16,OPT_CENTER,"Nombre :");
                    ComTXT(HSIZE/4, VSIZE/4,16,OPT_CENTER,"Contrasena:");
                    ComTXT(HSIZE*1/2, (VSIZE/4)+30,16,OPT_CENTER,"Pulse B1 para cambiar campo de escritura");
                    if(cuadro_seleccionado_estado_1 == 0)
                    {
                        ComColor(255,255,255);
                        ComRect(HSIZE/2,(VSIZE/4)-10, HSIZE, (VSIZE/4)+10, true); //Cuadro de contraseña
                        if(parpadeo_cuadrado_seleccionado_estado_1 == 0)
                        {
                            ComColor(100,100,100);
                            ComRect(HSIZE/2,(VSIZE/8)-10, HSIZE, (VSIZE/8)+10, true); //Cuadro de nombre
                        }
                        else
                        {
                            ComColor(255,255,255);
                            ComRect(HSIZE/2,(VSIZE/8)-10, HSIZE, (VSIZE/8)+10, true); //Cuadro de nombre
                        }
                    }
                    else if(cuadro_seleccionado_estado_1 == 1)
                    {
                        ComColor(255,255,255);
                        ComRect(HSIZE/2,(VSIZE/8)-10, HSIZE, (VSIZE/8)+10, true); //Cuadro de nombre

                        if(parpadeo_cuadrado_seleccionado_estado_1 == 0)
                        {
                            ComColor(100,100,100);
                            ComRect(HSIZE/2,(VSIZE/4)-10, HSIZE, (VSIZE/4)+10, true); //Cuadro de contraseña
                        }
                        else
                        {
                            ComColor(255,255,255);
                            ComRect(HSIZE/2,(VSIZE/4)-10, HSIZE, (VSIZE/4)+10, true); //Cuadro de contraseña
                        }
                    }


                default:
                    break;
            }




            Dibuja();
        }

    }



    return 0;
}

