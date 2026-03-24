/*---------------------------------------------------------------------------------------------
#  * Copyright (c) 2023 STMicroelectronics.
#  * All rights reserved.
#  *
#  * This software is licensed under terms that can be found in the LICENSE file in
#  * the root directory of this software component.
#  * If no LICENSE file comes with this software, it is provided AS-IS.
#  *--------------------------------------------------------------------------------------------*/

/* ---------------    Generated code    ----------------- */
#ifndef __POSTPROCESS_CONF_H__
#define __POSTPROCESS_CONF_H__


#ifdef __cplusplus
  extern "C" {
#endif

#include "arm_math.h"

/* Select palm detector postprocess */
#define POSTPROCESS_TYPE POSTPROCESS_MPE_PD_UF

/* I/O configuration */
#define AI_PD_MODEL_PP_WIDTH                      (192)
#define AI_PD_MODEL_PP_HEIGHT                     (192)
#define AI_PD_MODEL_PP_TOTAL_DETECTIONS           (2016)
#define AI_PD_MODEL_PP_NB_KEYPOINTS               (7)

/* --------  Tuning below can be modified by the application --------- */
#define AI_PD_MODEL_PP_CONF_THRESHOLD              (0.5f)
#define AI_PD_MODEL_PP_IOU_THRESHOLD               (0.4f)
#define AI_PD_MODEL_PP_MAX_BOXES_LIMIT             (20)

#endif      /* __POSTPROCESS_CONF_H__  */

