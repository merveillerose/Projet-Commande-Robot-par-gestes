/**
  ******************************************************************************
  * @file    IPL_bilinear_private.h
  * @author  AIS Team
  * @brief   STM32 Image processing biliner resize kernel implementation
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

#ifndef __IPL_BILINEAR_PRIVATE__H_
#define __IPL_BILINEAR_PRIVATE__H_

#include "arm_math.h"


#ifndef MIN
#define MIN(x,y) ((x) < (y) ? (x) : (y))
#endif

#ifndef MAX
#define MAX(x,y) ((x) > (y) ? (x) : (y))
#endif

#define _BILINEAR_COMPUTE_WEIGHTS_Y( Y, weights) \
    float32_t distDown = Y - (uint32_t) Y; \
    weights[0] = (1.0f - (float32_t)distDown); \
    weights[1] = (float32_t)distDown;

#define _BILINEAR_COMPUTE_WEIGHTS_X( X, weights_Y, weights) \
    float32_t distLeft = X - (uint32_t) X; \
    weights[0] = (1.0f - (float32_t)distLeft) * weights_Y[0]; \
    weights[1] = (       (float32_t)distLeft) * weights_Y[0]; \
    weights[2] = (1.0f - (float32_t)distLeft) * weights_Y[1]; \
    weights[3] = (       (float32_t)distLeft) * weights_Y[1]; 

#define _BILINEAR_COMPUTE_WEIGHTS( X, Y, weights) \
    float32_t distLeft = X - (uint32_t) X; \
    float32_t distDown = Y - (uint32_t) Y; \
    weights[0] = (1.0f - (float32_t)distLeft) * (1.0f - (float32_t)distDown); \
    weights[1] = distLeft * (1.0f - (float32_t)distDown); \
    weights[2] = (1.0f - (float32_t)distLeft) * (float32_t)distDown; \
    weights[3] = (float32_t)distLeft * (float32_t)distDown; 




static inline
void IPL_bilinear_iu8ou8_kernel_RGB(const uint8_t* in_data_ptr,
                                    uint8_t* out_data,
                                    size_t inc[2],
                                    float32_t weights[4])
{
    // Can be parallelized over channels
    uint32x4_t  u32x4_read;
    float32x4_t f32x4_read;
    float32x4_t f32x4_tmp;
    uint32x4_t  u32x4_write;
    
    mve_pred16_t p = vctp32q(3);
    
    u32x4_read = vldrbq_u32(in_data_ptr);
    f32x4_read = vcvtq_f32_u32(u32x4_read);
    f32x4_tmp  = vmulq_n_f32(f32x4_read, weights[0]);
    
    u32x4_read = vldrbq_u32(in_data_ptr + inc[0]);
    f32x4_read = vcvtq_f32_u32( u32x4_read);
    f32x4_tmp  = vfmaq_n_f32(f32x4_tmp, f32x4_read, weights[1]);
    
    u32x4_read = vldrbq_u32(in_data_ptr + inc[1]);
    f32x4_read = vcvtq_f32_u32( u32x4_read);
    f32x4_tmp  = vfmaq_n_f32(f32x4_tmp, f32x4_read, weights[2]);
    
    u32x4_read = vldrbq_u32(in_data_ptr + inc[1] + inc[0]);
    f32x4_read = vcvtq_f32_u32( u32x4_read);
    f32x4_tmp  = vfmaq_n_f32(f32x4_tmp, f32x4_read, weights[3]);
    
    u32x4_write = vminq_u32(vcvtaq_u32_f32(f32x4_tmp), vdupq_n_u32(UINT8_MAX));
    
    vstrbq_p_u32(out_data, u32x4_write, p);
}

static inline
void IPL_bilinear_iu8ou8_kernel_RGB_p(const uint8_t* in_data_ptr,
                                      uint8_t* out_data,
                                      size_t inc[2],
                                      float32_t weights[4])
{
    // Can be parallelized over channels
    uint32x4_t  u32x4_read;
    float32x4_t f32x4_read;
    float32x4_t f32x4_tmp;
    uint32x4_t  u32x4_write;
    
    mve_pred16_t p = vctp32q(3);
    
    // Top left
    u32x4_read = vldrbq_z_u32(in_data_ptr, p);
    f32x4_read = vcvtq_f32_u32( u32x4_read);
    f32x4_tmp  = vmulq_n_f32(f32x4_read, weights[0]);
    
    // Top right
    u32x4_read = vldrbq_z_u32(in_data_ptr + inc[0], p);
    f32x4_read = vcvtq_f32_u32( u32x4_read);
    f32x4_tmp  = vfmaq_n_f32(f32x4_tmp, f32x4_read, weights[1]);
    
    // Bottom left
    u32x4_read = vldrbq_z_u32(in_data_ptr + inc[1], p);
    f32x4_read = vcvtq_f32_u32( u32x4_read);
    f32x4_tmp  = vfmaq_n_f32(f32x4_tmp, f32x4_read, weights[2]);
    
    // Bottom right
    u32x4_read = vldrbq_z_u32(in_data_ptr + inc[1] + inc[0], p);
    f32x4_read = vcvtq_f32_u32( u32x4_read);
    f32x4_tmp  = vfmaq_n_f32(f32x4_tmp, f32x4_read, weights[3]);
    
    u32x4_write = vminq_u32(vcvtaq_u32_f32(f32x4_tmp), vdupq_n_u32(UINT8_MAX));
    
    vstrbq_p_u32(out_data, u32x4_write, p);
}

static inline
void IPL_bilinear_iu8ou8_kernel_aRGB(const uint8_t* in_data_ptr,
                                     uint8_t* out_data,
                                     size_t inc[2],
                                     float32_t weights[4])
{
  //mve_pred16_t    p = vctp16q(4); // ie 4 x32bits

  uint32x4_t  u32x4_read;
  float32x4_t f32x4_read;
  float32x4_t f32x4_tmp;
  uint32x4_t  u32x4_write;
  
  u32x4_read = vldrbq_u32(in_data_ptr);
  f32x4_read = vcvtq_f32_u32( u32x4_read);
  f32x4_tmp  = vmulq_n_f32(f32x4_read, weights[0]);
  
  u32x4_read = vldrbq_u32(in_data_ptr + inc[0]);
  f32x4_read = vcvtq_f32_u32( u32x4_read);
  f32x4_tmp  = vfmaq_n_f32(f32x4_tmp, f32x4_read, weights[1]);
  
  u32x4_read = vldrbq_u32(in_data_ptr + inc[1]);
  f32x4_read = vcvtq_f32_u32( u32x4_read);
  f32x4_tmp  = vfmaq_n_f32(f32x4_tmp, f32x4_read, weights[2]);
  
  u32x4_read = vldrbq_u32(in_data_ptr + inc[1] + inc[0]);
  f32x4_read = vcvtq_f32_u32( u32x4_read);
  f32x4_tmp  = vfmaq_n_f32(f32x4_tmp, f32x4_read, weights[3]);
  
  u32x4_write = vminq_u32(vcvtaq_u32_f32(f32x4_tmp), vdupq_n_u32(UINT8_MAX));
  vstrbq_u32(out_data, u32x4_write);
}


/* Default version specifying n_channel */
static inline
void IPL_bilinear_iu8ou8_kernel(const uint8_t* in_data_ptr,
                                uint8_t* out_data,
                                const size_t n_channel,
                                size_t inc[2],
                                float32_t weights[4])
{
    int32_t loopCnt = n_channel;
    while (loopCnt > 0) {
        uint32x4_t u32x4_read;
        float32x4_t f32x4_read;
        float32x4_t f32x4_tmp;
        uint32x4_t u32x4_write;
        
        mve_pred16_t    p = vctp32q(loopCnt);
        
        u32x4_read = vldrbq_z_u32(in_data_ptr, p);
        f32x4_read = vcvtq_f32_u32( u32x4_read);
        f32x4_tmp  = vmulq_n_f32(f32x4_read, weights[0]);
        
        u32x4_read = vldrbq_z_u32(in_data_ptr + inc[0], p);
        f32x4_read = vcvtq_f32_u32( u32x4_read);
        f32x4_tmp  = vfmaq_n_f32(f32x4_tmp, f32x4_read, weights[1]);
        
        u32x4_read = vldrbq_z_u32(in_data_ptr + inc[1], p);
        f32x4_read = vcvtq_f32_u32( u32x4_read);
        f32x4_tmp  = vfmaq_n_f32(f32x4_tmp, f32x4_read, weights[2]);
        
        u32x4_read = vldrbq_z_u32(in_data_ptr + inc[1] + inc[0], p);
        f32x4_read = vcvtq_f32_u32( u32x4_read);
        f32x4_tmp  = vfmaq_n_f32(f32x4_tmp, f32x4_read, weights[3]);
        
        u32x4_write = vminq_u32(vcvtaq_u32_f32(f32x4_tmp), vdupq_n_u32(UINT8_MAX));
        vstrbq_p_u32(out_data, u32x4_write, p);
        out_data+= 4;
        in_data_ptr+= 4;
        loopCnt -= 4;
    }
}


#endif // __IPL_BILINEAR_PRIVATE__H_
