-- vsg_off
-- vsg_off
---------------------------------------------------------------------------------
-- Company: 
-- Engineer: Alejandro Torres Muñoz
-- 
-- Create Date: 09/12/2021
-- Design Name: 
-- Module Name: Controlador de escritura-lectura sobre teclado

-- 
----------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;



--Entidad del controlador
entity controlador_teclado is
    Generic(
        numero_pines   : integer; --Numero de pines sobre los que tenemos que escribir y leer
        numero_elementos_teclado : integer --Numero de teclas en el teclado
    );
    Port ( 
    clk : in std_logic;         --Pin de reloj
    reset : in std_logic;       --Pin de reseteo

    --Pines para la comunicacion con el teclado
    pines_lectura_teclado   : in  std_logic_vector(numero_pines-1 downto 0);
    pines_escritura_teclado : out std_logic_vector(numero_pines-1 downto 0);

    --Pines para la comunicacion con el microprocesador

    pines_escritura_micro   : out std_logic_vector(numero_elementos_teclado-1 downto 0)

    );
end controlador_teclado;



architecture Behavioral of controlador_teclado is

--Definicion de senales y constantes auxiliares
constant clk_to_wait : integer := 100; --Numero de senales que esperar

--Contador interno
signal counter : integer := 0; --Contandor para saber en que ciclo estamos 
signal counter_aux : integer := 1; --Contador para convertir en vector logica el valor que debemos escribir sobre las columnas
signal counter_aux_2 : integer := 4; --Contador para saber que elementos del vector logico que enviamos al micro tenemos que escribir
signal flag_lee : std_logic := '0'; --Si es un 0, estamos escribiendo. Si es un 1, estamos leyendo

--Senales para la comunicacion con el teclado
signal pines_lectura_teclado_signal   : std_logic_vector(numero_pines-1 downto 0) := (others => '1'); 
signal pines_escritura_teclado_signal : std_logic_vector(numero_pines-1 downto 0) := not std_logic_vector(to_unsigned(1,numero_pines));

--Senal para comunicacion con el micro
signal pines_escritura_micro_signal     : std_logic_vector(numero_elementos_teclado-1 downto 0); --Senal que escribira sobre los pines para la comunicacion con el micro
signal pines_escritura_micro_signal_aux : std_logic_vector(numero_elementos_teclado-1 downto 0) := (others => '1');  --Senal que escribira sobre los pines para la comunicacion con el micro 
signal aux1                             : std_logic_vector(numero_elementos_teclado-1 downto 0) := (others => '1');
signal aux2                             : std_logic_vector(numero_pines-1 downto 0) := (others => '1'); 
--Comienzo del comportamiento
begin

    --pines_escritura_micro <= pines_escritura_micro_signal_aux; --Escritura sobre los pines del micro
    --pines_lectura_teclado_signal <= pines_lectura_teclado; --Leemos las filas activadas
    pines_escritura_micro <= pines_escritura_micro_signal_aux;
    process(clk,reset) --Proceso sincrono
    begin 
        if reset = '0' then --En el caso de que se pulse el boton de reset
            counter <= 0;
            counter_aux <= 1; --Por el desfase entre escritura y lectura
            counter_aux_2 <= 4;
            flag_lee <= '0';
            pines_escritura_teclado_signal <= not std_logic_vector(to_unsigned(1,numero_pines));
            pines_escritura_micro_signal <= (others => '1'); 
            pines_escritura_micro_signal_aux <= (others => '1'); 
            pines_lectura_teclado_signal <= (others => '1'); 
            aux1 <= (others => '1');
            aux2 <= (others => '1'); 
        elsif(rising_edge(clk)) then --Si no se pulsa el boton de reset, y estamos en flanco de subida del reloj
            --Escritura sobre los pines del teclado
            --pines_escritura_teclado_signal <= not std_logic_vector(to_unsigned(counter_aux,numero_pines)); --El not es porque es a nivel bajo
            pines_escritura_teclado <= not std_logic_vector(to_unsigned(counter_aux,numero_pines)); --Escritura sobre los pines del teclado
            --Lectura de las filas activadas

            pines_escritura_micro_signal((counter_aux_2-1) downto (counter_aux_2-4)) <= pines_lectura_teclado;
            

            if(counter = 3) then --Si contador llega al maximo, se empieza de nuevo y actualizamos la salida hacia el micro
                counter <= 0;
                counter_aux <= 1;
                counter_aux_2 <= 4;
                --aux1 <= pines_escritura_micro_signal;
                --aux2 <= pines_escritura_micro_signal(15 downto 12);
                --aux1(15 downto 4) <= aux1(11 downto 0);
                --aux1(3 downto 0)  <= aux2;
                --pines_escritura_micro_signal_aux <= aux1;
                --pines_escritura_micro_signal_aux <= pines_escritura_micro_signal; --Solo se actualiza cada 3 ciclos de reloj
                
                pines_escritura_micro_signal_aux <= pines_escritura_micro_signal(11 downto 0) & pines_escritura_micro_signal(15 downto 12);
                --pines_escritura_micro <= pines_escritura_micro_signal;
            else                 --Si no, seguimos incrementando
                counter <= counter + 1; --Incremento del contador
                counter_aux <= counter_aux*2; --Incremento del contador auxiliar para el valor binario sobre el que se ha de escribir
                counter_aux_2 <= counter_aux_2 + 4;
            end if;

            ------------------------------------------------------------------------------------------------------------------------------------
            --Con 8 ciclos en total

            --if(flag_lee = '0') then 
                --Estamos en fase de escribir
            --    pines_escritura_teclado_signal <= not std_logic_vector(to_unsigned(counter_aux,numero_pines));   
            --else
                --Luego se lee, e incrementamos counter
                --pines_lectura_teclado_signal <= pines_lectura_teclado; --Leemos las filas activadas
            --    pines_escritura_micro_signal(counter_aux_2-1 downto counter_aux_2-4) <= pines_lectura_teclado;
            --    if(counter = 3) then
            --        counter <= 0;
            --        counter_aux <= 1;
            --        counter_aux_2 <= 4;
            --        pines_escritura_micro_signal_aux <= pines_escritura_micro_signal; --Actualizamos el valor de los pines de escritura sobre el micro
            --    else
            --        counter <= counter + 1;
            --        counter_aux <= counter_aux * 2;
            --        counter_aux_2 <= counter_aux_2 + 4;
            --    end if;

            --end if;

            --flag_lee <= not flag_lee; --Vamos conmutando entre escritura y lectura
        end if;
    end process;

    --pines_escritura_micro <= pines_escritura_micro_signal_aux; --Escritura sobre los pines del micro



end Behavioral;
