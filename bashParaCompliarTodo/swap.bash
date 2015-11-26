#!/bin/bash
gcc -I"/home/utnso/git/tp-2015-2c-so-didnt-c-that-coming/api" -I"/home/utnso/git/commons" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/conexion.d" -MT"src/conexion.d" -o "src/conexion.o" "../src/conexion.c"

gcc -I"/home/utnso/git/tp-2015-2c-so-didnt-c-that-coming/api" -I"/home/utnso/git/commons" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/configuracion.d" -MT"src/configuracion.d" -o "src/configuracion.o" "../src/configuracion.c"

gcc -I"/home/utnso/git/tp-2015-2c-so-didnt-c-that-coming/api" -I"/home/utnso/git/commons" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/particionSwap.d" -MT"src/particionSwap.d" -o "src/particionSwap.o" "../src/particionSwap.c"
 
gcc -I"/home/utnso/git/tp-2015-2c-so-didnt-c-that-coming/api" -I"/home/utnso/git/commons" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/swap.d" -MT"src/swap.d" -o "src/swap.o" "../src/swap.c"

gcc -L"/home/utnso/git/tp-2015-2c-so-didnt-c-that-coming/api/Debug" -L"/home/utnso/git/commons/Debug" -o "swap"  ./src/conexion.o ./src/configuracion.o ./src/particionSwap.o ./src/swap.o   -lapi -lpthread -lcommons
