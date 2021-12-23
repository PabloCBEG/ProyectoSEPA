// #################################################################################################
// # << NEORV32 - Blinking LED Demo Program >>                                                     #
// # ********************************************************************************************* #
// # BSD 3-Clause License                                                                          #
// #                                                                                               #
// # Copyright (c) 2021, Stephan Nolting. All rights reserved.                                     #
// #                                                                                               #
// # Redistribution and use in source and binary forms, with or without modification, are          #
// # permitted provided that the following conditions are met:                                     #
// #                                                                                               #
// # 1. Redistributions of source code must retain the above copyright notice, this list of        #
// #    conditions and the following disclaimer.                                                   #
// #                                                                                               #
// # 2. Redistributions in binary form must reproduce the above copyright notice, this list of     #
// #    conditions and the following disclaimer in the documentation and/or other materials        #
// #    provided with the distribution.                                                            #
// #                                                                                               #
// # 3. Neither the name of the copyright holder nor the names of its contributors may be used to  #
// #    endorse or promote products derived from this software without specific prior written      #
// #    permission.                                                                                #
// #                                                                                               #
// # THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS   #
// # OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF               #
// # MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE    #
// # COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,     #
// # EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE #
// # GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED    #
// # AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING     #
// # NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED  #
// # OF THE POSSIBILITY OF SUCH DAMAGE.                                                            #
// # ********************************************************************************************* #
// # The NEORV32 Processor - https://github.com/stnolting/neorv32              (c) Stephan Nolting #
// #################################################################################################


/**********************************************************************//**
 * @file blink_led/main.c
 * @author Stephan Nolting
 * @brief Simple blinking LED demo program using the lowest 8 bits of the GPIO.output port.
 **************************************************************************/

#include <neorv32.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
/**********************************************************************//**
 * @name User configuration
 **************************************************************************/
/**@{*/
/** UART BAUD rate */
#define BAUD_RATE 19200
/** Use the custom ASM version for blinking the LEDs defined (= uncommented) */
//#define USE_ASM_VERSION
/**@}*/
#define NUM_FILAS 4 //Numero de filas del teclado
#define NUM_COLUMNAS 4 //Numero de columnas del teclado
#define NUM_STRINGS  16
#define MAX_LENGTH  100



/**********************************************************************//**
 * ASM function to blink LEDs
 **************************************************************************/
extern void blink_led_asm(uint32_t gpio_out_addr);

/**********************************************************************//**
 * C function to blink LEDs
 **************************************************************************/
void blink_led_c(void);


/**********************************************************************//**
 * Main function; shows an incrementing 8-bit counter on GPIO.output(7:0).
 *
 * @note This program requires the GPIO controller to be synthesized (the UART is optional).
 *
 * @return 0 if execution was successful
 **************************************************************************/
uint32_t funcion_teclado(void);


int main() {

  // init UART (primary UART = UART0; if no id number is specified the primary UART is used) at default baud rate, no parity bits, ho hw flow control
  neorv32_uart0_setup(BAUD_RATE, PARITY_NONE, FLOW_CONTROL_NONE);

  // check if GPIO unit is implemented at all
  if (neorv32_gpio_available() == 0) {
    neorv32_uart0_print("Error! No GPIO unit synthesized!\n");
    return 1; // nope, no GPIO unit synthesized
  }

  // capture all exceptions and give debug info via UART
  // this is not required, but keeps us safe
  neorv32_rte_setup();

  // say hello
  neorv32_uart0_print("Lectura de registros wishbone.\n");

int i,j;
uint32_t cont = 1;
uint32_t direccion_lectura; //Direccion base del periferico wishbone al que queremos acceder
uint32_t valor_leido;

int columna;
int fila;
neorv32_uart0_printf("El valor de cont en la primera vuelta es : %u\n",cont);
// use ASM version of LED blinking (file: blink_led_in_asm.S)
#ifdef USE_ASM_VERSION

  blink_led_asm((uint32_t)(&GPIO_OUTPUT));

// use C version of LED blinking
#else
while(1)
{
    direccion_lectura = (uint32_t)0x90000000;
    neorv32_uart0_printf("Se va a leer la contrasena.\n");
    valor_leido = neorv32_cpu_load_unsigned_word(direccion_lectura);
    neorv32_uart0_printf("El valor de la contrasena es : %x.\n",valor_leido);
    neorv32_uart0_printf("Por favor, introduzca el valor de la contrasena por teclado.\n");
    
    uint32_t contrasena_introducida = funcion_teclado();
    neorv32_uart0_printf("Se ha introducido la siguiente contrasena : %x.\n",contrasena_introducida);

  if(contrasena_introducida == valor_leido)
  {
    neorv32_uart0_printf("Enhoranbuena. Correcto.\n");
  }
  else
  {
    neorv32_uart0_printf("Pruebe otra vez.\n");
  }


}


#endif
  return 0;
}


