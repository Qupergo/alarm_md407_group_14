##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Debug
ProjectName            :=UltrasonicRange
ConfigurationName      :=Debug
WorkspacePath          :=C:/Users/aminr/Desktop/dataprojekt
ProjectPath            :=C:/Users/aminr/Desktop/dataprojekt/UltrasonicRange
IntermediateDirectory  :=$(ConfigurationName)
OutDir                 := $(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=aminr
Date                   :=22/09/2022
CodeLitePath           :=C:/cseapp/CodeLite
LinkerName             :=$(CodeLiteDir)/tools/gcc-arm/bin/arm-none-eabi-g++.exe
SharedObjectLinkerName :=$(CodeLiteDir)/tools/gcc-arm/arm-none-eabi-g++.exe -shared -fPIC
ObjectSuffix           :=.o
DependSuffix           :=.o.d
PreprocessSuffix       :=.i
DebugSwitch            :=-g 
IncludeSwitch          :=-I
LibrarySwitch          :=-l
OutputSwitch           :=-o 
LibraryPathSwitch      :=-L
PreprocessorSwitch     :=-D
SourceSwitch           :=-c 
OutputFile             :=$(IntermediateDirectory)/$(ProjectName)
Preprocessors          :=
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=-E
ObjectsFileList        :="UltrasonicRange.txt"
PCHCompileFlags        :=
MakeDirCommand         :=makedir
RcCmpOptions           := 
RcCompilerName         :=
LinkOptions            :=  -T$(ProjectPath)/md407-ram.x -L$(ARM_V6LIB) -L$(ARM_GCC_V6LIB) -nostdlib -nostartfiles
IncludePath            :=  $(IncludeSwitch). $(IncludeSwitch). $(IncludeSwitch)C:/Users/aminr/Desktop/dataprojekt/STM32F4_lib/STM32F4_lib/CMSIS/Include $(IncludeSwitch)C:/Users/aminr/Desktop/dataprojekt/STM32F4_lib/STM32F4_lib/STM32F4xx_StdPeriph_Driver/inc 
IncludePCH             := 
RcIncludePath          := 
Libs                   := $(LibrarySwitch)gcc $(LibrarySwitch)c_nano 
ArLibs                 :=  "gcc" "c_nano" 
LibPath                := $(LibraryPathSwitch). 

##
## Common variables
## AR, CXX, CC, AS, CXXFLAGS and CFLAGS can be overriden using an environment variables
##
AR       := $(CodeLiteDir)/tools/gcc-arm/bin/arm-none-eabi-ar.exe rcu
CXX      := $(CodeLiteDir)/tools/gcc-arm/bin/arm-none-eabi-g++.exe
CC       := $(CodeLiteDir)/tools/gcc-arm/bin/arm-none-eabi-gcc.exe
CXXFLAGS :=  -g -O0 -W $(Preprocessors)
CFLAGS   :=  -g -O0 -w -mthumb -march=armv6-m  -mfloat-abi=soft -std=c99 $(Preprocessors)
ASFLAGS  := 
AS       := $(CodeLiteDir)/tools/gcc-arm/bin/arm-none-eabi-as.exe


##
## User defined environment variables
##
CodeLiteDir:=C:\cseapp\CodeLite
ARM_V6LIB:=$(CodeLiteDir)/tools/gcc-arm/arm-none-eabi/lib/thumb/v6-m/nofp
ARM_GCC_V6LIB:=$(CodeLiteDir)/tools/gcc-arm/lib/gcc/arm-none-eabi/9.2.1/thumb/v6-m/nofp
ARM_M4SFPLIB:=$(CodeLiteDir)/tools/gcc-arm/arm-none-eabi/lib/thumb/v7e-m+fp/softfp
ARM_GCC_M4SFPLIB:=$(CodeLiteDir)/tools/gcc-arm/lib/gcc/arm-none-eabi/9.2.1/thumb/v7e-m+fp/softfp
ARM_M4HFPLIB:=$(CodeLiteDir)/tools/gcc-arm/arm-none-eabi/lib/thumb/v7e-m+fp/hard
ARM_GCC_M4HFPLIB:=$(CodeLiteDir)/tools/gcc-arm/lib/gcc/arm-none-eabi/9.2.1/thumb/v7e-m+fp/hard
Objects0=$(IntermediateDirectory)/up_STM32F4_lib_STM32F4_lib_STM32F4xx_StdPeriph_Driver_src_stm32f4xx_rcc.c$(ObjectSuffix) $(IntermediateDirectory)/startup.c$(ObjectSuffix) $(IntermediateDirectory)/up_STM32F4_lib_STM32F4_lib_STM32F4xx_StdPeriph_Driver_src_stm32f4xx_gpio.c$(ObjectSuffix) $(IntermediateDirectory)/up_chat_app_usart.c$(ObjectSuffix) $(IntermediateDirectory)/up_STM32F4_lib_STM32F4_lib_STM32F4xx_StdPeriph_Driver_src_stm32f4xx_tim.c$(ObjectSuffix) 



Objects=$(Objects0) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild MakeIntermediateDirs
all: $(OutputFile)

$(OutputFile): $(IntermediateDirectory)/.d $(Objects) 
	@$(MakeDirCommand) $(@D)
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects0)  > $(ObjectsFileList)
	$(LinkerName) $(OutputSwitch)$(OutputFile) @$(ObjectsFileList) $(LibPath) $(Libs) $(LinkOptions)

