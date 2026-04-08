################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/Users/yassi/Desktop/projet\ github/x-cube-n6-ai-hand-landmarks-main/x-cube-n6-ai-hand-landmarks-main/Lib/AI_Runtime/Npu/Devices/STM32N6XX/mcu_cache.c \
C:/Users/yassi/Desktop/projet\ github/x-cube-n6-ai-hand-landmarks-main/x-cube-n6-ai-hand-landmarks-main/Lib/AI_Runtime/Npu/Devices/STM32N6XX/npu_cache.c 

OBJS += \
./Lib/AI_Runtime/Npu/Devices/STM32N6XX/mcu_cache.o \
./Lib/AI_Runtime/Npu/Devices/STM32N6XX/npu_cache.o 

C_DEPS += \
./Lib/AI_Runtime/Npu/Devices/STM32N6XX/mcu_cache.d \
./Lib/AI_Runtime/Npu/Devices/STM32N6XX/npu_cache.d 


# Each subdirectory must supply rules for building sources it contributes
Lib/AI_Runtime/Npu/Devices/STM32N6XX/mcu_cache.o: C:/Users/yassi/Desktop/projet\ github/x-cube-n6-ai-hand-landmarks-main/x-cube-n6-ai-hand-landmarks-main/Lib/AI_Runtime/Npu/Devices/STM32N6XX/mcu_cache.c Lib/AI_Runtime/Npu/Devices/STM32N6XX/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m55 -std=gnu11 -g3 -DSTM32N657xx -DUSE_FULL_ASSERT -DUSE_FULL_LL_DRIVER -DVECT_TAB_SRAM -DUSE_IMX335_SENSOR -DUSE_VD66GY_SENSOR -DUSE_VD55G1_SENSOR -DSTM32N6570_DK_REV -DHAS_ROTATION_SUPPORT=1 -DTX_MAX_PARALLEL_NETWORKS=1 -DLL_ATON_PLATFORM=LL_ATON_PLAT_STM32N6 -DLL_ATON_OSAL=LL_ATON_OSAL_FREERTOS -DLL_ATON_RT_MODE=LL_ATON_RT_ASYNC -DLL_ATON_SW_FALLBACK -DLL_ATON_DBG_BUFFER_INFO_EXCLUDED=1 -DAPP_HAS_PARALLEL_NETWORKS=0 -DFEAT_FREERTOS -DSCR_LIB_USE_LTDC -c -I../../../Inc -I../../../Src -I../../../STM32Cube_FW_N6/Drivers/STM32N6xx_HAL_Driver/Inc -I../../../STM32Cube_FW_N6/Drivers/STM32N6xx_HAL_Driver/Inc/Legacy -I../../../STM32Cube_FW_N6/Drivers/CMSIS/Device/ST/STM32N6xx/Include -I../../../STM32Cube_FW_N6/Drivers/CMSIS/Include -I../../../STM32Cube_FW_N6/Drivers/CMSIS/DSP/Include -I../../../STM32Cube_FW_N6/Drivers/BSP/Components/Common -I../../../STM32Cube_FW_N6/Drivers/BSP/STM32N6570-DK -I../../../STM32Cube_FW_N6/Utilities/lcd -I../../../Lib/AI_Runtime/Inc -I../../../Lib/AI_Runtime/Npu/ll_aton -I../../../Lib/AI_Runtime/Npu/Devices/STM32N6XX -I../../../Lib/lib_vision_models_pp/lib_vision_models_pp/Inc -I../../../Lib/ai-postprocessing-wrapper -I../../../Lib/Camera_Middleware -I../../../Lib/Camera_Middleware/sensors -I../../../Lib/Camera_Middleware/sensors/imx335 -I../../../Lib/Camera_Middleware/sensors/vd55g1 -I../../../Lib/Camera_Middleware/sensors/vd6g -I../../../Lib/Camera_Middleware/ISP_Library/isp/Inc -I../../../Lib/Camera_Middleware/ISP_Library/evision/Inc -I../../../Lib/FreeRTOS/Source/include -I../../../Lib/FreeRTOS/Source/portable/GCC/ARM_CM55_NTZ/non_secure -I../../../Lib/ipl/Inc -I../../../Lib/screenl/Inc -I../../../Lib/NemaGFX/include -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -mcmse -MMD -MP -MF"Lib/AI_Runtime/Npu/Devices/STM32N6XX/mcu_cache.d" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
Lib/AI_Runtime/Npu/Devices/STM32N6XX/npu_cache.o: C:/Users/yassi/Desktop/projet\ github/x-cube-n6-ai-hand-landmarks-main/x-cube-n6-ai-hand-landmarks-main/Lib/AI_Runtime/Npu/Devices/STM32N6XX/npu_cache.c Lib/AI_Runtime/Npu/Devices/STM32N6XX/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m55 -std=gnu11 -g3 -DSTM32N657xx -DUSE_FULL_ASSERT -DUSE_FULL_LL_DRIVER -DVECT_TAB_SRAM -DUSE_IMX335_SENSOR -DUSE_VD66GY_SENSOR -DUSE_VD55G1_SENSOR -DSTM32N6570_DK_REV -DHAS_ROTATION_SUPPORT=1 -DTX_MAX_PARALLEL_NETWORKS=1 -DLL_ATON_PLATFORM=LL_ATON_PLAT_STM32N6 -DLL_ATON_OSAL=LL_ATON_OSAL_FREERTOS -DLL_ATON_RT_MODE=LL_ATON_RT_ASYNC -DLL_ATON_SW_FALLBACK -DLL_ATON_DBG_BUFFER_INFO_EXCLUDED=1 -DAPP_HAS_PARALLEL_NETWORKS=0 -DFEAT_FREERTOS -DSCR_LIB_USE_LTDC -c -I../../../Inc -I../../../Src -I../../../STM32Cube_FW_N6/Drivers/STM32N6xx_HAL_Driver/Inc -I../../../STM32Cube_FW_N6/Drivers/STM32N6xx_HAL_Driver/Inc/Legacy -I../../../STM32Cube_FW_N6/Drivers/CMSIS/Device/ST/STM32N6xx/Include -I../../../STM32Cube_FW_N6/Drivers/CMSIS/Include -I../../../STM32Cube_FW_N6/Drivers/CMSIS/DSP/Include -I../../../STM32Cube_FW_N6/Drivers/BSP/Components/Common -I../../../STM32Cube_FW_N6/Drivers/BSP/STM32N6570-DK -I../../../STM32Cube_FW_N6/Utilities/lcd -I../../../Lib/AI_Runtime/Inc -I../../../Lib/AI_Runtime/Npu/ll_aton -I../../../Lib/AI_Runtime/Npu/Devices/STM32N6XX -I../../../Lib/lib_vision_models_pp/lib_vision_models_pp/Inc -I../../../Lib/ai-postprocessing-wrapper -I../../../Lib/Camera_Middleware -I../../../Lib/Camera_Middleware/sensors -I../../../Lib/Camera_Middleware/sensors/imx335 -I../../../Lib/Camera_Middleware/sensors/vd55g1 -I../../../Lib/Camera_Middleware/sensors/vd6g -I../../../Lib/Camera_Middleware/ISP_Library/isp/Inc -I../../../Lib/Camera_Middleware/ISP_Library/evision/Inc -I../../../Lib/FreeRTOS/Source/include -I../../../Lib/FreeRTOS/Source/portable/GCC/ARM_CM55_NTZ/non_secure -I../../../Lib/ipl/Inc -I../../../Lib/screenl/Inc -I../../../Lib/NemaGFX/include -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -mcmse -MMD -MP -MF"Lib/AI_Runtime/Npu/Devices/STM32N6XX/npu_cache.d" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Lib-2f-AI_Runtime-2f-Npu-2f-Devices-2f-STM32N6XX

clean-Lib-2f-AI_Runtime-2f-Npu-2f-Devices-2f-STM32N6XX:
	-$(RM) ./Lib/AI_Runtime/Npu/Devices/STM32N6XX/mcu_cache.cyclo ./Lib/AI_Runtime/Npu/Devices/STM32N6XX/mcu_cache.d ./Lib/AI_Runtime/Npu/Devices/STM32N6XX/mcu_cache.o ./Lib/AI_Runtime/Npu/Devices/STM32N6XX/mcu_cache.su ./Lib/AI_Runtime/Npu/Devices/STM32N6XX/npu_cache.cyclo ./Lib/AI_Runtime/Npu/Devices/STM32N6XX/npu_cache.d ./Lib/AI_Runtime/Npu/Devices/STM32N6XX/npu_cache.o ./Lib/AI_Runtime/Npu/Devices/STM32N6XX/npu_cache.su

.PHONY: clean-Lib-2f-AI_Runtime-2f-Npu-2f-Devices-2f-STM32N6XX

