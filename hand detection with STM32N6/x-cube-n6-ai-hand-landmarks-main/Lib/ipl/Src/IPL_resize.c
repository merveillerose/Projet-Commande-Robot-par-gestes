/**
******************************************************************************
* @file    bilinear_kernel.c
* @author  AIS Team
* @brief   ST Lite upsample functions with 8-bit inputs/outputs

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

#include <arm_math.h>
#include "IPL_resize.h"
#include "IPL_resize_bilinear_private.h"
#include "IPL_resize_nearest_private.h"


void IPL_resize_bilinear_iu8ou8(const uint8_t* in_data,
                                uint8_t* out_data,
                                const size_t width_in,
                                const size_t height_in,
                                const size_t n_channel,
                                const size_t width_out,
                                const size_t height_out)
{
    float32_t inv_width_scale, inv_height_scale;
    inv_width_scale = ((float32_t)width_in) / ((float32_t)width_out);
    inv_height_scale = ((float32_t)height_in) / ((float32_t)height_out);
    
    for (size_t h = 0; h < height_out; h++)
    {
        float32_t Y = MIN(MAX(-0.5f + inv_height_scale*(h + 0.5f),0.0f),(float32_t)height_in-1);
        const uintptr_t y_step = (Y == (height_in - 1)) ? 0 : width_in * n_channel;
        float32_t weights_Y[2];
        _BILINEAR_COMPUTE_WEIGHTS_Y( Y, weights_Y)
            
            for (size_t w = 0; w < width_out; w++)
            {
                float32_t X = MIN(MAX(-0.5f + inv_width_scale*(w + 0.5f),0.0f),(float32_t)width_in-1);
                const uintptr_t x_step = (X == (width_in - 1)) ? 0 : n_channel;
                uint32_t offset = ((uint32_t) X + (uint32_t) Y * width_in) * n_channel;
                uint32_t offset_out = w*n_channel + h*width_out*n_channel;
                
                const uint8_t * in_data_ptr = in_data + offset;
                uint8_t * out_data_ptr = out_data + offset_out;
                
                size_t inc[2] = { x_step, y_step};
                float32_t weights[4];
                _BILINEAR_COMPUTE_WEIGHTS_X( X, weights_Y, weights)
                IPL_bilinear_iu8ou8_kernel(in_data_ptr, out_data_ptr, n_channel, inc, weights);
                
            }
    }
}


void IPL_resize_bilinear_iu8ou8_RGB( const uint8_t* in_data,
                                     uint8_t* out_data,
                                     const size_t width_in,
                                     const size_t height_in,
                                     const size_t width_out,
                                     const size_t height_out)
{
    IPL_resize_bilinear_iu8ou8_with_strides_RGB(in_data,
                                                out_data,
                                                width_in*3,
                                                width_out*3,
                                                width_in,
                                                height_in,
                                                width_out,
                                                height_out);
}


void IPL_resize_bilinear_iu8ou8_with_strides_RGB(const uint8_t* in_data,
                                             uint8_t* out_data,
                                             const size_t stride_in,
                                             const size_t stride_out,
                                             const size_t width_in,
                                             const size_t height_in,
                                             const size_t width_out,
                                             const size_t height_out)
{ 
    float32_t inv_width_scale, inv_height_scale;
    inv_width_scale = ((float32_t)width_in) / ((float32_t) width_out);
    inv_height_scale = ((float32_t)height_in) / ((float32_t)height_out);

    uint8_t * out_data_current_ptr = out_data;
    uint8_t * out_data_ptr;

    for (size_t h = 0; h < height_out; h++)
    {
        float32_t Y = MIN(MAX(-0.5f + inv_height_scale*(h + 0.5f),0.0f),(float32_t)height_in-1);
        uint32_t Yi = (uint32_t)Y;
        const uintptr_t y_step = (Yi == (height_in - 1)) ? 0 : stride_in;
        uint32_t offset_y = Yi * stride_in;
        float32_t weights_Y[2];
        _BILINEAR_COMPUTE_WEIGHTS_Y( Y, weights_Y);

        out_data_ptr = out_data_current_ptr;
        for (size_t w = 0; w < width_out; w++)
        {
            float32_t X = MIN(MAX(-0.5f + inv_width_scale*(w + 0.5f),0.0f),(float32_t)width_in-1);
            uint32_t Xi = (uint32_t)X;
            const uintptr_t x_step = (Xi == (width_in - 1)) ? 0 : 3;
            uint32_t offset = offset_y + Xi * 3;
            const uint8_t * in_data_ptr = in_data + offset;
            
            size_t inc[2] = { x_step, y_step};
            float32_t weights[4];
            _BILINEAR_COMPUTE_WEIGHTS_X( X, weights_Y, weights);
            IPL_bilinear_iu8ou8_kernel_RGB(in_data_ptr, out_data_ptr, inc, weights);
            out_data_ptr +=3;
        }
        out_data_current_ptr += stride_out;
    }  
}


void IPL_resize_nearest_iu8ou8_RGB( const uint8_t* in_data,
                                    uint8_t* out_data,
                                    const size_t width_in,
                                    const size_t height_in,
                                    const size_t width_out,
                                    const size_t height_out)
{
  IPL_resize_nearest_iu8ou8_with_strides_RGB(in_data,
                                             out_data,
                                             3 * width_in,
                                             3 * width_out,
                                             width_in,
                                             height_in,
                                             width_out,
                                             height_out);
}

void IPL_resize_nearest_iu8ou8_with_strides_RGB( const uint8_t* in_data,
                                                 uint8_t* out_data,
                                                 const size_t stride_in,
                                                 const size_t stride_out,
                                                 const size_t width_in,
                                                 const size_t height_in,
                                                 const size_t width_out,
                                                 const size_t height_out)
{
    float32_t inv_width_scale, inv_height_scale;
    inv_width_scale = ((float32_t)width_in) / ((float32_t) width_out);
    inv_height_scale = ((float32_t)height_in) / ((float32_t)height_out);
    
    uint8_t *out_data_current_ptr = out_data;
    uint8_t *out_data_ptr;
    
    for (size_t h = 0; h < height_out; h++)
    {
        float32_t iy_fl = -0.5 + inv_height_scale*(h + 0.5f);
        int32_t Y = (int32_t) (MIN(MAX(iy_fl, 0.0f),(float32_t)height_in-1) + 0.5);
        int32_t offset_y = Y * stride_in;
        
        out_data_ptr = out_data_current_ptr;
        for (size_t w = 0; w < width_out; w++)
        {
            float32_t ix_fl = -0.5 + inv_width_scale*(w + 0.5f);
            int32_t X = (int32_t) (MIN(MAX(ix_fl, 0.0f),(float32_t)width_in-1) + 0.5);
            
            int32_t offset =   X * 3 + offset_y;
            
            const uint8_t *in_data_ptr = in_data + offset ;
            
            out_data_ptr[0] = in_data_ptr[0];
            out_data_ptr[1] = in_data_ptr[1];
            out_data_ptr[2] = in_data_ptr[2];
            out_data_ptr+=3;
        }
        out_data_current_ptr += stride_out;
    }
}


void IPL_resize_nearest_iu8ou8( const uint8_t* in_data,
                                uint8_t* out_data,
                                const size_t width_in,
                                const size_t height_in,
                                const size_t n_channel,
                                const size_t width_out,
                                const size_t height_out)
{
    float32_t inv_width_scale, inv_height_scale;
    inv_width_scale = ((float32_t)width_in) / ((float32_t) width_out);
    inv_height_scale = ((float32_t)height_in) / ((float32_t)height_out);
    
    uint8_t * out_data_ptr = out_data;
    
    for (size_t h = 0; h < height_out; h++)
    {
        float32_t iy_fl = -0.5 + inv_height_scale*(h + 0.5f);
        int32_t Y = (int32_t) (MIN(MAX(iy_fl, 0.0f),(float32_t)height_in-1) + 0.5);
        int32_t offset_y = Y * width_in * n_channel;
        
        for (size_t w = 0; w < width_out; w++)
        {
            float32_t ix_fl = -0.5 + inv_width_scale*(w + 0.5f);
            int32_t X = (int32_t) (MIN(MAX(ix_fl, 0.0f),(float32_t)width_in-1) + 0.5);
            
            int32_t offset =   X * n_channel + offset_y;
            
            
            IPL_nearest_iu8ou8_memcpy((const uint8_t *) (in_data + offset), out_data_ptr, n_channel);
            out_data_ptr += n_channel;
        }
    }
}