PostBuild:
	@echo Executing Post Build commands ...
	C:\cseapp\CodeLite/tools/gcc-arm/bin/arm-none-eabi-objcopy -S -O srec  Debug/UltrasonicRange Debug/UltrasonicRange.s19
	C:\cseapp\CodeLite/tools/gcc-arm/bin/arm-none-eabi-objdump -D -S Debug/UltrasonicRange > Debug/UltrasonicRange.lst
	@echo Done

MakeIntermediateDirs:
	@$(MakeDirCommand) "$(ConfigurationName)"


$(IntermediateDirectory)/.d:
	@$(MakeDirCommand) "$(ConfigurationName)"

PreBuild:


##
## Objects
##
$(IntermediateDirectory)/up_STM32F4_lib_STM32F4_lib_STM32F4xx_StdPeriph_Driver_src_stm32f4xx_rcc.c$(ObjectSuffix): ../STM32F4_lib/STM32F4_lib/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rcc.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_STM32F4_lib_STM32F4_lib_STM32F4xx_StdPeriph_Driver_src_stm32f4xx_rcc.c$(ObjectSuffix) -MF$(IntermediateDirectory)/up_STM32F4_lib_STM32F4_lib_STM32F4xx_StdPeriph_Driver_src_stm32f4xx_rcc.c$(DependSuffix) -MM ../STM32F4_lib/STM32F4_lib/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rcc.c
	$(CC) $(SourceSwitch) "C:/Users/aminr/Desktop/dataprojekt/STM32F4_lib/STM32F4_lib/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rcc.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_STM32F4_lib_STM32F4_lib_STM32F4xx_StdPeriph_Driver_src_stm32f4xx_rcc.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_STM32F4_lib_STM32F4_lib_STM32F4xx_StdPeriph_Driver_src_stm32f4xx_rcc.c$(PreprocessSuffix): ../STM32F4_lib/STM32F4_lib/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rcc.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_STM32F4_lib_STM32F4_lib_STM32F4xx_StdPeriph_Driver_src_stm32f4xx_rcc.c$(PreprocessSuffix) ../STM32F4_lib/STM32F4_lib/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rcc.c

