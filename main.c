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
 int escribe(uint32_t vector_de_pines_out);
 int lectura(void);
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
  neorv32_uart0_print("Blinking LED demo program\n");

int i,j;
uint32_t cont = 1;

int columna;
int fila;
uint32_t primero = 1;
neorv32_uart0_printf("El valor de cont en la primera vuelta es : %u\n",cont);
// use ASM version of LED blinking (file: blink_led_in_asm.S)
#ifdef USE_ASM_VERSION

  blink_led_asm((uint32_t)(&GPIO_OUTPUT));

// use C version of LED blinking
#else
while(1)
{

    
    for(i=0;i<=3;i++)
    {
        if(i==0)
        {
            cont = 1;
        }
        else
        {
            cont <<= 1;
        }
        //neorv32_uart0_printf("El valor de cont es : %u\n",cont);
        fila = escribe(cont);
        columna = i;
        //neorv32_uart0_printf("fila : %u\n",fila);
        //neorv32_uart0_printf("columna : %u\n",columna);
        
        if(columna == 0)
        {
            if(fila == 0)
            {
                neorv32_uart0_printf("Se ha pulsado 1\n");
            }
            
            else if(fila == 1)
            {
                neorv32_uart0_print("Se ha pulsado 4\n");
            }
            else if(fila == 2)
            {
                neorv32_uart0_print("Se ha pulsado 7\n");
            }
            else if(fila == 3)
            {
                neorv32_uart0_print("Se ha pulsado 0\n");
            }
            
        }
        
        else if(columna == 1)
        {
            if(fila == 0)
            {
                neorv32_uart0_print("Se ha pulsado 2\n");
            }
            else if(fila == 1)
            {
                neorv32_uart0_print("Se ha pulsado 5\n");
            }
            else if(fila == 2)
            {
                neorv32_uart0_print("Se ha pulsado 8\n");
            }
            else if(fila == 3)
            {
                neorv32_uart0_print("Se ha pulsado F\n");
            }
        }
        else if(columna == 2)
        {
            
            //neorv32_uart0_printf("El columna 2, valor de fila : %u\n",fila);
            if(fila == 0)
            {
                neorv32_uart0_print("Se ha pulsado 3\n");
            }
            else if(fila == 1)
            {
                neorv32_uart0_print("Se ha pulsado 6\n");
            }
            else if(fila == 2)
            {
                neorv32_uart0_print("Se ha pulsado 9\n");
            }
            else if(fila == 3)
            {
                neorv32_uart0_print("Se ha pulsado E\n");
            }
        }
        else if(columna == 3)
        {
            if(fila == 0)
            {
                neorv32_uart0_print("Se ha pulsado A\n");
            }
            else if(fila == 1)
            {
                neorv32_uart0_print("Se ha pulsado B\n");
            }
            else if(fila == 2)
            {
                neorv32_uart0_print("Se ha pulsado C\n");
            }
            else if(fila == 3)
            {
                neorv32_uart0_print("Se ha pulsado D\n");
            }
        }
        

        
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
int escribe(uint32_t vector_de_pines_out)
{
    
    //neorv32_gpio_pin_set(~vector_de_pines_out); //Se debe activar a nivel bajo 
    neorv32_gpio_port_set(~vector_de_pines_out); //Escribimos sobre todo el puerto de pines
    //neorv32_uart0_printf("El valor negado de cont es : %x\n",~vector_de_pines_out);
    int fila_activada;
    fila_activada = lectura();
    return fila_activada;

}
int lectura(void)
{
    int fila_activada = 6;
    int i;
    for(i=0;i<=3;i++)
    {
            
        if(neorv32_gpio_pin_get(i) == 0) //Porque se activa a nivel bajo. Sin embargo, al haber introducido el rising_edge_detector, hemos cambiado este comportamiento
        {
            fila_activada = i;
            neorv32_uart0_printf("  activada fila: %u.\n", fila_activada);
        }
        
        
    }
    return fila_activada;
}