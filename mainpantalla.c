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
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>






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

//Variables para el teclado
char *texto_introducido;
char nombre_introducido[200];
char contrasena_introducida[200];
char nombre_leido[200];
char contrasena_leida[200];
int cont_texto_introducido = 0;
int flag_pulsado = 0; //Para letras del teclado
int flag_espacio_pulsado = 0; //Para el espacio
int flag_delete_pulsado = 0; //Para el delete
int primera_tecla_pulsada = 0;
char tecla_pulsada;

//FILES
FILE *usuarios_file;
char *cadena; //Cadena auxiliar para trabajar sobre los archivos de texto


//Variables para las opciones de configuracion
int mostrar_reloj = 0;

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




void interrupcion_botones(void)
{
    int boton_activado = GPIOIntStatus(GPIO_PORTJ_BASE, true);
    if(boton_activado == 1)
    {

        while(B1_ON)
        {

        }
        SysCtlDelay(10*MSEC);
        if(estado == 1 || estado == 2) //Estado de creacion de usuario o inicio de sesion
        {
            //free(texto_introducido); //Al cambiar de campo sobre el que vamos a trabajar, debemos tener en cuenta que cambiamos de string sobre el que trabajamos
            //texto_introducido = malloc(sizeof(char)*200);
            if(cuadro_seleccionado_estado_1 == 0)
            {
                strcpy(nombre_introducido,texto_introducido);
                //strncpy(nombre_introducido,texto_introducido,strlen(texto_introducido));
                cuadro_seleccionado_estado_1 = 1;
                strcpy(texto_introducido,contrasena_introducida);
            }
            else
            {
                strcpy(contrasena_introducida,texto_introducido);
                //strncpy(contrasena_introducida,texto_introducido,strlen(texto_introducido));
                cuadro_seleccionado_estado_1 = 0;
                strcpy(texto_introducido,nombre_introducido);
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
                //UARTprintf("%c", tecla_pulsada);
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

    ComColor(255,255,255);
    ComFgcolor(0, 0, 255);
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


void pinta_reloj(void) //Funcion que pinta un reloj con la hora actual
{

    time_t tiempo_loc = time(0);
    time(&tiempo_loc);
    struct tm *tlocal;
    tlocal = localtime(&tiempo_loc);

    int16_t x = HSIZE*3/4;
    int16_t y = VSIZE/8+10;
    int16_t r = 25;
    uint16_t options = 0;
    uint16_t h = (uint16_t)tlocal->tm_hour+7; //Se ha visto que la hora esta desfasada en 7 horas por detras
    uint16_t m = (uint16_t)tlocal->tm_min;
    uint16_t s = (uint16_t)tlocal->tm_sec;
    uint16_t ms = 0;

    EscribeRam32(CMD_CLOCK);
    EscribeRam16(x);
    EscribeRam16(y);
    EscribeRam16(r);
    EscribeRam16(options);
    EscribeRam16(h);
    EscribeRam16(m);
    EscribeRam16(s);
    EscribeRam16(ms);

}



int main(void) {


    //RELOJ
    RELOJ=SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);

    //BOTONES DE ENTRADA
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ); //DECLARAMOS EL PUERTO DE LOS BOTONES DE ENTRADAw
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
    texto_introducido = malloc(sizeof(char)*200); //Variable de lo que es escribe por texto
    //nombre_introducido = malloc(sizeof(char)*200);
    //contrasena_introducida = malloc(sizeof(char)*200);

    //Comprobacion de que el archivo que queremos usar para escribir los los usuarios y contrasenas existe o no
    cadena = malloc(sizeof(char)*200);
    if(fopen("users.txt","a") != NULL)
    {
        //En el caso de que el archivo exista
        UARTprintf("El archivo de users.txt ya existia.\n");
        usuarios_file = fopen("users.txt","a");
    }
    else
    {
        //En el caso de que el archivo no exista
        UARTprintf("El archivo de users.txt NO existia.\n");
        usuarios_file = fopen("users.txt","w");
        strcpy(cadena,"Usuarios Contrasenas");
        fwrite(cadena,1,strlen(cadena),usuarios_file);

    }

    fclose(usuarios_file);
    free(cadena); //Para liberar espacio en la HEAP
    /*
    size_t primero_escrito = fwrite(cadena,1,strlen(cadena),fp);
    if(primero_escrito != strlen(cadena))
    {
        UARTprintf("Error en la escritura de la primera linea.\n");
        UARTprintf(strerror(errno));
        UARTprintf("\n");
        UARTprintf("El valor de long cadena es : %d \n",strlen(cadena));
        UARTprintf("El valor de primero_escrito es : %d \n",(int)primero_escrito);
    }
    */
    /*
    if(fclose(fp) != 0)
    {
        UARTprintf("Fallo al cerrar el fichero!!!!!!\n");
    }
    else
    {
        UARTprintf("Fichero cerrado correctamente!!!!\n");
    }
    */
    /*
    UARTprintf("Se ha escrito la primera linea y se ha cerrado el archivo.\n");
    UARTprintf("Se va a escribr una segunda linea en el archivo ya creado.\n");
    fp = fopen("archivoescritura.txt","a");
    char *cadena2 = "Segunda linea escrita hoy";
    fwrite("\n",1,1,fp);
    fwrite(cadena2,1,strlen(cadena2),fp);


    if(fclose(fp) != 0) //Lo dejamos cerrado, para que se tenga que abrir para escribir
    {
        UARTprintf("Fallo al cerrar el fichero!!!!!!\n");
    }
    else
    {
        UARTprintf("Fichero cerrado correctamente!!!!\n");
    }
    */
    while(1)
    {
        if(Cambia==1){
            Cambia=0;

            ticks=0;

            Lee_pantalla();
            Nueva_pantalla(16,16,16);
            ComColor(12, 183, 242); //Valores RGB del celeste del fondo de la pantalla
            ComLineWidth(5);
            ComRect(1, 1, HSIZE-1, VSIZE-1, true);

            switch(estado)
            {
                case 0:
                    //Pantalla inicial. Creacion de usuario o registro
                    ComColor(255,255,255); // Su texto estara en blanco
                    ComFgcolor(0, 0, 255);
                    ComTXT(HSIZE/2, VSIZE/4, 22, OPT_CENTER, "Asistente de recetas");
                    ComTXT(HSIZE/2, VSIZE/2, 22, OPT_CENTER, "Seleccione una opcion");

                    if(pinta_boton(HSIZE/4, VSIZE*3/4, 120, 25, 18,"Registrarse"))
                    {
                        //memset(nombre_introducido,0,strlen(nombre_introducido));
                        //memset(contrasena_introducida,0,strlen(contrasena_introducida));
                        TimerEnable(TIMER1_BASE, TIMER_A); //Lanzamos el timer de periodo de 0.5 segundos, para el parpadeo
                        estado = 1;
                    }
                    if(pinta_boton(HSIZE*3/4, VSIZE*3/4, 120, 25, 18,"Iniciar sesion"))
                    {
                        estado = 2;
                    }

                    break;
                case 1:
                    //Pantalla de registro de usuario
                    ComColor(255,255,255);
                    ComTXT((HSIZE/4)+10, VSIZE/8,16,OPT_CENTER,"Nombre :");
                    ComTXT((HSIZE/4)+10, VSIZE/4,16,OPT_CENTER,"Contrasena:");
                    ComTXT(HSIZE*1/2, (VSIZE/4)+30,16,OPT_CENTER,"Pulse B1 para cambiar campo de escritura");
                    teclado(35); //Funcion del teclado
                    if(cuadro_seleccionado_estado_1 == 0)
                    {
                        //Estamos escribiendo sobre el campo del nombre
                        //strcpy(nombre_introducido,texto_introducido); //Copiamos el texto que se introduzca en la variable de nombre introducido
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
                        ComColor(0,0,0);
                        strcpy(nombre_introducido,texto_introducido);
                        ComTXT(HSIZE*3/4, VSIZE/8, 22, OPT_CENTER,nombre_introducido);
                        ComTXT(HSIZE*3/4, VSIZE/4, 22, OPT_CENTER,contrasena_introducida);
                    }
                    else if(cuadro_seleccionado_estado_1 == 1)
                    {
                        //Estamos escribiendo sobre el campo de la contrasena
                        //strcpy(contrasena_introducida,texto_introducido); //Copiamos el texto que se introduzca en la variable de contrasena introducida
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
                        ComColor(0,0,0);
                        strcpy(contrasena_introducida,texto_introducido);
                        ComTXT(HSIZE*3/4, VSIZE/8, 22, OPT_CENTER,nombre_introducido);
                        ComTXT(HSIZE*3/4, VSIZE/4, 22, OPT_CENTER,contrasena_introducida);
                    }
                    //Pintamos el boton para volver

                    ComColor(255,255,255);
                    ComFgcolor(255, 0, 0);
                    if(Boton((HSIZE/10)-20, VSIZE/10, 20, 20, 16, "Back"))
                    {
                        memset(nombre_introducido,0,strlen(nombre_introducido));
                        memset(contrasena_introducida,0,strlen(contrasena_introducida));
                        memset(texto_introducido,0,strlen(texto_introducido));
                        estado = 0;
                    }

                    //Pintamos el boton para guardar

                    ComColor(255, 255, 255);
                    ComFgcolor(0, 255, 0);
                    if(Boton((HSIZE/10)-20,VSIZE/4, 20, 20, 16, "Save"))
                    {
                        UARTprintf("Guardando!!!\n");
                        usuarios_file = fopen("users.txt","a"); //Lo abrimos para a�adir datos al final
                        cadena = malloc(sizeof(char)*200);
                        strcpy(cadena,"\n");
                        fwrite(cadena,1,strlen(cadena),usuarios_file); //Salto de linea
                        strcpy(cadena,nombre_introducido);
                        fwrite(cadena,1,strlen(cadena),usuarios_file);
                        strcpy(cadena," ");
                        fwrite(cadena,1,strlen(cadena),usuarios_file);
                        strcpy(cadena,contrasena_introducida);
                        fwrite(cadena,1,strlen(cadena),usuarios_file);
                        fclose(usuarios_file);
                        free(cadena); //Para liberar espacio en la HEAP
                        memset(nombre_introducido,0,strlen(nombre_introducido));
                        memset(contrasena_introducida,0,strlen(contrasena_introducida));
                        memset(texto_introducido,0,strlen(texto_introducido));
                        estado = 0;
                    }



                    break;

                case 2:
                    //Pantalla de inicio de sesion
                    ComColor(255,255,255);
                    ComTXT((HSIZE/4)+10, VSIZE/8,16,OPT_CENTER,"Nombre :");
                    ComTXT((HSIZE/4)+10, VSIZE/4,16,OPT_CENTER,"Contrasena:");
                    ComTXT(HSIZE*1/2, (VSIZE/4)+30,16,OPT_CENTER,"Pulse B1 para cambiar campo de escritura");
                    teclado(35); //Funcion del teclado
                    if(cuadro_seleccionado_estado_1 == 0)
                    {
                        //Estamos escribiendo sobre el campo del nombre
                        //strcpy(nombre_introducido,texto_introducido); //Copiamos el texto que se introduzca en la variable de nombre introducido
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
                        ComColor(0,0,0);
                        strcpy(nombre_introducido,texto_introducido);
                        ComTXT(HSIZE*3/4, VSIZE/8, 22, OPT_CENTER,nombre_introducido);
                        ComTXT(HSIZE*3/4, VSIZE/4, 22, OPT_CENTER,contrasena_introducida);
                    }
                    else if(cuadro_seleccionado_estado_1 == 1)
                    {
                        //Estamos escribiendo sobre el campo de la contrasena
                        //strcpy(contrasena_introducida,texto_introducido); //Copiamos el texto que se introduzca en la variable de contrasena introducida
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
                        ComColor(0,0,0);
                        strcpy(contrasena_introducida,texto_introducido);
                        ComTXT(HSIZE*3/4, VSIZE/8, 22, OPT_CENTER,nombre_introducido);
                        ComTXT(HSIZE*3/4, VSIZE/4, 22, OPT_CENTER,contrasena_introducida);
                    }
                    //Pintamos el boton para volver
                    ComColor(255,255,255);
                    ComFgcolor(255, 0, 0);
                    if(Boton((HSIZE/10)-20, VSIZE/10, 20, 20, 16, "Back"))
                    {
                        memset(nombre_introducido,0,strlen(nombre_introducido));
                        memset(contrasena_introducida,0,strlen(contrasena_introducida));
                        memset(texto_introducido,0,strlen(texto_introducido));
                        estado = 0;
                    }
                    //Pintamos el boton para guardar

                    ComColor(255, 255, 255);
                    ComFgcolor(0, 255, 0);
                    if(Boton((HSIZE/10)-20,VSIZE/4, 20, 20, 16, "Save"))
                    {
                        int flag_usuario_existe = 0; //Para ver si el nombre introducido existe
                        usuarios_file = fopen("users.txt","r");
                        cadena = malloc(sizeof(char)*200);
                        char buff[200];
                        char texto[5][200];
                        int i,j;
                        if (usuarios_file == NULL)
                        {
                            UARTprintf("Error en la apertura del fichero.\n");
                            exit(1);
                        }
                        else
                        {
                            UARTprintf("Apertura realizada con exito.\n");
                            i = 0; //Para ir contando

                            int flag_espacio_encontrado = 0;
                            int flag_contrasena_leida_correctamente = 0;
                            int pos_espacio = 0; //Para conocer la posicion donde se encuentra el espacio
                            while (feof(usuarios_file) == 0 && (flag_usuario_existe == 0)) //Mientras no hayamos llegado al final del fichero
                            {
                                flag_espacio_encontrado = 0;
                                //fscanf(usuarios_file, _fmt);
                                fgets(cadena,200,usuarios_file);
                                for(j=0;j<strlen(cadena) && flag_espacio_encontrado == 0;j++)
                                {
                                    if(cadena[j] == ' ') //Si el caracter que estamos comparando es un espacio
                                    {

                                        flag_espacio_encontrado = 1; //Hemos encontrado un espacio
                                        pos_espacio = j;
                                        UARTprintf("Espacio encontrado en la posicion : %d.\n",pos_espacio);
                                        int k = 0;
                                        memset(nombre_leido,0,strlen(nombre_leido));
                                        for(k=0;k<pos_espacio;k++)
                                        {
                                            nombre_leido[k] = cadena[k];
                                        }
                                        nombre_leido[pos_espacio] = '\0';
                                        UARTprintf("Nombre leido : %s\n",nombre_leido);
                                    }
                                }

                                if(flag_espacio_encontrado == 1) //En el caso de que hayamos encontrado el espacio
                                {
                                    int k=0;
                                    memset(contrasena_leida,0,strlen(contrasena_leida));
                                    for(j=pos_espacio+1;j<strlen(cadena);j++)
                                    {
                                        contrasena_leida[k] = cadena[j];
                                        k++;
                                    }
                                    contrasena_leida[k+1] = '\0';
                                    UARTprintf("Contrasena leida : %s\n",contrasena_leida);
                                    if(k > 0)
                                    {

                                        flag_contrasena_leida_correctamente = 1; //Si la contrasena se ha leido bien, teniendo mas de un caracter
                                    }
                                }

                                //En el caso de que hayamos encontrado espacio y cadena

                                if(flag_espacio_encontrado == 1 && flag_contrasena_leida_correctamente == 1)
                                {
                                    //Realizamos la comparacion de lo que se ha introducido por teclado con lo que acabamos de leer en el archivo de texto
                                    if(strcmp(nombre_introducido,nombre_leido) == 0)
                                    {
                                        UARTprintf("Nombre leido y nombre introducido son iguales.\n");
                                        if(strcmp(contrasena_introducida,contrasena_leida) == 0)
                                        {
                                            UARTprintf("Contrasena leida y contrasena introducida son iguales.\n");
                                            flag_usuario_existe = 1;
                                        }
                                    }
                                }
                                i++;
                            }
                        }
                        UARTprintf("\n");
                        UARTprintf("Fin de la lectura del archivo.\n");
                        fclose(usuarios_file);

                        free(cadena); //Para liberar espacio en la HEAP
                        memset(nombre_introducido,0,strlen(nombre_introducido)); //Limpieza de las cadenas de strings
                        memset(contrasena_introducida,0,strlen(contrasena_introducida));
                        memset(texto_introducido,0,strlen(texto_introducido));
                        memset(nombre_leido,0,strlen(nombre_leido));
                        memset(contrasena_leida,0,strlen(contrasena_leida));
                        if(flag_usuario_existe == 1)
                        {
                            estado = 3;
                            UARTprintf("Pasando al estado 3.\n");
                        }
                        else
                        {
                            UARTprintf("Los valores de usuario y contrasena no existen.\n");
                            estado = 0;
                        }
                    }
                    break;
                case 3:
                    //En el caso de que se haya introducido un nombre y contrasena correctos en el estado 2
                    //Entramos en la pagina principal

                    ComFgcolor(139,69,19);
                    ComColor(255, 255, 255);
                    //Pintamos el boton de ajustes
                    if(Boton((HSIZE)-50,VSIZE/4, 40, 30, 16, "Ajustes"))
                    {
                        estado = 4; //Pasamos al estado de los ajustes
                    }


                    //Comprobamos los valores de los ajustes
                    if(mostrar_reloj == 1)
                    {
                        //En el caso de que queramos mostrar el reloj
                        pinta_reloj();
                    }
                    break;
                case 4:
                    //Estado de los ajustes
                    //Pintamos boton para mostrar reloj o no
                    if(mostrar_reloj == 0)//Pintamos un cuadro al lado en blanco si no esta seleccionado
                    {
                        ComColor(255, 255, 255);
                        ComLineWidth(5);
                        ComRect((HSIZE/2)+50, (VSIZE/4)-10, (HSIZE/2)+70, (VSIZE/4)+10, true);
                    }
                    else//Pintamos un cuadro al lado en verde si esta seleccionado
                    {
                        ComColor(100, 255, 0); //Valores RGB del celeste del fondo de la pantalla
                        ComLineWidth(5);
                        ComRect((HSIZE/2)+50, (VSIZE/4)-10, (HSIZE/2)+70, (VSIZE/4)+10, true);
                    }

                    ComColor(0,0,0);
                    ComFgcolor(255, 255, 255);
                    if(Boton(((HSIZE)/2)-50,(VSIZE/4)-10, 60, 20, 16, "Pon/Quita reloj"))
                    {
                        if(mostrar_reloj == 0)
                        {
                            mostrar_reloj = 1; //Si no estaba puesto y se pulsa, se debe poner
                        }
                        else
                        {
                            mostrar_reloj = 0; //Si estaba puesto y se pulsa, se debe retirar
                        }
                    }



                    //Pintamos boton para volver
                    ComColor(255,255,255);
                    ComFgcolor(255, 0, 0);
                    if(Boton((HSIZE/10)-20, VSIZE/10, 20, 20, 16, "Back"))
                    {
                        estado = 3;
                    }
                    break;
                default:
                    break;
            }



            Dibuja();
        }

    }



    return 0;
}

