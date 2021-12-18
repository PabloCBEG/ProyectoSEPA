# ProyectoSEPA
Proyecto de Sistemas Electrónicos Para Automatización GIERM 2021/22

En este branch se encuentran los archivos de la parte de FPGA dedicados al control del teclado numérico:
  -archivo VHDL del todo el sistema.
  -archivo VHDL del controlador de escritura/lectura sobre el teclado.
  -archivo main.c, que es lo ejecutado por el micro. Simplemente lee unas entradas que recibe del controlador en VHDL y muestra por pantalla el resultado.
  -archivo VHDL para un modulo de debouncer.
  -Se ha llegado a la conclusión de que no es sencillo implementar un detector de flanco en VHDL, debido a la diferencia de frecuencia existente entre el microprocesador y la FPGA. Por tanto, este se ha implementado en .c. Se ve que al activar la detección de flanco sólo durante de un segundo, el micro es muy pocas veces las que llega a detectar el flanco, ya que sólo está activo durante un ciclo de reloj.
  
  -A realizar:
    -Aunque no es sencillo implementar el detector de flanco en VHDL, quizás sería interesante plantear la posibilidad de ejecutarlo de la siguiente manera:
      -Introducir una señal de entrada en el módulo VHDL, a través de la cuál el micro le pueda comunicar que ya se ha leído el valor de su salida.
      -A su vez, una señal de salida, para que el detector de flanco le diga al microprocesador que lea un nuevo valor. Así, el microprocesador, cuando esa señal tenga un valor elevado, leerá los valores de los 16 pines de entrada; y tras ello, poner en alto la entrada que se ha comentado anteriormente, durante un ciclo del microprocesador.
      
  
