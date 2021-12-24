---------------------------------------------------------------------------------
-- Company: Hogwarts Escuela de Magia y Hechicería
-- Engineer: Alejandro Torres Muñoz
-- 
-- Create Date: 20/12/2021
-- Design Name: 
-- Module Name: Controlador servo

-- 
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;
use IEEE.MATH_REAL.ALL;

--Entidad del controlador del servomotor
entity controlador_servo is
    Generic (
        numero_pines  : integer; --Numero de pines de servo(4)
        pin_conectado : integer  --Pin al que conectamos nuestra salida
    );
    Port ( 
        clk                     : in std_logic;                                 --Pin de reloj
        reset                   : in std_logic;                                 --Pin de reseteo
        entrada_microsegundos   : in std_logic_vector(10 downto 0);             --Porcentaje al que queremos que se mueva el servo
        port_output             : out std_logic_vector(numero_pines-1 downto 0) --Boton de salida
    );
end controlador_servo;



architecture Behavioral of controlador_servo is


constant maximo_pulsos : integer := 27600;  --2300us * 12pulsos/us (pues la freq del micro es 12MHz == 12*10⁶ ciclos/s)
constant pulsos_offset : integer := 17220;  --Valor calculado experimentalmente: 1500us * 12pulsos/us. Pero se comprueba que para 1500*12 el servo no se queda quieto. Por tanto se ha ido probando con distintos valores hasta que se ha dado con el adecuado.
                                            --Es 1435us * 12pulsos/us

signal pulse_width  : integer := pulsos_offset; --Numero de pulsos en alto
signal contador     : integer := 0;
signal aux          : integer := 0;

--Comienzo del comportamiento / behavioral
begin

    process(clk,reset)
    begin
        if reset = '0' then --Si reseteamos, todo a 0
            contador    <= 0;
            pulse_width <= pulsos_offset;
            port_output <= std_logic_vector(to_unsigned(0,numero_pines));
            aux         <= 0;
            
        elsif rising_edge(clk) then --Si no reseteamos, si hay flanco ascendente de reloj
            if(contador = maximo_pulsos-1) then 
                contador <= 0;
            else
                if(contador = 0) then --Si estamos en el primer ciclo de reloj, actualizamos el valor del pulse_width
                    aux         <= (to_integer(signed(entrada_microsegundos))*12);
                    pulse_width <= pulsos_offset+aux; 
                else
                    
                    if(contador < pulse_width) then
                        for i in 0 to numero_pines-1 loop
                            if (i = pin_conectado) then --Escribimos solo el valor alto sobre el pin del servo
                                port_output(i) <= '1';  --En el pin del servo, valor alto
                            else
                                port_output(i) <= '0';
                            end if;
                        end loop; 
                    elsif(contador >= pulse_width) then --Si ya hemos terminado el ancho de pulso, todos a 0
                        port_output <= std_logic_vector(to_unsigned(0,numero_pines)); 
                    end if;
                end if;
                contador <= contador+1;
            end if;
        end if;
    end process;

end Behavioral;
