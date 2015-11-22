################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# CPU
all: cpu

dependents:
	-cd /../../../../../../../../lib && $(MAKE) all

# Tool invocations
cpu: $(OBJS) $(USER_OBJS) /../../../../lib/libapi.so
	@echo 'Compilando... $@'
	@echo 'Invoking: GCC C Linker'
	gcc -L"/../../../../../../../../lib" -o "cpu" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finalizo: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) cpu
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:
/../../../../../../../../lib/libapi.so:

-include ../makefile.targets
