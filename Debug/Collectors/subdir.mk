################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Collectors/Collector.cpp \
../Collectors/CopyingCollector.cpp \
../Collectors/MarkSweepCollector.cpp \
../Collectors/TraversalCollector.cpp \
../Collectors/ReferenceCountingCollector.cpp

OBJS += \
./Collectors/Collector.o \
./Collectors/CopyingCollector.o \
./Collectors/MarkSweepCollector.o \
./Collectors/TraversalCollector.o \
./Collectors/ReferenceCountingCollector.o

CPP_DEPS += \
./Collectors/Collector.d \
./Collectors/CopyingCollector.d \
./Collectors/MarkSweepCollector.d \
./Collectors/TraversalCollector.d \
./Collectors/ReferenceCountingCollector.d

# Each subdirectory must supply rules for building sources it contributes
Collectors/%.o: ../Collectors/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


