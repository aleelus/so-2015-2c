#!/bin/bash
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
