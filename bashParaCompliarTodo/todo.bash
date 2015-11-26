#!/bin/bash
#commons
gcc -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"commons/collections/dictionary.d" -MT"commons/collections/dictionary.d" -o "commons/collections/dictionary.o" "../commons/collections/dictionary.c"

gcc -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"commons/collections/list.d" -MT"commons/collections/list.d" -o "commons/collections/list.o" "../commons/collections/list.c"

gcc -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"commons/collections/queue.d" -MT"commons/collections/queue.d" -o "commons/collections/queue.o" "../commons/collections/queue.c"

gcc -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"commons/bitarray.d" -MT"commons/bitarray.d" -o "commons/bitarray.o" "../commons/bitarray.c"
 
gcc -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"commons/config.d" -MT"commons/config.d" -o "commons/config.o" "../commons/config.c"
 
gcc -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"commons/error.d" -MT"commons/error.d" -o "commons/error.o" "../commons/error.c"
 
gcc -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"commons/log.d" -MT"commons/log.d" -o "commons/log.o" "../commons/log.c"
 
gcc -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"commons/process.d" -MT"commons/process.d" -o "commons/process.o" "../commons/process.c"
 
gcc -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"commons/string.d" -MT"commons/string.d" -o "commons/string.o" "../commons/string.c"
 
gcc -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"commons/temporal.d" -MT"commons/temporal.d" -o "commons/temporal.o" "../commons/temporal.c"
 
gcc -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"commons/txt.d" -MT"commons/txt.d" -o "commons/txt.o" "../commons/txt.c"
 
gcc -shared -o "libcommons.so"  ./commons/collections/dictionary.o ./commons/collections/list.o ./commons/collections/queue.o  ./commons/bitarray.o ./commons/config.o ./commons/error.o ./commons/log.o ./commons/process.o ./commons/string.o ./commons/temporal.o ./commons/txt.o
#api
gcc -I"/home/utnso/git/commons" -I"/home/utnso/git/commons/Debug" -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"api.d" -MT"api.d" -o "api.o" "../api.c"

gcc -L"/home/utnso/git/commons/Debug" -shared -o "libapi.so"  ./api.o   -lcommons
#swap
gcc -I"/home/utnso/git/tp-2015-2c-so-didnt-c-that-coming/api" -I"/home/utnso/git/commons" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/conexion.d" -MT"src/conexion.d" -o "src/conexion.o" "../src/conexion.c"

gcc -I"/home/utnso/git/tp-2015-2c-so-didnt-c-that-coming/api" -I"/home/utnso/git/commons" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/configuracion.d" -MT"src/configuracion.d" -o "src/configuracion.o" "../src/configuracion.c"

gcc -I"/home/utnso/git/tp-2015-2c-so-didnt-c-that-coming/api" -I"/home/utnso/git/commons" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/particionSwap.d" -MT"src/particionSwap.d" -o "src/particionSwap.o" "../src/particionSwap.c"
 
gcc -I"/home/utnso/git/tp-2015-2c-so-didnt-c-that-coming/api" -I"/home/utnso/git/commons" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/swap.d" -MT"src/swap.d" -o "src/swap.o" "../src/swap.c"

gcc -L"/home/utnso/git/tp-2015-2c-so-didnt-c-that-coming/api/Debug" -L"/home/utnso/git/commons/Debug" -o "swap"  ./src/conexion.o ./src/configuracion.o ./src/particionSwap.o ./src/swap.o   -lapi -lpthread -lcommons
#memoria
gcc -I"/home/utnso/git/commons" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/memoria.d" -MT"src/memoria.d" -o "src/memoria.o" "../src/memoria.c"

gcc -L"/home/utnso/git/commons/Debug" -o "memoria"  ./src/memoria.o   -lpthread -lcommons
#planificador
gcc -I"/home/utnso/git/tp-2015-2c-so-didnt-c-that-coming/api" -I"/home/utnso/git/commons" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/Utilidades/Utils.d" -MT"src/Utilidades/Utils.d" -o "src/Utilidades/Utils.o" "../src/Utilidades/Utils.c"

gcc -I"/home/utnso/git/tp-2015-2c-so-didnt-c-that-coming/api" -I"/home/utnso/git/commons" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/Consola.d" -MT"src/Consola.d" -o "src/Consola.o" "../src/Consola.c"

gcc -I"/home/utnso/git/tp-2015-2c-so-didnt-c-that-coming/api" -I"/home/utnso/git/commons" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/Dispatcher.d" -MT"src/Dispatcher.d" -o "src/Dispatcher.o" "../src/Dispatcher.c"

gcc -I"/home/utnso/git/tp-2015-2c-so-didnt-c-that-coming/api" -I"/home/utnso/git/commons" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/planificador.d" -MT"src/planificador.d" -o "src/planificador.o" "../src/planificador.c"

gcc -L"/home/utnso/git/commons/Debug" -L"/home/utnso/git/tp-2015-2c-so-didnt-c-that-coming/api/Debug" -o "planificador"  ./src/Utilidades/Utils.o  ./src/Consola.o ./src/Dispatcher.o ./src/planificador.o   -lcommons -lapi -lpthread
#cpu
gcc -I"/home/utnso/git/tp-2015-2c-so-didnt-c-that-coming/api" -I"/home/utnso/git/commons" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/cpu.d" -MT"src/cpu.d" -o "src/cpu.o" "../src/cpu.c"

gcc -I"/home/utnso/git/tp-2015-2c-so-didnt-c-that-coming/api" -I"/home/utnso/git/commons" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/interprete.d" -MT"src/interprete.d" -o "src/interprete.o" "../src/interprete.c"

gcc -L"/home/utnso/git/commons/Debug" -L"/home/utnso/git/tp-2015-2c-so-didnt-c-that-coming/api/Debug" -o "cpu"  ./src/cpu.o ./src/interprete.o   -lcommons -lapi -lpthread
echo "it is done"
