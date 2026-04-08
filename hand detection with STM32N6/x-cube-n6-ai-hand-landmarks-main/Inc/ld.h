/**
  ******************************************************************************
  * @file    ld.h
  * @author  MDG Application Team
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
  */

#ifndef _HAND_LANDMARK_
#define _HAND_LANDMARK_

/* model hard coded values */
#define LD_WIDTH              224.0f
#define LD_HEIGHT             224.0f
#define LD_LANDMARK_NB        21
#define LD_BINDING_NB         21
/* you can tune this */
#define LD_PROB_THRESHOLD     0.25f

typedef struct {
  float x;
  float y;
} ld_point_t;

extern const int ld_bindings_idx[LD_BINDING_NB][2];

int ld_post_process(float *probs, float *raw_landmarks, ld_point_t landmarks[LD_LANDMARK_NB]);

#endif
