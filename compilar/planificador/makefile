################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include /Utilidades/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: planificador

dependents:
	-cd /../../../../lib && $(MAKE) clean all

# Tool invocations
planificador: $(OBJS) $(USER_OBJS) /../../../../lib/libapi.so
	@echo 'Compilando... $@'
	@echo 'Invoking: GCC C Linker'
	gcc -L"/../../../../lib" -o "planificador" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finalizado $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) planificador
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:
/../../../../lib/libapi.so:

-include ../makefile.targets
