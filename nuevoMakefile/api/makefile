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
	-gcc -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"./src/api.d" -MT"./src/api.d" -o "api.o" "api.c"	
	-gcc -shared -o "libapi.so"  ./api.o   -lcommons 
	-sudo cp ./libapi.so /lib
-include ../makefile.targets
