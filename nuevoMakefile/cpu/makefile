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
	-gcc -I"../api" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/cpu.d" -MT"src/cpu.d" -o "src/cpu.o" "cpu.c"	
	-gcc -I"../api" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/interprete.d" -MT"src/interprete.d" -o "src/interprete.o" "interprete.c" 
	-gcc -L"../api" -o "cpu"  ./src/cpu.o ./src/interprete.o   -lcommons -lapi -lpthread
-include ../makefile.targets
