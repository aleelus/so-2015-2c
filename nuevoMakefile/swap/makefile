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
	-gcc -I"../api" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/conexion.d" -MT"src/conexion.d" -o "src/conexion.o" "conexion.c"
	-gcc -I"../api" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/configuracion.d" -MT"src/configuracion.d" -o "src/configuracion.o" "configuracion.c"
	-gcc -I"../api" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/particionSwap.d" -MT"src/particionSwap.d" -o "src/particionSwap.o" "particionSwap.c"
	-gcc -I"../api" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/swap.d" -MT"src/swap.d" -o "src/swap.o" "swap.c"
	-gcc -L"../api" -o "swap"  ./src/conexion.o ./src/configuracion.o ./src/particionSwap.o ./src/swap.o   -lapi -lpthread -lcommons

-include ../makefile.targets