$(IntermediateDirectory)/startup.c$(ObjectSuffix): startup.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/startup.c$(ObjectSuffix) -MF$(IntermediateDirectory)/startup.c$(DependSuffix) -MM startup.c
	$(CC) $(SourceSwitch) "C:/Users/aminr/Desktop/dataprojekt/UltrasonicRange/startup.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/startup.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/startup.c$(PreprocessSuffix): startup.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/startup.c$(PreprocessSuffix) startup.c

$(IntermediateDirectory)/up_STM32F4_lib_STM32F4_lib_STM32F4xx_StdPeriph_Driver_src_stm32f4xx_gpio.c$(ObjectSuffix): ../STM32F4_lib/STM32F4_lib/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_gpio.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_STM32F4_lib_STM32F4_lib_STM32F4xx_StdPeriph_Driver_src_stm32f4xx_gpio.c$(ObjectSuffix) -MF$(IntermediateDirectory)/up_STM32F4_lib_STM32F4_lib_STM32F4xx_StdPeriph_Driver_src_stm32f4xx_gpio.c$(DependSuffix) -MM ../STM32F4_lib/STM32F4_lib/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_gpio.c
	$(CC) $(SourceSwitch) "C:/Users/aminr/Desktop/dataprojekt/STM32F4_lib/STM32F4_lib/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_gpio.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_STM32F4_lib_STM32F4_lib_STM32F4xx_StdPeriph_Driver_src_stm32f4xx_gpio.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_STM32F4_lib_STM32F4_lib_STM32F4xx_StdPeriph_Driver_src_stm32f4xx_gpio.c$(PreprocessSuffix): ../STM32F4_lib/STM32F4_lib/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_gpio.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_STM32F4_lib_STM32F4_lib_STM32F4xx_StdPeriph_Driver_src_stm32f4xx_gpio.c$(PreprocessSuffix) ../STM32F4_lib/STM32F4_lib/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_gpio.c

$(IntermediateDirectory)/up_chat_app_usart.c$(ObjectSuffix): ../chat_app/usart.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_chat_app_usart.c$(ObjectSuffix) -MF$(IntermediateDirectory)/up_chat_app_usart.c$(DependSuffix) -MM ../chat_app/usart.c
	$(CC) $(SourceSwitch) "C:/Users/aminr/Desktop/dataprojekt/chat_app/usart.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_chat_app_usart.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_chat_app_usart.c$(PreprocessSuffix): ../chat_app/usart.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_chat_app_usart.c$(PreprocessSuffix) ../chat_app/usart.c

$(IntermediateDirectory)/up_STM32F4_lib_STM32F4_lib_STM32F4xx_StdPeriph_Driver_src_stm32f4xx_tim.c$(ObjectSuffix): ../STM32F4_lib/STM32F4_lib/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_tim.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_STM32F4_lib_STM32F4_lib_STM32F4xx_StdPeriph_Driver_src_stm32f4xx_tim.c$(ObjectSuffix) -MF$(IntermediateDirectory)/up_STM32F4_lib_STM32F4_lib_STM32F4xx_StdPeriph_Driver_src_stm32f4xx_tim.c$(DependSuffix) -MM ../STM32F4_lib/STM32F4_lib/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_tim.c
	$(CC) $(SourceSwitch) "C:/Users/aminr/Desktop/dataprojekt/STM32F4_lib/STM32F4_lib/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_tim.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_STM32F4_lib_STM32F4_lib_STM32F4xx_StdPeriph_Driver_src_stm32f4xx_tim.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_STM32F4_lib_STM32F4_lib_STM32F4xx_StdPeriph_Driver_src_stm32f4xx_tim.c$(PreprocessSuffix): ../STM32F4_lib/STM32F4_lib/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_tim.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_STM32F4_lib_STM32F4_lib_STM32F4xx_StdPeriph_Driver_src_stm32f4xx_tim.c$(PreprocessSuffix) ../STM32F4_lib/STM32F4_lib/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_tim.c


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) -r $(ConfigurationName)/


