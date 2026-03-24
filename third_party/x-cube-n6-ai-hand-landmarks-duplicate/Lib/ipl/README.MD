# STM32 Image processing Library

This readme explains how to use the different availabe image processing.   
Image format is uint8 RGB.

## Available image processing functions


| function        | Comment                 |
|---------------|----------------------|
| IPL_resize_bilinear_iu8ou8        | Bilinear resize      |
| IPL_resize_bilinear_iu8ou8_RGB        | Bilinear resize  RGB optimized ( 3 channels)     |
| IPL_resize_bilinear_iu8ou8_with_strides_RGB | Bilinear resize RGB optimized ( 3 channels)  specifying strides |
| IPL_resize_nearest_iu8ou8 | Nearest resize |
| IPL_resize_nearest_iu8ou8_RGB | Nearest resize RGB optimized ( 3 channels) |
| IPL_resize_nearest_iu8ou8_with_strides_RGB |  | Nearest resize RGB optimized ( 3 channels) specifying strides |


## Version History

### v0.2.2 - 2024-11-15
- Updated README.MD file with Version History v0.2.2


### v0.2.1 - 2024-11-15
- Updated README.MD file with Version History v0.2.0

### v0.2.0 - 2024-11-15
- **Bug Fixes:**
  - Added LICENCE.md file
  - Corrected compilation warnings


### v0.1.0 - 2024-10-25

- **Initial Release:**
  - IPL_resize_bilinear_iu8ou8: resize bilinear uint8 input/output with n channels
  - IPL_resize_bilinear_iu8ou8: resize bilinear uint8 input/output with 3 channels
  - IPL_resize_bilinear_iu8ou8: resize bilinear uint8 input/output with 3 channels and strides
  - IPL_resize_neareset_iu8ou8: resize nearest uint8 input/output with n channels
  - IPL_resize_nearest_iu8ou8: resize nearest uint8 input/output with 3 channels
  - IPL_resize_nearest_iu8ou8: resize nearest uint8 input/output with 3 channels and strides


bilinear resize 

  resized = a * (1 - x_weight) * (1 - y_weight) + \
            b * x_weight * (1 - y_weight) + \
            c * y_weight * (1 - x_weight) + \
            d * x_weight * y_weight
            
            
Nearest resize 
  
  Simply copy the nearest pixel from the original image
            
```C
/* Include header file */
#include "IPL_resize.h"

/* bilinear resize */
/* Input image 45x129 3 channels */  
uint8_t ImgIn[45][129][3];
uint8_t ImgOut[256][256][3];
IPL_resize_bilinear_iu8ou8( (uint8_t *)ImgIn,   // pointer to image in 
                            (uint8_t *)ImgOut,  // pointer to image out
                            (size_t)45,         // width_in
                            (size_t)129,        // height_in
                            (size_t)3,          // nb channels in/out
                            (size_t)256,        // width_out
                            (size_t)256);       // height_out
                            
IPL_resize_bilinear_iu8ou8_RGB( (uint8_t *)ImgIn,   // pointer to image in 
                                (uint8_t *)ImgOut,  // pointer to image out
                                (size_t)45,         // width_in
                                (size_t)129,        // height_in
                                (size_t)256,        // width_out
                                (size_t)256);       // height_out
                            
// Writing the whole ImgIn image in part of the image out: a 64x64 area from pos y=48, x=100
IPL_resize_bilinear_iu8ou8_with_strides_RGB( (uint8_t *)ImgIn,                        // pointer to image in 
                                             (uint8_t *)ImgOut + (48 * 256 + 100)*3,  // pointer to image out
                                             {size_t)3*45                        // stride_in
                                             {size_t)3*256                       // stride_out
                                             (size_t)45,                         // width_in
                                             (size_t)129,                        // height_in
                                             (size_t)64,                         // width_out
                                             (size_t)64);                        // height_out

/*  nearest resize */


IPL_resize_nearest_iu8ou8( (uint8_t *)ImgIn,    // pointer to image in 
                           (uint8_t *)ImgOut,  // pointer to image out
                           (size_t)45,         // width_in
                           (size_t)129,        // height_in
                           (size_t)3,          // nb channels in/out
                           (size_t)256,        // width_out
                           (size_t)256);       // height_out
                            
IPL_resize_neareset_iu8ou8_RGB( (uint8_t *)ImgIn,   // pointer to image in 
                                (uint8_t *)ImgOut,  // pointer to image out
                                (size_t)45,         // width_in
                                (size_t)129,        // height_in
                                (size_t)256,        // width_out
                                (size_t)256);       // height_out
                            
IPL_resize_neareset_iu8ou8_with_strides_RGB( (uint8_t *)ImgIn,                        // pointer to image in 
                                             (uint8_t *)ImgOut,  // pointer to image out
                                             {size_t)3*45                        // stride_in
                                             {size_t)3*256                       // stride_out
                                             (size_t)45,                         // width_in
                                             (size_t)129,                        // height_in
                                             (size_t)256,                         // width_out
                                             (size_t)256);                        // height_out
```
