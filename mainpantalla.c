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

#include "recetas.h" //Incluimos el fichero de las recetas




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
//----------------------------------------------//
//----------------------------------------------//
//----------------------------------------------//
//Direcciones de los registros en la EEPROM para los ajutes
//----------------------------------------------//
#define DIR_AJUSTES_RELOJ 0x00000000 //0 o 1
//----------------------------------------------//
#define DIR_AJUSTES_TAM_LETRA 0x00000004 //Tamano de la letra
#define TAM_DEFECTO_LETRA 20
#define TAM_MAXIMO_LETRA 31
#define TAM_MINIMO_LETRA 16

int16_t tam_letra; //Variable global para el tamano de letra
//----------------------------------------------//
#define DIR_RC 0X00000008 //Direccion donde se almacena el valor del color rojo de fondo de pantalla
#define DIR_GC 0X0000000C //Direccion donde se almacena el valor del color verde de fondo de pantalla
#define DIR_BC 0X00000010 //Direccion donde se almacena el valor del color azul de fondo de pantalla
#define VALOR_R_DEFECTO 127 //Valor por defecto del rojo, en caso de fallo en la memoria EEPROM
#define VALOR_G_DEFECTO 127 //Valor por defecto del verde, en caso de fallo en la memoria EEPROM
#define VALOR_B_DEFECTO 127 //Valor por defecto del azul, en caso de fallo en la memoria EEPROM

int r_c; //Valores RGB globales para el fondo de pantalla. El valor inicial es un valor de celeste //OOJJJJJJJOOOOOOO SE DEBEN QUITAR LOS VALORES POR DEFECTO
int g_c;
int b_c;

int r_c_temporal; //Valores temporales para los sliders
int g_c_temporal;
int b_c_temporal;
//----------------------------------------------//
//Variables para mostrar la receta
#define DIR_CHECK_RECETA 0x00000014 //Direccion donde chequear si mostrar receta o no
int mostrar_receta = 0; //Flag para determinar si mostrar receta o no
char *receta_en_una_linea; //Para guardar todos los pasos de las receta en un solo string


//PARTE DE MOSTRAR POR PANTALLA (PENSAR SI PONER ALGUNOS VALORES EN MEMORIA DE LA EEPROM, PARA PODER MODIFICAR, COMO LA LONGITUD DE LAS CADENAS
#define longitud 25
//const int longitud = 25;
char lineas[3][longitud]; //Tres lineas por pantalla, de 25 caracteres

int caracter_empezar_a_leer = 0; //Indica por donde empezar a leer, para la primera linea

int flag_termina = 0; //Para indicar que deje de mostrar por pantalla

int puntero_caracter = 0;
int puntero_linea_0 = 0;

//----------------------------------------------//
//Variables para mostrar las recetas disponibles
#define DIR_PUNTERO_RECETA 0x00000018 //Direccion donde se almacena el valor del puntero
char nombre_receta_a_mostrar[longitud];
int puntero_a_nombre_receta; //Puntero al numero de nombre de receta a mostrar

//----------------------------------------------//
//----------------------------------------------//
//----------------------------------------------//
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


//Periodos para los timers
uint32_t periodo0_global; //Timer 0
uint32_t periodo1_global; //Timer 1


//VARIABLES GLOBALES

//PARA EL ESTADO 1

int cuadro_seleccionado_estado_1 = 0; // 0 -> Campo nombre, 1 -> Campo contraseÃ±a
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
char *texto_introducido; //Para el texto que se escribe sobre el teclado
char receta_introducida[200]; //Nombre de la receta introducida
char nombre_introducido[200];
char contrasena_introducida[200];
char nombre_receta_introducida[200];
char nombre_leido[200];
char contrasena_leida[200];
int cont_texto_introducido = 0;
int flag_pulsado = 0; //Para letras del teclado
int flag_espacio_pulsado = 0; //Para el espacio
int flag_delete_pulsado = 0; //Para el delete
int primera_tecla_pulsada = 0;
char tecla_pulsada;





//Variables para las opciones de configuracion
int mostrar_reloj = 0; //Flag para ver si se debe mostrar el reloj
int mostrar_reloj_pulsado = 0; //Flag para que el estado de la variable "mostrar_reloj" se cambie al dejar del pulsar el boton anterior
int cambiar_tam_letra_pulsado = 0;
int cambiar_color_fondo_pulsado = 0;
int elegir_receta_pulsado = 0; //Flag para que se pase a elegir receta solo cuando se suelte el boton
int mostrar_recetas_pulsado = 0; //Flag para que se pase a mostrar las recetas disponibles
//Variables para que los botones de salvar y volver se activen por flanco de bajada
int avanza_e0_pulsado = 0;
int back_e1_pulsado = 0;
int back_e2_pulsado = 0;
int save_e2_pulsado = 0;
int back_e3_pulsado = 0;
int back_e4_pulsado = 0;
int save_e4_pulsado = 0;
int back_e6_pulsado = 0;



//Cadenas que se muestran en la plantilla en blanco
#define longitud 25 //Longitud de las cadenas del texto -->PROGRAMAR SU AJUSTE PARA ALMACENAR VALOR EN EEPROM
char cadena_superior[longitud];
int cadena_superior_activa = 0; //Para saber si pintarla o no

char cadena_medio[longitud];

int cadena_medio_activa = 0;
int x_cadena_medio = HSIZE/2;
int y_cadena_medio = VSIZE/2;

char cadena_inferior[longitud];
int cadena_inferior_activa = 0;

int receta_seleccionada = 0; //Para saber si se tiene que ir mostrando una receta o no
int tamano_cadenas = 16;
int flag_fin_lectura = 0; //Flag para determinar si hemos terminado de leer el archivo txt




//Para el boton de avance en la lectura de la receta
int boton_avanza_pulsado = 0;
int flag_avanza = 0;


//Para definir un widget
//Mirar libreria de programacion FT800
//ft800.tiva.c

//FUNCIONES

