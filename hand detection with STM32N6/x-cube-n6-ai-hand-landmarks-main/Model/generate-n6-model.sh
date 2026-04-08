#!/bin/bash

stedgeai generate --no-inputs-allocation --name palm_detector --model 033_palm_detection_full_quant_pc_uf_od.tflite --target stm32n6 --st-neural-art palm_detector@user_neuralart.json
cp st_ai_output/palm_detector_ecblobs.h .
cp st_ai_output/palm_detector.c .
cp st_ai_output/palm_detector_atonbuf.xSPI2.raw st_ai_output/palm_detector_data.xSPI2.bin
arm-none-eabi-objcopy -I binary st_ai_output/palm_detector_data.xSPI2.bin --change-addresses 0x70380000 -O ihex palm_detector_data.hex

stedgeai generate --name hand_landmark --model 033_hand_landmark_full_quant_pc_uf_handl.tflite --target stm32n6 --st-neural-art hand_landmark@user_neuralart.json
cp st_ai_output/hand_landmark_ecblobs.h .
cp st_ai_output/hand_landmark.c .
cp st_ai_output/hand_landmark_atonbuf.xSPI2.raw st_ai_output/hand_landmark_data.xSPI2.bin
arm-none-eabi-objcopy -I binary st_ai_output/hand_landmark_data.xSPI2.bin --change-addresses 0x70580000 -O ihex hand_landmark_data.hex
