/**
  ******************************************************************************
  * @file    IPL_nearest_private.h
  * @author  AIS Team
  * @brief   STM32 Image processing nearest resize kernel implementation
  *          with 8-bit inputs/outputs
  
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  @verbatim
  @endverbatim
  ******************************************************************************
  */

#ifndef __IPL_NEAREST_PRIVATE__H_
#define __IPL_NEAREST_PRIVATE__H_

#include "arm_math.h"

inline
void IPL_nearest_iu8ou8_memcpy(const uint8_t* in_data_ptr,
                           uint8_t* out_data,
                           const size_t n_channel
                               )
{
  int32_t loopCnt = n_channel;
  while (loopCnt > 0) {
    uint8x16_t u8x16_read;
    
    mve_pred16_t    p = vctp8q(loopCnt);
    
    u8x16_read = vldrbq_u8(in_data_ptr);
    vstrbq_p_u8(out_data, u8x16_read, p);
    out_data+= 16;
    in_data_ptr+= 16;
    loopCnt -= 16;
  }
}
inline
void IPL_nearest_iu8ou8_memcpy_rgb(const uint8_t* in_data_ptr,
                                    uint8_t* out_data)
{
    uint8x16_t u8x16_read;
        
    mve_pred16_t    p = vctp8q(3);
        
    u8x16_read = vldrbq_u8(in_data_ptr);
    vstrbq_p_u8(out_data, u8x16_read, p);
}




#endif // __IPL_NEAREST_PRIVATE__H_
