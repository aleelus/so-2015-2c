#!/bin/bash
gcc -I"/home/utnso/git/commons" -I"/home/utnso/git/commons/Debug" -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"api.d" -MT"api.d" -o "api.o" "../api.c"

gcc -L"/home/utnso/git/commons/Debug" -shared -o "libapi.so"  ./api.o   -lcommons