/**********************************************************************//**
 * C-version of blinky LED counter
 **************************************************************************/
 /*
void blink_led_c(void) {

  neorv32_gpio_port_set(0); // clear gpio output

  int cnt = 0;

  while (1) {
    neorv32_gpio_port_set(cnt++ & 0xFF); // increment counter and mask for lowest 8 bit
    neorv32_cpu_delay_ms(200); // wait 200ms using busy wait
  }
}

*/

uint32_t funcion_teclado(void)
{
  uint32_t valores_teclado; //Variable para la lectura de los pines del gpio
  uint32_t cont = 15;
  uint32_t mascaras[4] = {0x000f,0x00f0,0x0f00,0xf000}; //Mascaras a aplicar
  uint32_t valores_fila_lectura[4];
  uint32_t valores_fila_lectura_anterior[4];

  char arr1[NUM_STRINGS][MAX_LENGTH] = {"NADA","1","4","1 y 4","7","1 y 7","4 y 7","1 y 4 y 7","0","1 y 0","4 y 0","1 y 4 y 0","7 y 0","1 y 7 y 0","4 y 7 y 0","1 y 4 y 7 y 0"};
  char arr2[NUM_STRINGS][MAX_LENGTH] = {"NADA","2","5","2 y 5","8","2 y 8","5 y 8","2 y 5 y 8","F","2 y F","5 y F","2 y 5 y F","8 y F","2 y 8 y F","5 y 8 y F","2 y 5 y 8 y F"};
  char arr3[NUM_STRINGS][MAX_LENGTH] = {"NADA","3","6","3 y 6","9","3 y 9","6 y 9","3 y 6 y 9","E","3 y E","6 y E","3 y 6 y E","9 y E","3 y 9 y E","6 y 9 y E","3 y 6 y 9 y E"};
  char arr4[NUM_STRINGS][MAX_LENGTH] = {"NADA","A","B","A y B","C","A y C","B y C","A y B y C","D","A y D","B y D","A y B y D","C y D","A y C y D","B y C y D","A y B y C y D"};

  uint32_t contrasena_escrita; //Valor que se va a devolver
  char string_teclas_pulsadas[MAX_LENGTH]; //Cadena que ira almacenando el valor de la tecla que se pulsa por teclado
  int longitud_teclas_pulsadas;
  int cont_contrasena = 0;

  int i; //Variable auxiliar para el bucle
  int flag_escritura_terminada = 0;
  while(flag_escritura_terminada == 0)
  {
      longitud_teclas_pulsadas = 0;
      valores_teclado = neorv32_gpio_port_get(); //Obtencion del vector de valores del teclado. Solo nos interesan los 16 menos significativos
      valores_teclado = ~valores_teclado; //Se niega. Por lo tanto, si es un 1, significa que esta pulsado, ya que antes era activa a nivel bajo.
      //valores_teclado = valores_teclado << 4;
      //neorv32_uart0_printf("Valor de valores_teclado : %x\n",valores_teclado);

      for(i=0;i<4;i++)
      {
          valores_fila_lectura[i] = valores_teclado & mascaras[i];
          valores_fila_lectura[i] = valores_fila_lectura[i] >>(4*i);
          //neorv32_uart0_printf("Valor de valores_fila_lectura : %x\n",valores_fila_lectura);
          switch(i)
          {
              case 0:
                  if(valores_fila_lectura[i] > 0 && valores_fila_lectura[i] != valores_fila_lectura_anterior[i])
                  {
                      neorv32_uart0_printf("Se ha pulsado : %s.\n",arr1[valores_fila_lectura[i]]);
                      strcpy(string_teclas_pulsadas,arr1[valores_fila_lectura[i]]);
                      longitud_teclas_pulsadas = strlen(string_teclas_pulsadas);
                  }
                  break;
              case 1:
                  if(valores_fila_lectura[i] > 0 && valores_fila_lectura[i] != valores_fila_lectura_anterior[i])
                  {
                      neorv32_uart0_printf("Se ha pulsado : %s.\n",arr2[valores_fila_lectura[i]]);
                      strcpy(string_teclas_pulsadas,arr2[valores_fila_lectura[i]]);
                      longitud_teclas_pulsadas = strlen(string_teclas_pulsadas);
                  }
                  break;
              case 2:
                  if(valores_fila_lectura[i] > 0 && valores_fila_lectura[i] != valores_fila_lectura_anterior[i]) 
                  {
                      neorv32_uart0_printf("Se ha pulsado : %s.\n",arr3[valores_fila_lectura[i]]);
                      strcpy(string_teclas_pulsadas,arr3[valores_fila_lectura[i]]);
                      longitud_teclas_pulsadas = strlen(string_teclas_pulsadas);
                  }
                  break;
              case 3:
                  if(valores_fila_lectura[i] > 0 && valores_fila_lectura[i] != valores_fila_lectura_anterior[i])
                  {
                      neorv32_uart0_printf("Se ha pulsado : %s.\n",arr4[valores_fila_lectura[i]]);
                      strcpy(string_teclas_pulsadas,arr4[valores_fila_lectura[i]]);
                      longitud_teclas_pulsadas = strlen(string_teclas_pulsadas);
                  }
                  break;
              default:
                  break;
          }
        valores_fila_lectura_anterior[i] = valores_fila_lectura[i];
      }

      if(longitud_teclas_pulsadas == 1)
      {
        //int aux = string_teclas_pulsadas[0]-'0'; //Lo convertimos de caracter a entero
        uint32_t aux = strtol(&string_teclas_pulsadas[0],NULL,16);
        uint32_t aux2 = aux;
        neorv32_uart0_printf("El valor de aux2 es : %x.\n",aux2);
        uint32_t aux3 = aux2<<(28-(4*cont_contrasena));
        neorv32_uart0_printf("El valor de aux3 es : %x.\n",aux3);
        contrasena_escrita = contrasena_escrita+aux3;
        cont_contrasena = cont_contrasena+1;
      }
      else if(longitud_teclas_pulsadas > 1)
      {
        neorv32_uart0_printf("Se han pulsado demasiadas teclas a la vez.\n");
      }

      if(cont_contrasena == 8)
      {
        flag_escritura_terminada = 1;
      }
      else if(cont_contrasena > 8)
      {
        neorv32_uart0_printf("cont_contrasena = %x.\n",cont_contrasena);
      }
          
      //Parte encarga de dar valores a la senal del controlador
      //Recordamos que debemos escribir sobre los 7 pines significativos del controlador el valor
      //neorv32_gpio_port_set(porcentaje_servo);
      //neorv32_uart0_printf("El valor de porcentaje servo es : %x.\n",porcentaje_servo);

      
  }

  return contrasena_escrita;
}

