/**
  ******************************************************************************
  * @file    IPL_resize.h
  * @author  AIS Team
  * @brief   STM32 Image processing library functions with 8-bit inputs/outputs

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

#ifndef __IPL_RESIZE__
#define __IPL_RESIZE__

#include "arm_math.h"

/*!
 * @brief Bilinear resize from u8 buffer HxWxN
 *
 * @param [IN]  in_data: Pointer on input data
 * @param [OUT] out_ata: Pointer on output data
 * @param [IN]  width_in: input width 
 * @param [IN]  height_in: input height
 * @param [IN]  n_channel: input and output nb channels (ie 3 for RGB image)
 * @param [IN]  width_out: output width
 * @param [IN]  height_out: output height
 *             
 * @retval None
 */
void IPL_resize_bilinear_iu8ou8( const uint8_t* in_data,
                             uint8_t* out_data,
                             const size_t width_in,
                             const size_t height_in,
                             const size_t n_channel,
                             const size_t width_out,
                             const size_t height_out);

/*!
 * @brief Bilinear resize from RGB image (uint8)
 *
 * @param [IN]  in_data: Pointer on input data
 * @param [OUT] out_ata: Pointer on output data
 * @param [IN]  width_in: input width 
 * @param [IN]  height_in: input height
 * @param [IN]  width_out: output width
 * @param [IN]  height_out: output height
 *             
 * @retval None
 */
void IPL_resize_bilinear_iu8ou8_RGB( const uint8_t* in_data,
                             uint8_t* out_data,
                             const size_t width_in,
                             const size_t height_in,
                             const size_t width_out,
                             const size_t height_out);

/*!     
 * @brief Bilinear resize from RGB image (uint8) specifying strides
 *
 * @param [IN]  in_data: Pointer on input data
 * @param [OUT] out_ata: Pointer on output data
 * @param [IN]  stride_in: input stride for next line 
 * @param [IN]  stride_out: output stride for next line
 * @param [IN]  width_in: input width 
 * @param [IN]  height_in: input height
 * @param [IN]  width_out: output width
 * @param [IN]  height_out: output height
 *             
 * @retval None
 */
void IPL_resize_bilinear_iu8ou8_with_strides_RGB( const uint8_t* in_data,
                                              uint8_t* out_data,
                                              const size_t stride_in,
                                              const size_t stride_out,
                                              const size_t width_in,
                                              const size_t height_in,
                                              const size_t width_out,
                                              const size_t height_out);

/*!
 * @brief Nearest resize from u8 buffer HxWxN
 *
 * @param [IN]  in_data: Pointer on input data
 * @param [OUT] out_ata: Pointer on output data
 * @param [IN]  width_in: input width 
 * @param [IN]  height_in: input height
 * @param [IN]  n_channel: input and output nb channels (ie 3 for RGB image)
 * @param [IN]  width_out: output width
 * @param [IN]  height_out: output height
 *             
 * @retval None
 */
void IPL_resize_nearest_iu8ou8(const uint8_t* in_data,
                             uint8_t* out_data,
                             const size_t width_in,
                             const size_t height_in,
                             const size_t n_channel,
                             const size_t width_out,
                             const size_t height_out);
/*!
 * @brief Nearest resize from RGB image (uint8)
 *
 * @param [IN]  in_data: Pointer on input data
 * @param [OUT] out_ata: Pointer on output data
 * @param [IN]  width_in: input width 
 * @param [IN]  height_in: input height
 * @param [IN]  width_out: output width
 * @param [IN]  height_out: output height
 *             
 * @retval None
 */
void IPL_resize_nearest_iu8ou8_RGB(const uint8_t* in_data,
                             uint8_t* out_data,
                             const size_t width_in,
                             const size_t height_in,
                             const size_t width_out,
                             const size_t height_out);

/*!
 * @brief Nearest resize from RGB image (uint8) specifying strides
 *
 * @param [IN]  in_data: Pointer on input data
 * @param [OUT] out_ata: Pointer on output data
 * @param [IN]  stride_in: input stride for next line 
 * @param [IN]  stride_out: output stride for next line
 * @param [IN]  width_in: input width 
 * @param [IN]  height_in: input height
 * @param [IN]  width_out: output width
 * @param [IN]  height_out: output height
 *             
 * @retval None
 */
void IPL_resize_nearest_iu8ou8_with_strides_RGB( const uint8_t* in_data,
                                                 uint8_t* out_data,
                                                 const size_t stride_in,
                                                 const size_t stride_out,
                                                 const size_t width_in,
                                                 const size_t height_in,
                                                 const size_t width_out,
                                                 const size_t height_out);


#endif // __IPL_RESIZE__