//Para el tamano de letra
void IncTamLetra(void);
void DecTamLetra(void);
uint32_t CheckTamLetra(void);
//Para el color de fondo
int *CheckColorFondo(void);
void SalvaColorFondo(void);
//Para la receta introducida por teclado
int CompruebaReceta(char *receta_a_comprobar);
int CheckMostrarReceta(void);
char *array_receta(char *receta_seccionada[],int num_pasos);
char *ExtraeReceta(char *receta_a_extraer);
void SetMostrarReceta(void);
void ResetMostrarReceta(void);
void MostrarReceta(char *receta_a_leer_en_una_linea);
//Para mostar los nombres de las recetas
void AvanzarNombreRecetas(void);
void RetrocederNombreRecetas(void);
void ResetPunteroNombresRecetas(void);
int CheckPunteroRecetas(void);
void MostrarNombresRecetas(void);
//TIMERS
void Timer0IntHandler(void)
{
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT); //Borra flag
    Cambia=1;
    SysCtlDelay(100);
}
void Timer1IntHandler(void)
{
    TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT); //Borra flag
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

        while(B1_ON) //Esperamos a que el boton deje de ser pulsado
        {

        }
        SysCtlDelay(10*MSEC);
        if(estado == 3) //Estamos en el estado de cambiar la letra
        {
            //Incrementamos el tamano de la letra
            IncTamLetra();
            UARTprintf("Actualizado el valor tam_letra a : %d.\n",tam_letra);
        }
        else if(estado == 6) //Estamos en el estado de visualizar los nombres de las recetas
        {
            AvanzarNombreRecetas();
        }

        GPIOIntClear(GPIO_PORTJ_BASE, GPIO_PIN_0);

    }
    else if(boton_activado == 2)
    {

        while(B2_ON){
        }
        SysCtlDelay(10*MSEC);//Esperamos 10 ms para evitar efectos de bouncing
        if(estado == 3) //Estamos en el estado de cambiar la letra
        {
            DecTamLetra(); //Decrementamos el tamano de la letra
            UARTprintf("Actualizado el valor de tam_letra a : %d.\n",tam_letra);
        }
        else if(estado == 6) //Estamos en el estado de visualizar los nombres de las recetas
        {
            RetrocederNombreRecetas();
        }

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
    char aux[2];

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
        strncpy(aux,s+contador_cadena,1);
        aux[1] = '\0';
        //aux es el caracter que estamos evaluando
        if(Boton(xaux+contador_cadena*w, y, w, h, font, aux))//&aux[contador_cadena]);//*logico (pal 3 de la coord x)
        {

            //UARTprintf("%c", aux);
            flag_pulsado = 1;
            strncpy(&tecla_pulsada,s+contador_cadena,1);
            //guardamos en el caracter tecla_pulsada el valor de la tecla que se ha pulsado
        }
        else
        {
            if(flag_pulsado == 1 && tecla_pulsada == aux[0])
            {
                if(primera_tecla_pulsada == 0)
                {
                    //strcpy(texto_introducido,&tecla_pulsada);
                    texto_introducido[0] = tecla_pulsada;
                    texto_introducido[1] = '\0';
                    primera_tecla_pulsada = 1;
                }
                else
                {
                    //char *caracter_auxiliar;
                    //caracter_auxiliar = malloc(sizeof(char)*1);
                    //strcpy(caracter_auxiliar,&tecla_pulsada);
                    char cadenaTemporal[2];
                    cadenaTemporal[0] = tecla_pulsada;
                    cadenaTemporal[1] = '\0';
                    strcat(texto_introducido,cadenaTemporal);
                    //texto_introducido = strcat(texto_introducido,caracter_auxiliar);
                }
                flag_pulsado = 0;
                UARTprintf("%c\n", tecla_pulsada);
                UARTprintf("%s\n",texto_introducido);
                cont_texto_introducido++;
            }
        }

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

void pinta_cadenas_plantilla(void)
{

    if(CheckMostrarReceta() == 0)
    {
        //No se ha seleccionado aun una receta
        ComColor(0,0,0);
        ComTXT(x_cadena_medio,y_cadena_medio, tam_letra, OPT_CENTER,"Elija una receta en ajustes");
    }
    else if(CheckMostrarReceta() == 1)
    {
        //En el caso de que se tenga que mostrar receta
        ComColor(0,0,0);
        ComTXT(x_cadena_medio,30 , tam_letra, OPT_CENTER,"Pulse avanzar");
        ComTXT(x_cadena_medio,y_cadena_medio-50 , tam_letra, OPT_CENTER,cadena_superior);
        ComTXT(x_cadena_medio,y_cadena_medio , tam_letra, OPT_CENTER,cadena_medio);
        ComTXT(x_cadena_medio,y_cadena_medio+50 , tam_letra, OPT_CENTER,cadena_inferior);
    }
}

void pinta_plantilla_textos(void)
{
    //Funcion dedicada a pintar la plantilla donde se mostraran los textos de las recetas y mensajes
    ComColor(255,255,255);
    ComLineWidth(5);
    int x_esq_sup_izq = (int)HSIZE/5;
    int y_esq_sup_izq = (int)VSIZE/6;
    int x_esq_inf_der = (int)x_esq_sup_izq*4;
    int y_esq_inf_der = (int)y_esq_sup_izq*5;
    ComRect(x_esq_sup_izq,y_esq_sup_izq,x_esq_inf_der,y_esq_inf_der,true);
    //ComRect(20, 20, HSIZE-20, VSIZE-20, true);
}


void pinta_reloj(void) //Funcion que pinta un reloj con la hora actual
{

    time_t tiempo_loc = time(0);
    time(&tiempo_loc);
    struct tm *tlocal;
    tlocal = localtime(&tiempo_loc);

    int16_t x = HSIZE*1/8-10;
    int16_t y = VSIZE/8+10;
    int16_t r = x/2;
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

int init_EEPROM(void)
{

        UARTprintf("Entrando en init_EEPROM.\n");
        //Funcion que se encarga de inicializar la EEPROm
        //Se observa que por defecto la EEPROM esta establecida en el modo de seguridad EEPROM_PROT_RW_LRO_URW
        uint32_t resu_init_EEPROM = EEPROMInit();
        int resu_return;
        if(resu_init_EEPROM == EEPROM_INIT_OK )
        {
            UARTprintf("Se ha iniciado correctamente la EEPROM.\n");
            resu_return = 0;
        }
        else
        {
            UARTprintf("Fallo al iniciar la EEPROM.\n");
            resu_return = -1;
        }

        uint32_t num_bloques_EEPROM = EEPROMBlockCountGet();
        uint32_t tam_bloques_EEPROM = EEPROMSizeGet();

        UARTprintf("El numero de bloques de la EEPROM es : %d.\n",num_bloques_EEPROM);
        UARTprintf("El tamano de los bloques de la EEPROM es : %d.\n",tam_bloques_EEPROM);

        return resu_return;
}

void ReseteaValoresEEPROM(uint32_t direccion,uint32_t valor)
{
    //Funcion para resetear registros
    uint32_t num_bytes = 4;
    EEPROMProgram(&valor, direccion, num_bytes); //Limpiamos el numero de usuarios
    uint32_t resu_status = EEPROMStatusGet();

    while( resu_status != 0) //Esperamos a que se quede libre de la operacion anterior
    {
        resu_status = EEPROMStatusGet();
    }
}

void CambiarEstadoReloj(void)
{
    //Funcion para cambiar el registro del reloj

    //Leemos de su direccion
    uint32_t estado_reloj;
    uint32_t direccion = (uint32_t) DIR_AJUSTES_RELOJ;
    uint32_t num_bytes = 4;

    EEPROMRead(&estado_reloj, direccion, num_bytes);


    uint32_t resu_status = EEPROMStatusGet();

    while( resu_status != 0) //Esperamos a que se quede libre de la operacion anterior
    {
        resu_status = EEPROMStatusGet();
    }
    UARTprintf("El valor establecido en la direccion del registro del reloj es : %d.\n",estado_reloj);

    //Comprobamos que el valor no este entre los no permitidos
    if(estado_reloj != 0 && estado_reloj != 1)
    {
        UARTprintf("El estado del registro del error era invalido. Se resetea su valor.\n");
        ReseteaValoresEEPROM(direccion,(uint32_t)0);
    }
    else //En el caso de que tuviera un valor legal
    {
        if(estado_reloj == 0)
        {
            uint32_t activo = 0x00000001;
            EEPROMProgram(&activo, direccion, num_bytes); //Limpiamos el numero de usuarios
        }
        else
        {
            uint32_t desactivado = 0x00000000;
            EEPROMProgram(&desactivado, direccion, num_bytes); //Limpiamos el numero de usuarios
        }
        uint32_t resu_status = EEPROMStatusGet();

        while( resu_status != 0) //Esperamos a que se quede libre de la operacion anterior
        {
            resu_status = EEPROMStatusGet();
        }
        UARTprintf("Valor del registro del reloj cambiado con exito.\n");

        //Lo leemos para ver que se ha hecho
        EEPROMRead(&estado_reloj, direccion, num_bytes);
        UARTprintf("El valor del registro es : %d.\n",estado_reloj);
    }
}

void PintaReloj(void) //Funcion que pinta un reloj con la hora actual
{

    time_t tiempo_loc = time(0);
    time(&tiempo_loc);
    struct tm *tlocal;
    tlocal = localtime(&tiempo_loc);

    int16_t x = HSIZE*1/8-10;
    int16_t y = VSIZE/8+10;
    int16_t r = x/2;
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

int CheckReloj(void)
{
    //Funcion para pintar reloj
    //Leemos de su direccion
    uint32_t estado_reloj;
    uint32_t direccion = (uint32_t) DIR_AJUSTES_RELOJ;
    uint32_t num_bytes = 4;

    EEPROMRead(&estado_reloj, direccion, num_bytes);
    uint32_t resu_status = EEPROMStatusGet();

    while( resu_status != 0) //Esperamos a que se quede libre de la operacion anterior
    {
        resu_status = EEPROMStatusGet();
    }

    //Comprobamos que el valor no este entre los no permitidos
    if(estado_reloj != 0 && estado_reloj != 1)
    {
        //En el caso de que tenga un valor no permitido, se resetea el reloj
        UARTprintf("El estado del registro del error era invalido. Se resetea su valor.\n");
        ReseteaValoresEEPROM(direccion,(uint32_t)0);
        return 0;
    }
    else
    {
        return (int)estado_reloj;
    }

}

void IncTamLetra(void)
{
    //Funcion para incrementar el tamano de letra
    UARTprintf("Incrementando tam letra.\n");
    uint32_t direccion = (uint32_t) DIR_AJUSTES_TAM_LETRA;
    uint32_t num_bytes = 4;

    uint32_t resultado_check_tam = CheckTamLetra();
    if(resultado_check_tam != 0) //Comprobamos si habia un valor valido
    {
        //En el caso de que todo fuera bien, se nos ha devuelto el tamano
        if(resultado_check_tam < 20)
        {
            UARTprintf("Incrementando de 2.\n");
            //Dado que solo podemos trabajar con 16,18,20,21,22,23...31
            resultado_check_tam = resultado_check_tam + 2;
        }
        else if(resultado_check_tam == TAM_MAXIMO_LETRA)
        {
            //Si el tamano que habia era el maximo permitido, se queda igual
            UARTprintf("Valor maximo ya alcanzado.\n");
            resultado_check_tam = TAM_MAXIMO_LETRA;
        }
        else
        {
            //Si es mayor o igual a 20 y menor al tamano maximo de letra admisible
            UARTprintf("Incrementando de 1.\n");
            resultado_check_tam = resultado_check_tam + 1;
        }

        //Escribimos ese valor sobre la direccion de memoria
        EEPROMProgram(&resultado_check_tam, direccion, num_bytes); //Limpiamos el numero de usuarios
        uint32_t resu_status = EEPROMStatusGet();

        while( resu_status != 0) //Esperamos a que se quede libre de la operacion anterior
        {
            resu_status = EEPROMStatusGet();
        }

        tam_letra = (int16_t)resultado_check_tam; //Actualizamos el valor de la variable global

    }
    else
    {
        //Hubo un fallo en el check del valor
        UARTprintf("Habia un fallo en el tamano de la letra. Se reseta su valor.\n");
    }
}
void DecTamLetra(void)
{
    //Funcion para decrementar el tamano de letra
    UARTprintf("Incrementando tam letra.\n");
    uint32_t direccion = (uint32_t) DIR_AJUSTES_TAM_LETRA;
    uint32_t num_bytes = 4;

    uint32_t resultado_check_tam = CheckTamLetra();
    if(resultado_check_tam != 0) //Comprobamos si habia un valor valido
    {
        //En el caso de que tod fuera bien, se nos ha devuelto el tamano
        if(resultado_check_tam <= 20 && resultado_check_tam != TAM_MINIMO_LETRA)
        {
            //En el caso de que el tamano esta dentro del rango entre 16 y 20
            UARTprintf("Decrementando de 2.\n");
            resultado_check_tam = resultado_check_tam - 2;
        }
        else if(resultado_check_tam == TAM_MINIMO_LETRA)
        {
            //Si ya hemos alcanzado el tamano minimo
            UARTprintf("Tamano minimo ya alcanzado.\n");
            resultado_check_tam = TAM_MINIMO_LETRA;
        }
        else
        {
            //Si no hemos alcanzado el tamano minimo y el valor de tamano era superior a 21
            UARTprintf("Decrementando de 1.\n");
            resultado_check_tam = resultado_check_tam - 1;
        }

        //Escribimos ese valor sobre la direccion de memoria
        EEPROMProgram(&resultado_check_tam, direccion, num_bytes); //Limpiamos el numero de usuarios
        uint32_t resu_status = EEPROMStatusGet();

        while( resu_status != 0) //Esperamos a que se quede libre de la operacion anterior
        {
            resu_status = EEPROMStatusGet();
        }

        tam_letra = (int16_t)resultado_check_tam; //Actualizamos el valor de la variable global

    }
    else
    {
        //Hubo un fallo en el check del valor
        UARTprintf("Habia un fallo en el tamano de la letra. Se reseta su valor.\n");
    }
}
uint32_t CheckTamLetra(void)
{
    //Funcion para chequear el valor del tamano de letra
    //Leemos de su direccion
    uint32_t estado_tam_letra;
    uint32_t direccion = (uint32_t) DIR_AJUSTES_TAM_LETRA;
    uint32_t num_bytes = 4;

    EEPROMRead(&estado_tam_letra, direccion, num_bytes);
    uint32_t resu_status = EEPROMStatusGet();

    while( resu_status != 0) //Esperamos a que se quede libre de la operacion anterior
    {
        resu_status = EEPROMStatusGet();
    }

    if(estado_tam_letra >= 16 && estado_tam_letra <= 31)
    {
        //Si el tamano de letra esta definido dentro del rango posible
        if(estado_tam_letra != 17 && estado_tam_letra != 19)
        {
            //Y no esta entre los tamano "invalidos"
            return estado_tam_letra;
        }
        else
        {
            //Si es uno de los invalidos, se pone un valor por defecto
            uint32_t valor = (uint32_t) TAM_DEFECTO_LETRA;
            ReseteaValoresEEPROM(direccion,valor); //Reseteamos el valor al defecto

            tam_letra = (int16_t)TAM_DEFECTO_LETRA; //Actualizamos el valor de la variable global

            return 0; //Devolvemos 0
        }
    }
    else
    {
        //Si el tamano no estaba dentro del rango permitido
        uint32_t valor = (uint32_t) TAM_DEFECTO_LETRA;
        ReseteaValoresEEPROM(direccion,valor); //Reseteamos el valor al defecto

        tam_letra = (int16_t)TAM_DEFECTO_LETRA; //Actualizamos el valor de la variable global

        return 0; //Devolvemos 0
    }

}

int *CheckColorFondo(void)
{
    //Funcion que se encarga de comprobar los valores de color para el fondo de pantalla almacenados
    //En caso de alguno ilegal, lo resetea al valor por defecto y devuelve un valor nulo para el color correspondiente

    //Leemos de su direccion
    uint32_t estado_colores_fondo;
    uint32_t direccion ;
    uint32_t num_bytes = 4;
    //Almacenamos los valores de las direcciones donde leer

    int *valor_devuelto; //Variable a devolver por la funcion
    valor_devuelto = malloc(sizeof(int)*3);

    //-------------------------------------//
    //Leemos del rojo
    direccion = (uint32_t) DIR_RC;
    EEPROMRead(&estado_colores_fondo, direccion, num_bytes);

    uint32_t resu_status; //Para almacenar el valor del estado EEPROM
    resu_status = EEPROMStatusGet();

    while( resu_status != 0) //Esperamos a que se quede libre de la operacion anterior
    {
        resu_status = EEPROMStatusGet();
    }

    if(estado_colores_fondo <= 255)
    {
        //Si se tiene un valor valido en el rojo
        valor_devuelto[0] = estado_colores_fondo;
        r_c = estado_colores_fondo; //Actualizamos el valor de la variable global
    }
    else
    {
        //El valor almacenado en rojo no era legal
        valor_devuelto[0] = NULL;
        //Escribimos sobre la EEPROM
        ReseteaValoresEEPROM(direccion,(uint32_t)VALOR_B_DEFECTO);
        r_c = (int)VALOR_R_DEFECTO; //Reseteamos el valor de la variable global
    }
    //-------------------------------------//
    //Leemos del verde
    direccion = (uint32_t) DIR_GC;
    EEPROMRead(&estado_colores_fondo, direccion, num_bytes);
    resu_status = EEPROMStatusGet();
    while( resu_status != 0) //Esperamos a que se quede libre de la operacion anterior
    {
        resu_status = EEPROMStatusGet();
    }
    if(estado_colores_fondo <= 255)
    {
        //Si se tiene un valor valido en el verde
        valor_devuelto[1] = estado_colores_fondo;
        g_c = estado_colores_fondo; //Actualizamos el valor de la variable global
    }
    else
    {
        //El valor almacenado en verde no era legal
        valor_devuelto[1] = NULL;
        //Escribimos sobre la EEPROM
        ReseteaValoresEEPROM(direccion,(uint32_t)VALOR_B_DEFECTO);
        g_c = (int)VALOR_G_DEFECTO; //Reseteamos el valor de la variable global
    }
    //-------------------------------------//
    //Leemos del azul
    direccion = (uint32_t) DIR_BC;
    EEPROMRead(&estado_colores_fondo, direccion, num_bytes);
    resu_status = EEPROMStatusGet();
    while( resu_status != 0) //Esperamos a que se quede libre de la operacion anterior
    {
        resu_status = EEPROMStatusGet();
    }
    if(estado_colores_fondo <= 255)
    {
        //Si se tiene un valor valido en el azul
        valor_devuelto[2] = estado_colores_fondo;
        b_c = estado_colores_fondo; //Actualizamos el valor de la variable global
    }
    else
    {
        //El valor almacenado en azul no era legal
        valor_devuelto[2] = NULL;

        //Escribimos sobre la EEPROM
        ReseteaValoresEEPROM(direccion,(uint32_t)VALOR_B_DEFECTO);

        b_c = (int)VALOR_B_DEFECTO; //Reseteamos el valor de la variable global
    }

    return valor_devuelto;
}

void SalvaColorFondo(void)
{
    //Funcion para guardar los colores del fondo.

    //Escribimos sobre sus direcciones
    uint32_t estado_colores_fondo;
    uint32_t direccion ;
    uint32_t num_bytes = 4;



    //-------------------------------------//
    //Escribimos el rojo

    //Actualizamos la variable global
    r_c = r_c_temporal;

    direccion = (uint32_t)DIR_RC;
    estado_colores_fondo = (uint32_t)r_c;
    EEPROMProgram(&estado_colores_fondo, direccion, num_bytes); //Limpiamos el numero de usuarios
    uint32_t resu_status = EEPROMStatusGet();

    while( resu_status != 0) //Esperamos a que se quede libre de la operacion anterior
    {
        resu_status = EEPROMStatusGet();
    }



    //-------------------------------------//
    //Escribimos el verde
    //Escribimos ese valor sobre la direccion de memoria
    //Actualizamos la variable global
    g_c = g_c_temporal;
    direccion = (uint32_t)DIR_GC;
    estado_colores_fondo = (uint32_t)g_c;
    EEPROMProgram(&estado_colores_fondo, direccion, num_bytes); //Limpiamos el numero de usuarios
    resu_status = EEPROMStatusGet();

    while( resu_status != 0) //Esperamos a que se quede libre de la operacion anterior
    {
        resu_status = EEPROMStatusGet();
    }



    //-------------------------------------//
    //Escribimos el azul
    //Escribimos ese valor sobre la direccion de memoria
    //Actualizamos la variable global
    b_c = b_c_temporal;
    direccion = (uint32_t)DIR_BC;
    estado_colores_fondo = (uint32_t)b_c;
    EEPROMProgram(&estado_colores_fondo, direccion, num_bytes); //Limpiamos el numero de usuarios
    resu_status = EEPROMStatusGet();

    while( resu_status != 0) //Esperamos a que se quede libre de la operacion anterior
    {
        resu_status = EEPROMStatusGet();
    }



}

int CompruebaReceta(char *receta_a_comprobar)
{
    //Funcion que se encarga de comprobar si se ha introducida una receta correcta por teclado

    //Obtenemos el numero de recetas disponibles
    int num_recetas = (int)(sizeof(nombres_recetas)/sizeof(nombres_recetas[0]));
    UARTprintf("El numero de recetas disponible es : %d.\n",num_recetas);

    //OJO, se debe excluir del build el recetas.c!!!!!!!!!!

    int i;
    int resultado = 0; //Variable a devolver
    for(i=0;i<num_recetas;i++)
    {
        if(strcmp(receta_a_comprobar, nombres_recetas[i]) == 0)
        {
            //En el caso de que se encuentre la receta
            resultado = 1;
            break; //Salimos del for
        }

    }

    return resultado;
}

int CheckMostrarReceta(void)
{
    //Funcion que se encarga de ver si se debe mostrar una receta o no, con el valor almacenado en la EEPROM
    uint32_t estado_check_receta;
    uint32_t direccion ;
    uint32_t num_bytes = 4;


    int resultado;
    //Leemos de la direccion
    direccion = (uint32_t) DIR_CHECK_RECETA;
    EEPROMRead(&estado_check_receta, direccion, num_bytes);
    if(estado_check_receta == 1)
    {
        //En el caso de que se tenga que mostrar receta
        resultado = 1;
    }
    else
    {
        //En el caso de que no se tenga que mostrar receta
        resultado = 0;
    }

    return resultado;
}

void SetMostrarReceta(void)
{
    //Funcion que se encarga de establecer el set para mostrar una receta
    //Escribimos sobre sus direcciones
    uint32_t estado_mostrar_receta;
    uint32_t direccion ;
    uint32_t num_bytes = 4;

    direccion = (uint32_t)DIR_CHECK_RECETA;
    estado_mostrar_receta = (uint32_t)1;
    EEPROMProgram(&estado_mostrar_receta, direccion, num_bytes); //Limpiamos el numero de usuarios
    uint32_t resu_status = EEPROMStatusGet();

    while( resu_status != 0) //Esperamos a que se quede libre de la operacion anterior
    {
        resu_status = EEPROMStatusGet();
    }
}

void ResetMostrarReceta(void)
{
    //Funcion que se encarga de establecer el reset para mostrar una receta
    uint32_t estado_mostrar_receta;
    uint32_t direccion ;
    uint32_t num_bytes = 4;

    direccion = (uint32_t)DIR_CHECK_RECETA;
    estado_mostrar_receta = (uint32_t)0;
    EEPROMProgram(&estado_mostrar_receta, direccion, num_bytes); //Limpiamos el numero de usuarios
    uint32_t resu_status = EEPROMStatusGet();

    while( resu_status != 0) //Esperamos a que se quede libre de la operacion anterior
    {
        resu_status = EEPROMStatusGet();
    }

    //Reseteamos los valores de las cadenas "globales" para mostrar la receta
    memset(cadena_inferior, 0, longitud);
    memset(cadena_medio, 0, longitud);
    memset(cadena_superior, 0, longitud);
    flag_termina = 0;
    memset(lineas[0], 0, longitud);
    memset(lineas[1], 0, longitud);
    memset(lineas[2], 0, longitud);
    caracter_empezar_a_leer = 0; //Indica por donde empezar a leer, para la primera linea
    puntero_caracter = 0;
    puntero_linea_0 = 0;
}

char *ExtraeReceta(char *receta_a_extraer)
{
    //Funcion que busca la receta entre las variables de las recetas, y la devuelve como una sola cadena

    char *receta_en_una_cadena; //Donde almacenamos la cadena en un solo string
    int num_pasos; //Numero de pasos de la receta seleccionada

    if(strcmp("ARROZ", receta_a_extraer) == 0)
    {
        //Se trabaja con arroz
        num_pasos = sizeof(arroz)/sizeof(arroz[0]);
        receta_en_una_cadena = array_receta(arroz, num_pasos); //DA UN WARNING QUE NO TIENE EFECTO
    }
    else if(strcmp("POLLO", receta_a_extraer) == 0)
    {
        //Se trabaja con pollo
        num_pasos = sizeof(pollo)/sizeof(pollo[0]);
        receta_en_una_cadena = array_receta(pollo, num_pasos); //DA UN WARNING QUE NO TIENE EFECTO
    }
    else if(strcmp("TORTILLA", receta_a_extraer) == 0)
    {
        //Se trabaja con tortilla
        num_pasos = sizeof(tortilla)/sizeof(tortilla[0]);
        receta_en_una_cadena = array_receta(tortilla, num_pasos); //DA UN WARNING QUE NO TIENE EFECTO
    }
    else
    {
        UARTprintf("La cadena introducida no esta disponible.\n");
    }
    UARTprintf("La cadena a devolver en una sola linea : \n%s",receta_en_una_cadena);
    return receta_en_una_cadena;
}


char *array_receta(char *receta_seccionada[],int num_pasos)
{
    //Funcion para poner una receta en un solo array
    char *cadena_a_devolver; //Lo que vamos a devolver
    int num_caracteres_receta = 0; //Numero total de caracteres de la receta
    int i;
    for(i=0;i<num_pasos;i++)
    {
        //Recorremos todos los pasos de la receta
        num_caracteres_receta = num_caracteres_receta + strlen(receta_seccionada[i]);
        UARTprintf("Numero cadena %d es : %s.\n",i,receta_seccionada[i]);
    }
    UARTprintf("El numero total de caracteres en la receta es : %d.\n",num_caracteres_receta);

    cadena_a_devolver = malloc((sizeof(char)*num_caracteres_receta)+num_pasos); //Reservamos el espacio necesario
    int num_bytes_reservados = strlen(cadena_a_devolver);
    UARTprintf("Se han reservado : %d bytes.\n",(sizeof(char)*num_caracteres_receta)+num_pasos);
    //Vamos copiando en cadena_a_devolver los valores de la receta
    for(i=0;i<num_pasos;i++)
    {
        if(i==0)
        {
            strcpy(cadena_a_devolver,receta_seccionada[0]);
        }
        else
        {
            strcat(cadena_a_devolver,"\n"); //Inlcuimos los saltos de lineas
            strcat(cadena_a_devolver,receta_seccionada[i]);
        }
    }
    UARTprintf("array_receta : %s.\n",cadena_a_devolver);
    return cadena_a_devolver; //Devolvemos la cadena ya construida
}


void MostrarReceta(char *receta_a_leer_en_una_linea)
{
    //Funcion para ir mostrando la receta
    int i;
    UARTprintf("MostrarReceta recibe : %s.\n",receta_a_leer_en_una_linea);
    puntero_caracter = puntero_linea_0;
    for(i=0;i<3;i++)
    {
        memset(lineas[i], 0, longitud); //Limpiamos la cadena que vamos a escribir
        int pos_fin_linea = puntero_caracter; //Para buscar posicion de un fin de linea
        int contador = 0; //Para contar el numero de caracteres que llevamos
        //En primer lugar, buscamos el espacio

        while(receta_a_leer_en_una_linea[pos_fin_linea] != '\n' && receta_a_leer_en_una_linea[pos_fin_linea] != '\0' && contador < longitud-1)
        {
            //printf("%c.\n",receta_en_una_linea[pos_fin_linea]);
            pos_fin_linea = pos_fin_linea + 1;
            contador = contador + 1;
        }
        //Una vez salido del while
        if(contador == longitud-1) //Se ha alcanzado el limite de longitud
        {
            UARTprintf("Saliendo por exceso de longitud.\n");
            memcpy( lineas[i], &receta_a_leer_en_una_linea[puntero_caracter], contador-1);
            lineas[i][contador] = '\0';
            puntero_caracter = pos_fin_linea-1;
        }
        else if(receta_a_leer_en_una_linea[pos_fin_linea] == '\n') //Si lo que se ha encontrado ha sido un salto de linea
        {
            UARTprintf("Saliendo por fin de linea.\n");
            memcpy(lineas[i],&receta_a_leer_en_una_linea[puntero_caracter],contador);
            puntero_caracter = pos_fin_linea+1;
        }
        else if(receta_a_leer_en_una_linea[pos_fin_linea] == '\0') //Si lo que se ha encontrado ha sido el final
        {
            UARTprintf("Saliendo por fin de fichero.\n");
            memcpy(lineas[i],&receta_a_leer_en_una_linea[puntero_caracter],contador);
            puntero_caracter = pos_fin_linea;
            if(i==0)
            {
                flag_termina = 1; //Marcamos el final
                break;
            }
            UARTprintf("Encontado final.\n");
        }

        if(i==0) //Guardamos el valor desde donde leer en la proxima iteraccion
        {
            puntero_linea_0 = puntero_caracter;
            strcpy(cadena_superior, lineas[i]);
        }
        else if(i==1)
        {
            strcpy(cadena_medio, lineas[i]);
        }
        else if(i==2)
        {
            strcpy(cadena_inferior, lineas[i]);
        }

        UARTprintf("El valor de la cadena %d es : |%s|.\n",i,lineas[i]);

    }
    UARTprintf("Pulse una tecla...\n");

}

void ResetPunteroNombresRecetas(void)
{
    //Funcion para resetear la direccion de memoria del puntero para mostrar los nombres de las recetas
    uint32_t estado_puntero_recetas;
    uint32_t direccion ;


    direccion = (uint32_t)DIR_PUNTERO_RECETA;
    estado_puntero_recetas = (uint32_t)0;
    ReseteaValoresEEPROM(direccion,estado_puntero_recetas); //Reseteamos el valor del puntero
    puntero_a_nombre_receta = 0; //Actualizamos el valor de la variable global
}

void AvanzarNombreRecetas(void)
{
    //Funcion para avanzar el puntero de los nombres de las recetas
    uint32_t direccion;
    uint32_t num_bytes = 4;
    direccion = (uint32_t)DIR_PUNTERO_RECETA; //Guardamos la direccion donde mirar
    int valor_puntero_actual = CheckPunteroRecetas(); //Extraemos el valor del punteor

    //Obtenemos el numero de recetas
    int num_recetas = (int)(sizeof(nombres_recetas)/sizeof(nombres_recetas[0]));

    if(valor_puntero_actual < num_recetas-1)
    {
        //Si podemos incrementar
        valor_puntero_actual = valor_puntero_actual + 1; //Incrementamos en 1 en el caso contrario
    }

    uint32_t valor_puntero_a_escribir = (uint32_t)valor_puntero_actual;
    //Escribimos sobre la direccion de memoria
    EEPROMProgram(&valor_puntero_a_escribir, direccion, num_bytes); //Limpiamos el numero de usuarios
    uint32_t resu_status = EEPROMStatusGet();

    while( resu_status != 0) //Esperamos a que se quede libre de la operacion anterior
    {
        resu_status = EEPROMStatusGet();
    }

    puntero_a_nombre_receta = valor_puntero_actual; //Actualizamos la variable global
    UARTprintf("El valor del puntero a receta actual, tras incrementar, es : %d.\n",valor_puntero_actual);

}

void RetrocederNombreRecetas(void)
{
    //Funcion para retroceder el puntero de los nombres de las recetas
    uint32_t direccion;
    uint32_t num_bytes = 4;
    direccion = (uint32_t)DIR_PUNTERO_RECETA; //Guardamos la direccion donde mirar
    int valor_puntero_actual = CheckPunteroRecetas(); //Extraemos el valor del punteor

    if(valor_puntero_actual == 0)
    {
        //Si ya estamos en el minimo
        valor_puntero_actual = 0; //Nos quedamos en el minimo
    }
    else
    {
        valor_puntero_actual = valor_puntero_actual - 1; //Incrementamos en 1 en el caso contrario
    }

    //Escribimos sobre la direccion de memoria
    uint32_t valor_a_escribir = (uint32_t)valor_puntero_actual;
    EEPROMProgram(&valor_a_escribir, direccion, num_bytes); //Limpiamos el numero de usuarios
    uint32_t resu_status = EEPROMStatusGet();

    while( resu_status != 0) //Esperamos a que se quede libre de la operacion anterior
    {
        resu_status = EEPROMStatusGet();
    }

    puntero_a_nombre_receta = valor_puntero_actual; //Actualizamos el valor de la variable global
    UARTprintf("El valor del puntero a receta actual, tras decrementar, es : %d.\n",valor_puntero_actual);

}

int CheckPunteroRecetas(void)
{
    //Funcion para chequear el valor del puntero de los nombres de las recetas
    int resultado; //Valor a devolver

    uint32_t direccion;
    uint32_t valor;
    uint32_t num_bytes = 4;
    direccion = (uint32_t)DIR_PUNTERO_RECETA;
    EEPROMRead(&valor, direccion, num_bytes); //Extraemos el valor

    uint32_t resu_status; //Para almacenar el valor del estado EEPROM
    resu_status = EEPROMStatusGet();
        while( resu_status != 0) //Esperamos a que se quede libre de la operacion anterior
    {
        resu_status = EEPROMStatusGet();
    }

    //Sacamos el maximo de recetas disponibles
    int num_recetas = (int)(sizeof(nombres_recetas)/sizeof(nombres_recetas[0]));

    if((int)valor < 0)
    {
        //En el caso de un valor negativo, se indicaria como un error. Por tanto, se resetea el valor de ese "registro"
        ResetPunteroNombresRecetas();
        resultado = 0;
    }
    else if((int)valor > num_recetas-1)
    {
        //En el caso de un valor superior al numero de recetas posibles
        ResetPunteroNombresRecetas();
        resultado = 0;
    }
    else
    {
        //En el caso de que todo haya ido bien
        resultado = (int)valor;
    }

    puntero_a_nombre_receta = resultado; //Escribimos sobre la variable global
    UARTprintf("El valor del puntero a receta actual es : %d.\n",resultado);
    return resultado;
}

void MostrarNombresRecetas(void)
{
    //Funcion para mostrar los nombres de las recetas
    memset(nombre_receta_a_mostrar,0,longitud); //Limpiamos la cadena a mostrar
    strcpy(nombre_receta_a_mostrar,nombres_recetas[puntero_a_nombre_receta]); //Copiamos el nombre
    ComTXT(HSIZE/2, (VSIZE/2)+40,tam_letra,OPT_CENTER,nombre_receta_a_mostrar); //Lo mostramos en la pantalla

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

    //Iniciliazacion de la EEPROM
    SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0); //Habilitacion del EEPROM
    if(init_EEPROM() != 0) //Funcion para inicializar la EEPROM
    {
        //En el caso de que no se pueda trabajar con la EEPROM
        UARTprintf("ERROR FATAL EN LA EEPROM.\n");
        return 0;
    }

    //Configuracion del BoosterPack de los sensores
    Conf_Boosterpack(BOOSTERPACK_SENSORES, RELOJ); //Funcion definida en la libreria HAL_I2C.c

    //Configuracion del tamano de la letra para la pantalla
    tam_letra = (int16_t)CheckTamLetra();
    if (tam_letra == 0)
    {
        //tam_letra = TAM_DEFECTO_LETRA; //Si habia un valor definido ilegal, ponemos el valor por defecto --> NO HACE FALTA, PUES EN CASO DE ERROR LA FUNCION "CheckTamLetra()" RESETEA EL VALOR DE LA VARIABLE GLOBAL
        UARTprintf("El valor que habia para la letra era ilegal.\n");
        UARTprintf("Ahora su valor es : %d.\n",tam_letra);
    }
    else
    {
        UARTprintf("El tamano de la letra era : %d.\n",tam_letra);
    }

    //Configuracion del "registro" para mostrar receta
    ResetMostrarReceta(); //Lo reseteamos a 0

    //Configuracion del "registro" del puntero para mostrar los nombres de las recetas
    ResetPunteroNombresRecetas();

    //Configuracion del color de fondo de la pantalla
    int *check_colores_fondo = CheckColorFondo();
    for (i=0;i<3;i++)
    {
        if(check_colores_fondo[i] == NULL)
        {
            if(i==0)
            {
                UARTprintf("El color rojo tenia un fallo en su valor. Se ha reseteado.\n");
            }
            else if(i==1)
            {
                UARTprintf("El color verde tenia un fallo en su valor. Se ha reseteado.\n");
            }
            else if(i==2)
            {
                UARTprintf("El color azul tenia un fallo en su valor. Se ha reseteado.\n");
            }
        }
        else
        {
            if(i==0)
            {
                UARTprintf("El color rojo tenia un valor de : %d.\n",check_colores_fondo[i]);
            }
            else if(i==1)
            {
                UARTprintf("El color verde tenia un valor de : %d.\n",check_colores_fondo[i]);
            }
            else if(i==2)
            {
                UARTprintf("El color azul tenia un valor de : %d.\n",check_colores_fondo[i]);
            }
        }
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
    ComTXT(HSIZE/2,VSIZE/5, 20, OPT_CENTERX,"Asistente de Recetas");
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

    memset(receta_introducida,0,strlen(receta_introducida)); //Se resetea el valor de la receta
    //Comprobacion de que el archivo que queremos usar para escribir los los usuarios y contrasenas existe o no

    while(1)
    {
        if(Cambia==1){
            Cambia=0;

            ticks=0;

            Lee_pantalla();
            Nueva_pantalla(16,16,16);
            ComColor(r_c, g_c, b_c); //Valores RGB del celeste del fondo de la pantalla
            ComLineWidth(5);
            ComRect(1, 1, HSIZE-1, VSIZE-1, true);
            switch(estado)
            {
                case 0:
                    //Pintamos la plantilla en blanco donde pondremos los textos
                    pinta_plantilla_textos();
                    //Pintamos las cadenas de texto de ese recuadro
                    pinta_cadenas_plantilla();

                    ComFgcolor(139,69,19);
                    ComColor(255, 255, 255);

                    //Pintamos el boton de ajustes
                    if(Boton((HSIZE)-50,10, 40, 30, 16, "Ajustes"))
                    {
                        estado = 1; //Pasamos al estado de los ajustes
                    }


                    //Comprobamos los valores de los ajustes
                    //El caso de pintar el reloj
                    if(CheckReloj() == 1)
                    {
                        PintaReloj();
                    }

                    //Pintamos boton par avanzar en la lectura de la receta
                    if(Boton((HSIZE)-50,(VSIZE)-50, 40, 30, 16, "Avanza"))
                    {
                        avanza_e0_pulsado = 1;
                    }
                    else
                    {
                        if(avanza_e0_pulsado == 1) //Si se habia pulsado
                        {
                            if(flag_termina == 0)
                            {
                                UARTprintf("Se ha pulsado avanza. La receta : \n%s\n",receta_en_una_linea);
                                MostrarReceta(receta_en_una_linea); //Vamos mostrando la receta
                            }
                            else
                            {
                                ResetMostrarReceta(); //Reseteamos el mostrar la receta
                            }
                        }
                        avanza_e0_pulsado = 0;
                    }

                    break;
                case 1:
                    //Estado de los ajustes

//---------------------------------------------------------------------------------------------//
                    //Pintamos boton para mostrar reloj o no
                    ComColor(0,0,0);
                    ComFgcolor(255, 255, 255);
                    if(Boton(((HSIZE)/2)-50,(VSIZE/8)-10, 60, 20, 16, "Pon/Quita reloj"))
                    {
                            mostrar_reloj_pulsado = 1;
                    }
                    else
                    {
                        if(mostrar_reloj_pulsado == 1) //Se ha pulsado el boton del reloj anteriormente, y lo acabamos de soltar
                        {
                            CambiarEstadoReloj(); //Cambiamos el estado del registro del reloj
                            mostrar_reloj_pulsado = 0; //Liberamos la flag
                        }
                    }
                    if(CheckReloj() == 1)
                    {
                        //Si se ha activado el reloj, mostramos un pequeño recuadro verde para hacerlo mas visual
                        ComColor(100, 255, 0); //Valores RGB del verde
                        ComLineWidth(5);
                        ComRect((HSIZE/2)+50, (VSIZE/8)-10, (HSIZE/2)+70, (VSIZE/8)+10, true);
                    }
                    else
                    {
                        //Si no se ha activado el reloj, mostramos un pequeño recuadro rojo para hacerlo mas visual
                        ComColor(255, 100, 0); //Valores RGB del rojo
                        ComLineWidth(5);
                        ComRect((HSIZE/2)+50, (VSIZE/8)-10, (HSIZE/2)+70, (VSIZE/8)+10, true);
                    }

//---------------------------------------------------------------------------------------------//
                    ComColor(0,0,0);
                    ComFgcolor(255, 255, 255);
                    if(Boton(((HSIZE)/2)-50,(VSIZE/4)-10, 60, 20, 16, "Elegir receta"))
                    {
                        if(elegir_receta_pulsado == 0)
                        {
                            elegir_receta_pulsado = 1;
                        }
                        UARTprintf("Pasando a elegir receta.\n");
                    }
                    else
                    {
                        if(elegir_receta_pulsado == 1) //Se suelta el boton
                        {
                            memset(texto_introducido,0,strlen(texto_introducido)); //Se resetea el valor del texto del teclado
                            memset(receta_introducida,0,strlen(receta_introducida)); //Se resetea el valor de la receta
                            estado = 2; //Se pasa al estado 2
                        }
                        elegir_receta_pulsado = 0;
                    }

//---------------------------------------------------------------------------------------------//
                    //Pintamos el boton para cambiar tamano de la letra
                    ComColor(0,0,0);
                    ComFgcolor(255, 255, 255);
                    if(Boton(((HSIZE)/2)-50,(VSIZE/4)+10, 60, 20, 16, "Cambiar.Tam.Letra"))
                    {
                        //Si se pulsa el boton
                        cambiar_tam_letra_pulsado = 1;
                    }
                    else
                    {
                        //Cuando no esta pulsado
                        if(cambiar_tam_letra_pulsado == 1)
                        {
                            //Si se ha soltado tras haberlo pulsado
                            estado = 3; //Cambiamos de estado
                        }
                        cambiar_tam_letra_pulsado = 0;
                    }



//---------------------------------------------------------------------------------------------//
                    //Pintamos el boton para cambiar el color de fondo de pantalla
                    ComColor(0,0,0);
                    ComFgcolor(255, 255, 255);
                    if(Boton(((HSIZE)/2)-50,(VSIZE/4)+30, 60, 20, 16, "Cambiar Color Fondo"))
                    {
                        cambiar_color_fondo_pulsado = 1;
                    }
                    else
                    {
                        if(cambiar_color_fondo_pulsado == 1)
                        {
                            //Si se habia pulsado el boton de cambiar de color
                            //Asignamos a los valores temporales los valores guardados
                            r_c_temporal = r_c;
                            g_c_temporal = g_c;
                            b_c_temporal = b_c;
                            estado = 4;
                        }
                        cambiar_color_fondo_pulsado = 0;
                    }
//---------------------------------------------------------------------------------------------//
                    //Pintamos el boton para mostrar las recetas disponibles
                    ComColor(0,0,0);
                    ComFgcolor(255, 255, 255);
                    if(Boton(((HSIZE)/2)-50,(VSIZE/4)+50, 60, 20, 16, "Mostrar recetas disponibles"))
                    {
                        mostrar_recetas_pulsado = 1;
                    }
                    else
                    {
                        if(mostrar_recetas_pulsado == 1)
                        {
                            //Se habia pulsado el boton de mostrar recetas
                            estado = 6; //Pasamos al estado de mostrar recetas
                        }
                        mostrar_recetas_pulsado = 0;
                    }

//---------------------------------------------------------------------------------------------//
                    //Pintamos boton para volver a la pantalla principal
                    ComColor(255,255,255);
                    ComFgcolor(255, 0, 0);
                    if(Boton((HSIZE/10)-20, VSIZE/10, 20, 20, 16, "Back"))
                    {
                        back_e1_pulsado = 1;
                    }
                    else
                    {
                        if(back_e1_pulsado == 1)
                        {
                            //Se habia pulsado el back
                            estado = 0;
                        }
                        back_e1_pulsado = 0;
                    }


                    break;
                case 2:
                    //Caso de la seleccion de receta
                    ComColor(255,255,255);
                    ComTXT((HSIZE/4)+10, VSIZE/8,tam_letra,OPT_CENTER,"Receta :");
                    ComColor(255,255,255);
                    ComRect(HSIZE/2,(VSIZE/8)-10, HSIZE, (VSIZE/8)+10, true); //Cuadro de disply de la contrasena escrita
                    teclado(35); //Funcion del teclado
                    strcpy(receta_introducida,texto_introducido); //Copiamos el valor del teclado en receta_introducida
                    ComColor(0,0,0);
                    ComTXT(HSIZE*3/4, VSIZE/8, tam_letra, OPT_CENTER,receta_introducida);
//---------------------------------------------------------------------------------------------//
                    //Pintamos boton para volver
                    ComColor(255,255,255);
                    ComFgcolor(255, 0, 0);
                    if(Boton((HSIZE/10)-20, VSIZE/10, 20, 20, 16, "Back"))
                    {
                        back_e2_pulsado = 1;
                    }
                    else
                    {
                        if(back_e2_pulsado == 1)
                        {
                            //Se habia pulsado el back
                            estado = 1;
                        }
                        back_e2_pulsado = 0;
                    }
//---------------------------------------------------------------------------------------------//
                    //Pintamos boton para guardar
                    ComColor(255,255,255);
                    ComFgcolor(0, 255, 0);
                    if(Boton((HSIZE/10)-20, (VSIZE/10)+30, 20, 20, 16, "Save"))
                    {
                        save_e2_pulsado = 1;
                    }
                    else
                    {
                        if(save_e2_pulsado == 1)
                        {
                            //Se habia pulsado el save
                            if(CompruebaReceta(receta_introducida) == 1)
                            {
                                //En el caso de que la receta introducida exista
                                UARTprintf("La receta introducida SI existe.\n");
                                free(receta_en_una_linea); //Limpiamos la cadena
                                receta_en_una_linea = ExtraeReceta(receta_introducida); //Almacenamos la receta en una sola linea
                                SetMostrarReceta(); //Activamos el registro para mostrar receta
                                estado = 0; //Volvemos a la pantalla principal
                                UARTprintf("Volviendo a la pantalla principal.\n");
                            }
                            else
                            {
                                //En el caso de que la receta introducida no exista
                                UARTprintf("La receta introducida NO existe.\n");
                                estado = 1; //Volvemos al estado de ajustes
                            }

                        }
                        save_e2_pulsado = 0;
                    }

                    break;
                case 3:
                    //En el caso de que queramos cambiar la letra
                    //Mostramos lo texto principales
                    ComColor(255,255,255);
                    ComTXT(HSIZE/2, (VSIZE/2)-20,tam_letra,OPT_CENTER,"Pulse B1 para inc tam letra.");
                    ComTXT(HSIZE/2, (VSIZE/2)+20,tam_letra,OPT_CENTER,"Pulse B2 para dec tam letra.");

                    //Pintamos boton para volver
                    ComColor(255,255,255);
                    ComFgcolor(255, 0, 0);
                    if(Boton((HSIZE/10)-20, VSIZE/10, 20, 20, 16, "Back"))
                    {
                        back_e3_pulsado = 1;
                    }
                    else
                    {
                        if(back_e3_pulsado == 1)
                        {
                            //Se habia pulsado el back
                            estado = 1;
                        }
                        back_e3_pulsado = 0;
                    }
                    break;

                case 4:
                    //Estado para cambiar el color de fondo de pantalla

                    //Pintamos barras para cambiar el valor del fondo de pantalla
                    Lee_pantalla();
                    ComColor(255,255,255);
                    ComTXT(HSIZE/2,(VSIZE/2)+10,16,OPT_CENTER,"Valores RGB del fondo");

                    /* Pintar Scrollbars y leer valores rgb*/
                    //R
                    ComFgcolor(200,10,10);
                    ComBgcolor(150,5,5);
                    if(POSX>35 && POSX<HSIZE-35 && POSY>130 && POSY <155 && POSX!=0x8000) r_c_temporal=((POSX-35)*255)/(HSIZE-70);
                    ComScrollbar(35,(VSIZE/2)+30,HSIZE-70,20,0,r_c_temporal,0,255);

                    //G
                    ComFgcolor(10,200,10);
                    ComBgcolor(5,150,5);
                    if(POSX>35 && POSX<HSIZE-35 && POSY>170 && POSY <195 && POSX!=0x8000) g_c_temporal=((POSX-35)*255)/(HSIZE-70);
                    ComScrollbar(35,(VSIZE/2)+60,HSIZE-70,20,0,g_c_temporal,0,255);
                    //B
                    ComFgcolor(10,10,200);
                    ComBgcolor(5,5,150);
                    if(POSX>35 && POSX<HSIZE-35 && POSY>210 && POSY <235 && POSX!=0x8000) b_c_temporal=((POSX-35)*255)/(HSIZE-70);
                    ComScrollbar(35,(VSIZE/2)+90,HSIZE-70,20,0,b_c_temporal,0,255);

//---------------------------------------------------------------------------------------------//
                    //Mostramos una muestra del color seleccionado
                    ComTXT(HSIZE/2,(VSIZE/4),16,OPT_CENTER,"Muestra color seleccionado");
                    ComColor(r_c_temporal, g_c_temporal, b_c_temporal); //Valores RGB selecionados
                    ComLineWidth(5);
                    ComRect(HSIZE/4,(VSIZE/4)+10, (HSIZE/4)*3, (VSIZE/2)-10, true);

                    //Pintamos un pequeno recuadro alrededor
                    ComColor(255,255,255); //Valores RGB del celeste del fondo de la pantalla
                    ComLineWidth(2);
                    ComRect((HSIZE/4)-4,(VSIZE/4)+8, ((HSIZE/4)*3)+2, (VSIZE/2)-8, false);
//---------------------------------------------------------------------------------------------//

                    //Pintamos boton para volver
                    ComColor(255,255,255);
                    ComFgcolor(255, 0, 0);
                    if(Boton((HSIZE/10)-20, VSIZE/10, 20, 20, 16, "Back"))
                    {
                        back_e4_pulsado = 1;
                    }
                    else
                    {
                        if(back_e4_pulsado == 1)
                        {
                            //Se habia pulsado el save
                            estado = 1;
                        }
                        back_e4_pulsado = 0;
                    }

//---------------------------------------------------------------------------------------------//


                    //Pintamos boton para salvar los valores
                    ComColor(255,255,255);
                    ComFgcolor(0, 255, 0);

                    if(Boton((HSIZE/10)-20, (VSIZE/10)+30, 20, 20, 16, "Save"))
                    {
                        save_e4_pulsado = 1;
                    }
                    else
                    {
                        if(save_e4_pulsado == 1)
                        {
                            //Se habia pulsado el save
                            SalvaColorFondo(); //Guardamos los colores en la EEPROM, para que se mantega su valor de manera no volatil
                            estado = 1;
                        }
                        save_e4_pulsado = 0;
                    }

                    break;
                case 6:
                    //El caso en el que se muestran las recetas disponibles
                    //Mostramos lo texto principales
                    ComColor(255,255,255);
                    ComTXT(HSIZE/2, (VSIZE/4)-10,tam_letra,OPT_CENTER,"Pulse B1 para pasar de nombre.");
                    ComTXT(HSIZE/2, (VSIZE/4)+30,tam_letra,OPT_CENTER,"Pulse B2 para volver en nombre.");
                    MostrarNombresRecetas(); //Funcion para mostrar la receta

                    //Pintamos boton para volver
                    ComColor(255,255,255);
                    ComFgcolor(255, 0, 0);
                    if(Boton((HSIZE/10)-20, VSIZE/10, 20, 20, 16, "Back"))
                    {
                        back_e6_pulsado = 1;
                    }
                    else
                    {
                        if(back_e6_pulsado == 1)
                        {
                            //Se habia pulsado el back
                            estado = 1; //Volvemos al estado de ajustes
                        }
                        back_e6_pulsado = 0;
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
