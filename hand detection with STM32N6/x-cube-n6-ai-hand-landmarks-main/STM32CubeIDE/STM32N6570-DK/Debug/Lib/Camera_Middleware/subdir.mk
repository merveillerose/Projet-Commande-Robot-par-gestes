################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/Users/yassi/Desktop/projet\ github/x-cube-n6-ai-hand-landmarks-main/x-cube-n6-ai-hand-landmarks-main/Lib/Camera_Middleware/cmw_camera.c \
C:/Users/yassi/Desktop/projet\ github/x-cube-n6-ai-hand-landmarks-main/x-cube-n6-ai-hand-landmarks-main/Lib/Camera_Middleware/cmw_utils.c 

OBJS += \
./Lib/Camera_Middleware/cmw_camera.o \
./Lib/Camera_Middleware/cmw_utils.o 

C_DEPS += \
./Lib/Camera_Middleware/cmw_camera.d \
./Lib/Camera_Middleware/cmw_utils.d 


# Each subdirectory must supply rules for building sources it contributes
Lib/Camera_Middleware/cmw_camera.o: C:/Users/yassi/Desktop/projet\ github/x-cube-n6-ai-hand-landmarks-main/x-cube-n6-ai-hand-landmarks-main/Lib/Camera_Middleware/cmw_camera.c Lib/Camera_Middleware/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m55 -std=gnu11 -g3 -DSTM32N657xx -DUSE_FULL_ASSERT -DUSE_FULL_LL_DRIVER -DVECT_TAB_SRAM -DUSE_IMX335_SENSOR -DUSE_VD66GY_SENSOR -DUSE_VD55G1_SENSOR -DSTM32N6570_DK_REV -DHAS_ROTATION_SUPPORT=1 -DTX_MAX_PARALLEL_NETWORKS=1 -DLL_ATON_PLATFORM=LL_ATON_PLAT_STM32N6 -DLL_ATON_OSAL=LL_ATON_OSAL_FREERTOS -DLL_ATON_RT_MODE=LL_ATON_RT_ASYNC -DLL_ATON_SW_FALLBACK -DLL_ATON_DBG_BUFFER_INFO_EXCLUDED=1 -DAPP_HAS_PARALLEL_NETWORKS=0 -DFEAT_FREERTOS -DSCR_LIB_USE_LTDC -c -I../../../Inc -I../../../Src -I../../../STM32Cube_FW_N6/Drivers/STM32N6xx_HAL_Driver/Inc -I../../../STM32Cube_FW_N6/Drivers/STM32N6xx_HAL_Driver/Inc/Legacy -I../../../STM32Cube_FW_N6/Drivers/CMSIS/Device/ST/STM32N6xx/Include -I../../../STM32Cube_FW_N6/Drivers/CMSIS/Include -I../../../STM32Cube_FW_N6/Drivers/CMSIS/DSP/Include -I../../../STM32Cube_FW_N6/Drivers/BSP/Components/Common -I../../../STM32Cube_FW_N6/Drivers/BSP/STM32N6570-DK -I../../../STM32Cube_FW_N6/Utilities/lcd -I../../../Lib/AI_Runtime/Inc -I../../../Lib/AI_Runtime/Npu/ll_aton -I../../../Lib/AI_Runtime/Npu/Devices/STM32N6XX -I../../../Lib/lib_vision_models_pp/lib_vision_models_pp/Inc -I../../../Lib/ai-postprocessing-wrapper -I../../../Lib/Camera_Middleware -I../../../Lib/Camera_Middleware/sensors -I../../../Lib/Camera_Middleware/sensors/imx335 -I../../../Lib/Camera_Middleware/sensors/vd55g1 -I../../../Lib/Camera_Middleware/sensors/vd6g -I../../../Lib/Camera_Middleware/ISP_Library/isp/Inc -I../../../Lib/Camera_Middleware/ISP_Library/evision/Inc -I../../../Lib/FreeRTOS/Source/include -I../../../Lib/FreeRTOS/Source/portable/GCC/ARM_CM55_NTZ/non_secure -I../../../Lib/ipl/Inc -I../../../Lib/screenl/Inc -I../../../Lib/NemaGFX/include -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -mcmse -MMD -MP -MF"Lib/Camera_Middleware/cmw_camera.d" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
Lib/Camera_Middleware/cmw_utils.o: C:/Users/yassi/Desktop/projet\ github/x-cube-n6-ai-hand-landmarks-main/x-cube-n6-ai-hand-landmarks-main/Lib/Camera_Middleware/cmw_utils.c Lib/Camera_Middleware/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m55 -std=gnu11 -g3 -DSTM32N657xx -DUSE_FULL_ASSERT -DUSE_FULL_LL_DRIVER -DVECT_TAB_SRAM -DUSE_IMX335_SENSOR -DUSE_VD66GY_SENSOR -DUSE_VD55G1_SENSOR -DSTM32N6570_DK_REV -DHAS_ROTATION_SUPPORT=1 -DTX_MAX_PARALLEL_NETWORKS=1 -DLL_ATON_PLATFORM=LL_ATON_PLAT_STM32N6 -DLL_ATON_OSAL=LL_ATON_OSAL_FREERTOS -DLL_ATON_RT_MODE=LL_ATON_RT_ASYNC -DLL_ATON_SW_FALLBACK -DLL_ATON_DBG_BUFFER_INFO_EXCLUDED=1 -DAPP_HAS_PARALLEL_NETWORKS=0 -DFEAT_FREERTOS -DSCR_LIB_USE_LTDC -c -I../../../Inc -I../../../Src -I../../../STM32Cube_FW_N6/Drivers/STM32N6xx_HAL_Driver/Inc -I../../../STM32Cube_FW_N6/Drivers/STM32N6xx_HAL_Driver/Inc/Legacy -I../../../STM32Cube_FW_N6/Drivers/CMSIS/Device/ST/STM32N6xx/Include -I../../../STM32Cube_FW_N6/Drivers/CMSIS/Include -I../../../STM32Cube_FW_N6/Drivers/CMSIS/DSP/Include -I../../../STM32Cube_FW_N6/Drivers/BSP/Components/Common -I../../../STM32Cube_FW_N6/Drivers/BSP/STM32N6570-DK -I../../../STM32Cube_FW_N6/Utilities/lcd -I../../../Lib/AI_Runtime/Inc -I../../../Lib/AI_Runtime/Npu/ll_aton -I../../../Lib/AI_Runtime/Npu/Devices/STM32N6XX -I../../../Lib/lib_vision_models_pp/lib_vision_models_pp/Inc -I../../../Lib/ai-postprocessing-wrapper -I../../../Lib/Camera_Middleware -I../../../Lib/Camera_Middleware/sensors -I../../../Lib/Camera_Middleware/sensors/imx335 -I../../../Lib/Camera_Middleware/sensors/vd55g1 -I../../../Lib/Camera_Middleware/sensors/vd6g -I../../../Lib/Camera_Middleware/ISP_Library/isp/Inc -I../../../Lib/Camera_Middleware/ISP_Library/evision/Inc -I../../../Lib/FreeRTOS/Source/include -I../../../Lib/FreeRTOS/Source/portable/GCC/ARM_CM55_NTZ/non_secure -I../../../Lib/ipl/Inc -I../../../Lib/screenl/Inc -I../../../Lib/NemaGFX/include -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -mcmse -MMD -MP -MF"Lib/Camera_Middleware/cmw_utils.d" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Lib-2f-Camera_Middleware

clean-Lib-2f-Camera_Middleware:
	-$(RM) ./Lib/Camera_Middleware/cmw_camera.cyclo ./Lib/Camera_Middleware/cmw_camera.d ./Lib/Camera_Middleware/cmw_camera.o ./Lib/Camera_Middleware/cmw_camera.su ./Lib/Camera_Middleware/cmw_utils.cyclo ./Lib/Camera_Middleware/cmw_utils.d ./Lib/Camera_Middleware/cmw_utils.o ./Lib/Camera_Middleware/cmw_utils.su

.PHONY: clean-Lib-2f-Camera_Middleware

