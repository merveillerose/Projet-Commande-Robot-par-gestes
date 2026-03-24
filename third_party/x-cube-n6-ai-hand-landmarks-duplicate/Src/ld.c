/**
  ******************************************************************************
  * @file    ld.c
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

#include "ld.h"

#include <stdio.h>

const int ld_bindings_idx[LD_BINDING_NB][2] = {
  {0,1}, {1,2}, {2,3}, {3,4}, {0,5}, {5,6}, {6,7}, {7,8},
  {5,9}, {9,10}, {10,11}, {11,12}, {9,13}, {13,14}, {14,15}, {15,16},
  {13,17}, {17,18}, {18,19}, {19,20}, {0,17},
};

int ld_post_process(float *probs, float *raw_landmarks, ld_point_t landmarks[LD_LANDMARK_NB])
{
  int i;

  if (*probs < LD_PROB_THRESHOLD)
    return 0;

  for (i = 0; i < LD_LANDMARK_NB; i++) {
    landmarks[i].x = raw_landmarks[i * 3 + 0] / LD_WIDTH;
    landmarks[i].y = raw_landmarks[i * 3 + 1] / LD_WIDTH;
  }

  return 1;
}
