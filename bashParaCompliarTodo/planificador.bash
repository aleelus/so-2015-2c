#!/bin/bash
gcc -I"/home/utnso/git/tp-2015-2c-so-didnt-c-that-coming/api" -I"/home/utnso/git/commons" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/Utilidades/Utils.d" -MT"src/Utilidades/Utils.d" -o "src/Utilidades/Utils.o" "../src/Utilidades/Utils.c"

gcc -I"/home/utnso/git/tp-2015-2c-so-didnt-c-that-coming/api" -I"/home/utnso/git/commons" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/Consola.d" -MT"src/Consola.d" -o "src/Consola.o" "../src/Consola.c"

gcc -I"/home/utnso/git/tp-2015-2c-so-didnt-c-that-coming/api" -I"/home/utnso/git/commons" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/Dispatcher.d" -MT"src/Dispatcher.d" -o "src/Dispatcher.o" "../src/Dispatcher.c"

gcc -I"/home/utnso/git/tp-2015-2c-so-didnt-c-that-coming/api" -I"/home/utnso/git/commons" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/planificador.d" -MT"src/planificador.d" -o "src/planificador.o" "../src/planificador.c"

gcc -L"/home/utnso/git/commons/Debug" -L"/home/utnso/git/tp-2015-2c-so-didnt-c-that-coming/api/Debug" -o "planificador"  ./src/Utilidades/Utils.o  ./src/Consola.o ./src/Dispatcher.o ./src/planificador.o   -lcommons -lapi -lpthread
