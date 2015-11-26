################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf


ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

all:
	-gcc -I"../api" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/Utilidades/Utils.d" -MT"src/Utilidades/Utils.d" -o "src/Utilidades/Utils.o" "Utilidades/Utils.c"
	-gcc -I"../api" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/Consola.d" -MT"src/Consola.d" -o "src/Consola.o" "Consola.c"
	-gcc -I"../api" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/Dispatcher.d" -MT"src/Dispatcher.d" -o "src/Dispatcher.o" "Dispatcher.c"
	-gcc -I"../api" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/planificador.d" -MT"src/planificador.d" -o "src/planificador.o" "planificador.c"
	-gcc -L"../api" -o "planificador"  ./src/Utilidades/Utils.o  ./src/Consola.o ./src/Dispatcher.o ./src/planificador.o   -lcommons -lapi -lpthread

-include ../makefile.targets
