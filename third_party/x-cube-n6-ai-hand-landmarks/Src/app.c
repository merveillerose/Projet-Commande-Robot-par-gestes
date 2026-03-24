/**
  ******************************************************************************
  * @file    app.c
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

#include "app.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "app_cam.h"
#include "app_config.h"
#include "IPL_resize.h"
#include "app_postprocess.h"
#include "isp_api.h"
#include "ld.h"
#include "ll_aton_runtime.h"
#include "cmw_camera.h"
#include "scrl.h"
#ifdef STM32N6570_DK_REV
#include "stm32n6570_discovery.h"
#else
#include "stm32n6xx_nucleo.h"
#endif
#include "stm32_lcd.h"
#include "stm32_lcd_ex.h"
#include "stm32n6xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "utils.h"
#include "stm32n6xx_hal_uart.h"

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;

typedef enum {
  G_NONE,
  G_STOP,
  G_BACKWARD,
  G_FORWARD,
  G_LEFT,
  G_RIGHT
} gesture_t;
int lock_state=1;
static int hand_recognition = 0;
static int stop_count = 0;
void send_gesture_command(gesture_t gesture);

#define FREERTOS_PRIORITY(p) ((UBaseType_t)((int)tskIDLE_PRIORITY + configMAX_PRIORITIES / 2 + (p)))

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#if HAS_ROTATION_SUPPORT == 1
#include "nema_core.h"
#include "nema_error.h"
void nema_enable_tiling(int);
#endif

#define LCD_FG_WIDTH LCD_BG_WIDTH
#define LCD_FG_HEIGHT LCD_BG_HEIGHT

#define CACHE_OP(__op__) do { \
  if (is_cache_enable()) { \
    __op__; \
  } \
} while (0)

#define DBG_INFO 0
#define USE_FILTERED_TS 1

#define BQUEUE_MAX_BUFFERS 2
#define CPU_LOAD_HISTORY_DEPTH 8

#define DISPLAY_BUFFER_NB (DISPLAY_DELAY + 2)

/* palm detector */
#define PD_MAX_HAND_NB 1

#if HAS_ROTATION_SUPPORT == 1
typedef float app_v3_t[3];
#endif

typedef struct {
  float cx;
  float cy;
  float w;
  float h;
  float rotation;
} roi_t;

#define UTIL_LCD_COLOR_TRANSPARENT 0

#ifdef STM32N6570_DK_REV
#define LCD_FONT Font20
#define DISK_RADIUS 2
#else
#define LCD_FONT Font12
#define DISK_RADIUS 1
#endif

typedef struct
{
  uint32_t X0;
  uint32_t Y0;
  uint32_t XSize;
  uint32_t YSize;
} Rectangle_TypeDef;

typedef struct {
  SemaphoreHandle_t free;
  StaticSemaphore_t free_buffer;
  SemaphoreHandle_t ready;
  StaticSemaphore_t ready_buffer;
  int buffer_nb;
  uint8_t *buffers[BQUEUE_MAX_BUFFERS];
  int free_idx;
  int ready_idx;
} bqueue_t;

typedef struct {
  uint64_t current_total;
  uint64_t current_thread_total;
  uint64_t prev_total;
  uint64_t prev_thread_total;
  struct {
    uint64_t total;
    uint64_t thread;
    uint32_t tick;
  } history[CPU_LOAD_HISTORY_DEPTH];
} cpuload_info_t;

typedef struct {
  int is_valid;
  pd_pp_box_t pd_hands;
  roi_t roi;
  ld_point_t ld_landmarks[LD_LANDMARK_NB];
} hand_info_t;

typedef struct {
  float nn_period_ms;
  uint32_t pd_ms;
  uint32_t hl_ms;
  uint32_t pp_ms;
  uint32_t disp_ms;
  int is_ld_displayed;
  int is_pd_displayed;
  int pd_hand_nb;
  float pd_max_prob;
  hand_info_t hands[PD_MAX_HAND_NB];
} display_info_t;

typedef struct {
  SemaphoreHandle_t update;
  StaticSemaphore_t update_buffer;
  SemaphoreHandle_t lock;
  StaticSemaphore_t lock_buffer;
  display_info_t info;
} display_t;

typedef struct {
  uint32_t nn_in_len;
  float *prob_out;
  uint32_t prob_out_len;
  float *boxes_out;
  uint32_t boxes_out_len;
  pd_model_pp_static_param_t static_param;
  pd_pp_out_t pd_out;
} pd_model_info_t;

typedef struct {
  uint8_t *nn_in;
  uint32_t nn_in_len;
  float *prob_out;
  uint32_t prob_out_len;
  float *landmarks_out;
  uint32_t landmarks_out_len;
} hl_model_info_t;

typedef struct {
  Button_TypeDef button_id;
  int prev_state;
  void (*on_click_handler)(void *cb_args);
  void *cb_args;
} button_t;

/* Globals */
/* Lcd Background area */
static Rectangle_TypeDef lcd_bg_area = {
  .X0 = 0,
  .Y0 = 0,
  .XSize = LCD_BG_WIDTH,
  .YSize = LCD_BG_HEIGHT,
};
/* Lcd Foreground area */
static Rectangle_TypeDef lcd_fg_area = {
  .X0 = 0,
  .Y0 = 0,
  .XSize = LCD_FG_WIDTH,
  .YSize = LCD_FG_HEIGHT,
};
/* Lcd Background Buffer */
static uint8_t lcd_bg_buffer[DISPLAY_BUFFER_NB][LCD_BG_WIDTH * LCD_BG_HEIGHT * DISPLAY_BPP] ALIGN_32 IN_PSRAM;
static int lcd_bg_buffer_disp_idx = 1;
static int lcd_bg_buffer_capt_idx = 0;
/* Lcd Foreground Buffer */
static uint8_t lcd_fg_buffer[2][LCD_FG_WIDTH * LCD_FG_HEIGHT* 2] ALIGN_32 IN_PSRAM;
static int lcd_fg_buffer_rd_idx;
static display_t disp = {
  .info.is_ld_displayed = 1,
  .info.is_pd_displayed = 0,
};
static cpuload_info_t cpu_load;
/* screen buffer */
static uint8_t screen_buffer[LCD_BG_WIDTH * LCD_BG_HEIGHT * 2] ALIGN_32 IN_PSRAM;

/* model */
 /* palm detector */
LL_ATON_DECLARE_NAMED_NN_INSTANCE_AND_INTERFACE(palm_detector);
static roi_t rois[PD_MAX_HAND_NB];
 /* hand landmark */
LL_ATON_DECLARE_NAMED_NN_INSTANCE_AND_INTERFACE(hand_landmark);
static ld_point_t ld_landmarks[PD_MAX_HAND_NB][LD_LANDMARK_NB];
static uint32_t frame_event_nb;
static volatile uint32_t frame_event_nb_for_resize;

 /* nn input buffers */
static uint8_t nn_input_buffers[2][NN_WIDTH * NN_HEIGHT * NN_BPP] ALIGN_32 IN_PSRAM;
static bqueue_t nn_input_queue;

 /* rtos */
static StaticTask_t nn_thread;
static StackType_t nn_thread_stack[2 * configMINIMAL_STACK_SIZE];
static StaticTask_t dp_thread;
static StackType_t dp_thread_stack[2 *configMINIMAL_STACK_SIZE];
static StaticTask_t isp_thread;
static StackType_t isp_thread_stack[2 *configMINIMAL_STACK_SIZE];
static SemaphoreHandle_t isp_sem;
static StaticSemaphore_t isp_sem_buffer;

#if HAS_ROTATION_SUPPORT == 1
static GFXMMU_HandleTypeDef hgfxmmu;
static nema_cmdlist_t cl;
#endif

static int is_cache_enable()
{
#if defined(USE_DCACHE)
  return 1;
#else
  return 0;
#endif
}

static float pd_normalize_angle(float angle)
{
  return angle - 2 * M_PI * floorf((angle - (-M_PI)) / (2 * M_PI));
}

/* Without rotation support allow limited amount of angles */
#if HAS_ROTATION_SUPPORT == 0
static float pd_cook_rotation(float angle)
{
  if (angle >= (3 * M_PI) / 4)
    angle = M_PI;
  else if (angle >= (1 * M_PI) / 4)
    angle = M_PI / 2;
  else if (angle >= -(1 * M_PI) / 4)
    angle = 0;
  else if (angle >= -(3 * M_PI) / 4)
    angle = -M_PI / 2;
  else
    angle = -M_PI;

  return angle;
}
#else
static float pd_cook_rotation(float angle)
{
  return angle;
}
#endif

static float pd_compute_rotation(pd_pp_box_t *box)
{
  float x0, y0, x1, y1;
  float rotation;

  x0 = box->pKps[0].x;
  y0 = box->pKps[0].y;
  x1 = box->pKps[2].x;
  y1 = box->pKps[2].y;

  rotation = M_PI * 0.5 - atan2f(-(y1 - y0), x1 - x0);

  return pd_cook_rotation(pd_normalize_angle(rotation));
}

static void cvt_pd_coord_to_screen_coord(pd_pp_box_t *box)
{
  int i;

  /* This is not a typo. Since screen aspect ratio was conserved. We really want to use LCD_BG_WIDTH for
   * y positions.
   */

  box->x_center *= LCD_BG_WIDTH;
  box->y_center *= LCD_BG_WIDTH;
  box->width *= LCD_BG_WIDTH;
  box->height *= LCD_BG_WIDTH;
  for (i = 0; i < AI_PD_MODEL_PP_NB_KEYPOINTS; i++) {
    box->pKps[i].x *= LCD_BG_WIDTH;
    box->pKps[i].y *= LCD_BG_WIDTH;
  }
}

static void roi_shift_and_scale(roi_t *roi, float shift_x, float shift_y, float scale_x, float scale_y)
{
  float long_side;
  float sx, sy;

  sx = (roi->w * shift_x * cos(roi->rotation) - roi->h * shift_y * sin(roi->rotation));
  sy = (roi->w * shift_x * sin(roi->rotation) + roi->h * shift_y * cos(roi->rotation));

  roi->cx += sx;
  roi->cy += sy;

  long_side = MAX(roi->w, roi->h);
  roi->w = long_side;
  roi->h = long_side;

  roi->w *= scale_x;
  roi->h *= scale_y;
}

static void pd_box_to_roi(pd_pp_box_t *box,  roi_t *roi)
{
  const float shift_x = 0;
  const float shift_y = -0.5;
  const float scale = 2.6;

  roi->cx = box->x_center;
  roi->cy = box->y_center;
  roi->w = box->width;
  roi->h = box->height;
  roi->rotation = pd_compute_rotation(box);

  roi_shift_and_scale(roi, shift_x, shift_y, scale, scale);

#if HAS_ROTATION_SUPPORT == 0
  /* In that case we can cancel rotation. This ensure corners are corrected oriented */
  roi->rotation = 0;
#endif
}

static void copy_pd_box(pd_pp_box_t *dst, pd_pp_box_t *src)
{
  int i;

  dst->prob = src->prob;
  dst->x_center = src->x_center;
  dst->y_center = src->y_center;
  dst->width = src->width;
  dst->height = src->height;
  for (i = 0 ; i < AI_PD_MODEL_PP_NB_KEYPOINTS; i++)
    dst->pKps[i] = src->pKps[i];
}

static void button_init(button_t *b, Button_TypeDef id, void (*on_click_handler)(void *), void *cb_args)
{
  int ret;

  ret = BSP_PB_Init(id, BUTTON_MODE_GPIO);
  assert(ret == BSP_ERROR_NONE);

  b->button_id = id;
  b->on_click_handler = on_click_handler;
  b->prev_state = 0;
  b->cb_args = cb_args;
}

static void button_process(button_t *b)
{
  int state = BSP_PB_GetState(b->button_id);

  if (state != b->prev_state && state && b->on_click_handler)
    b->on_click_handler(b->cb_args);

  b->prev_state = state;
}

static void cpuload_init(cpuload_info_t *cpu_load)
{
  memset(cpu_load, 0, sizeof(cpuload_info_t));
}

static void cpuload_update(cpuload_info_t *cpu_load)
{
  int i;

  cpu_load->history[1] = cpu_load->history[0];
  cpu_load->history[0].total = portGET_RUN_TIME_COUNTER_VALUE();
  cpu_load->history[0].thread = cpu_load->history[0].total - ulTaskGetIdleRunTimeCounter();
  cpu_load->history[0].tick = HAL_GetTick();

  if (cpu_load->history[1].tick - cpu_load->history[2].tick < 1000)
    return ;

  for (i = 0; i < CPU_LOAD_HISTORY_DEPTH - 2; i++)
    cpu_load->history[CPU_LOAD_HISTORY_DEPTH - 1 - i] = cpu_load->history[CPU_LOAD_HISTORY_DEPTH - 1 - i - 1];
}

static void cpuload_get_info(cpuload_info_t *cpu_load, float *cpu_load_last, float *cpu_load_last_second,
                             float *cpu_load_last_five_seconds)
{
  if (cpu_load_last)
    *cpu_load_last = 100.0 * (cpu_load->history[0].thread - cpu_load->history[1].thread) /
                     (cpu_load->history[0].total - cpu_load->history[1].total);
  if (cpu_load_last_second)
    *cpu_load_last_second = 100.0 * (cpu_load->history[2].thread - cpu_load->history[3].thread) /
                     (cpu_load->history[2].total - cpu_load->history[3].total);
  if (cpu_load_last_five_seconds)
    *cpu_load_last_five_seconds = 100.0 * (cpu_load->history[2].thread - cpu_load->history[7].thread) /
                     (cpu_load->history[2].total - cpu_load->history[7].total);
}

static int bqueue_init(bqueue_t *bq, int buffer_nb, uint8_t **buffers)
{
  int i;

  if (buffer_nb > BQUEUE_MAX_BUFFERS)
    return -1;

  bq->free = xSemaphoreCreateCountingStatic(buffer_nb, buffer_nb, &bq->free_buffer);
  if (!bq->free)
    goto free_sem_error;
  bq->ready = xSemaphoreCreateCountingStatic(buffer_nb, 0, &bq->ready_buffer);
  if (!bq->ready)
    goto ready_sem_error;

  bq->buffer_nb = buffer_nb;
  for (i = 0; i < buffer_nb; i++) {
    assert(buffers[i]);
    bq->buffers[i] = buffers[i];
  }
  bq->free_idx = 0;
  bq->ready_idx = 0;

  return 0;

ready_sem_error:
  vSemaphoreDelete(bq->free);
free_sem_error:
  return -1;
}

static uint8_t *bqueue_get_free(bqueue_t *bq, int is_blocking)
{
  uint8_t *res;
  int ret;

  ret = xSemaphoreTake(bq->free, is_blocking ? portMAX_DELAY : 0);
  if (ret == pdFALSE)
    return NULL;

  res = bq->buffers[bq->free_idx];
  bq->free_idx = (bq->free_idx + 1) % bq->buffer_nb;

  return res;
}

static void bqueue_put_free(bqueue_t *bq)
{
  int ret;

  ret = xSemaphoreGive(bq->free);
  assert(ret == pdTRUE);
}

static uint8_t *bqueue_get_ready(bqueue_t *bq)
{
  uint8_t *res;
  int ret;

  ret = xSemaphoreTake(bq->ready, portMAX_DELAY);
  assert(ret == pdTRUE);

  res = bq->buffers[bq->ready_idx];
  bq->ready_idx = (bq->ready_idx + 1) % bq->buffer_nb;

  return res;
}

static void bqueue_put_ready(bqueue_t *bq)
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  int ret;

  if (xPortIsInsideInterrupt()) {
    ret = xSemaphoreGiveFromISR(bq->ready, &xHigherPriorityTaskWoken);
    assert(ret == pdTRUE);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  } else {
    ret = xSemaphoreGive(bq->ready);
    assert(ret == pdTRUE);
  }
}

static void reload_bg_layer(int next_disp_idx)
{
  int ret;

  ret = SCRL_SetAddress_NoReload(lcd_bg_buffer[next_disp_idx], SCRL_LAYER_0);
  assert(ret == 0);
  ret = SCRL_ReloadLayer(SCRL_LAYER_0);
  assert(ret == 0);

  ret = SRCL_Update();
  assert(ret == 0);
}

static void app_main_pipe_frame_event()
{
  int next_disp_idx = (lcd_bg_buffer_disp_idx + 1) % DISPLAY_BUFFER_NB;
  int next_capt_idx = (lcd_bg_buffer_capt_idx + 1) % DISPLAY_BUFFER_NB;
  int ret;

  ret = HAL_DCMIPP_PIPE_SetMemoryAddress(CMW_CAMERA_GetDCMIPPHandle(), DCMIPP_PIPE1,
                                         DCMIPP_MEMORY_ADDRESS_0, (uint32_t) lcd_bg_buffer[next_capt_idx]);
  assert(ret == HAL_OK);

  reload_bg_layer(next_disp_idx);
  lcd_bg_buffer_disp_idx = next_disp_idx;
  lcd_bg_buffer_capt_idx = next_capt_idx;

  frame_event_nb++;
}


static void app_ancillary_pipe_frame_event()
{
  uint8_t *next_buffer;
  int ret;

  next_buffer = bqueue_get_free(&nn_input_queue, 0);
  if (next_buffer) {
    ret = HAL_DCMIPP_PIPE_SetMemoryAddress(CMW_CAMERA_GetDCMIPPHandle(), DCMIPP_PIPE2,
                                           DCMIPP_MEMORY_ADDRESS_0, (uint32_t) next_buffer);
    assert(ret == HAL_OK);
    /* minus 1 since app_main_pipe_frame_event occur before app_ancillary_pipe_frame_event() */
    frame_event_nb_for_resize = frame_event_nb - 1;
    bqueue_put_ready(&nn_input_queue);
  }
}

static void app_main_pipe_vsync_event()
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  int ret;

  ret = xSemaphoreGiveFromISR(isp_sem, &xHigherPriorityTaskWoken);
  if (ret == pdTRUE)
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

static int clamp_point(int *x, int *y)
{
  int xi = *x;
  int yi = *y;

  if (*x < 0)
    *x = 0;
  if (*y < 0)
    *y = 0;
  if (*x >= lcd_bg_area.XSize)
    *x = lcd_bg_area.XSize - 1;
  if (*y >= lcd_bg_area.YSize)
    *y = lcd_bg_area.YSize - 1;

  return (xi != *x) || (yi != *y);
}

static int clamp_point_with_margin(int *x, int *y, int margin)
{
  int xi = *x;
  int yi = *y;

  if (*x < margin)
    *x = margin;
  if (*y < margin)
    *y = margin;
  if (*x >= lcd_bg_area.XSize - margin)
    *x = lcd_bg_area.XSize - margin - 1;
  if (*y >= lcd_bg_area.YSize - margin)
    *y = lcd_bg_area.YSize - margin - 1;

  return (xi != *x) || (yi != *y);
}

static void display_pd_hand(pd_pp_box_t *hand)
{
  int xc, yc;
  int x0, y0;
  int x1, y1;
  int w, h;
  int i;

  /* display box around palm */
  xc = (int)hand->x_center;
  yc = (int)hand->y_center;
  w = (int)hand->width;
  h = (int)hand->height;
  x0 = xc - (w + 1) / 2;
  y0 = yc - (h + 1) / 2;
  x1 = xc + (w + 1) / 2;
  y1 = yc + (h + 1) / 2;
  clamp_point(&x0, &y0);
  clamp_point(&x1, &y1);
  UTIL_LCD_DrawRect(x0, y0, x1 - x0, y1 - y0, UTIL_LCD_COLOR_GREEN);

  /* display palm key points */
  for (i = 0; i < 7; i++) {
    uint32_t color = (i != 0 && i != 2) ? UTIL_LCD_COLOR_RED : UTIL_LCD_COLOR_BLUE;

    x0 = (int)hand->pKps[i].x;
    y0 = (int)hand->pKps[i].y;
    clamp_point(&x0, &y0);
    UTIL_LCD_FillCircle(x0, y0, 2, color);
  }
}

static void rotate_point(float pt[2], float rotation)
{
  float x = pt[0];
  float y = pt[1];

  pt[0] = cos(rotation) * x - sin(rotation) * y;
  pt[1] = sin(rotation) * x + cos(rotation) * y;
}

static void roi_to_corners(roi_t *roi, float corners[4][2])
{
  const float corners_init[4][2] = {
    {-roi->w / 2, -roi->h / 2},
    { roi->w / 2, -roi->h / 2},
    { roi->w / 2,  roi->h / 2},
    {-roi->w / 2,  roi->h / 2},
  };
  int i;

  memcpy(corners, corners_init, sizeof(corners_init));
  /* rotate */
  for (i = 0; i < 4; i++)
    rotate_point(corners[i], roi->rotation);

  /* shift */
  for (i = 0; i < 4; i++) {
    corners[i][0] += roi->cx;
    corners[i][1] += roi->cy;
  }
}

static int clamp_corners(float corners_in[4][2], int corners_out[4][2])
{
  int is_clamp = 0;
  int i;

  for (i = 0; i < 4; i++) {
    corners_out[i][0] = (int)corners_in[i][0];
    corners_out[i][1] = (int)corners_in[i][1];
    is_clamp |= clamp_point(&corners_out[i][0], &corners_out[i][1]);
  }

  return is_clamp;
}

static void display_roi(roi_t *roi)
{
  float corners_f[4][2];
  int corners[4][2];
  int is_clamp;
  int i;

  /* compute box corners */
  roi_to_corners(roi, corners_f);

  /* clamp */
  is_clamp = clamp_corners(corners_f, corners);
  if (is_clamp)
    return ;

  /* display */
  for (i = 0; i < 4; i++)
    UTIL_LCD_DrawLine(corners[i][0], corners[i][1], corners[(i + 1) % 4][0], corners[(i + 1) % 4][1],
                      UTIL_LCD_COLOR_RED);
}

static void decode_ld_landmark(roi_t *roi, ld_point_t *lm, ld_point_t *decoded)
{
  float rotation = roi->rotation;
  float w = roi->w;
  float h = roi->h;

  decoded->x = roi->cx + (lm->x - 0.5) * w * cos(rotation) - (lm->y - 0.5) * h * sin(rotation);
  decoded->y = roi->cy + (lm->x - 0.5) * w * sin(rotation) + (lm->y - 0.5) * h * cos(rotation);
}

static void display_ld_hand(hand_info_t *hand)
{
  const int disk_radius = DISK_RADIUS;
  roi_t *roi = &hand->roi;
  //LD_LANDMARK_NB correspond au nb de points repères(21)
  int x[LD_LANDMARK_NB];
  int y[LD_LANDMARK_NB];
  int is_clamped[LD_LANDMARK_NB];
  ld_point_t decoded;
  int i;

  for (i = 0; i < LD_LANDMARK_NB; i++) {
    decode_ld_landmark(roi, &hand->ld_landmarks[i], &decoded);
    // decode mets les coordonnées en x,y cartésien
    x[i] = (int)decoded.x;
    y[i] = (int)decoded.y;
    // is_clamped teste si le point x[i], y[i] est dans le cadre
    is_clamped[i] = clamp_point_with_margin(&x[i], &y[i], disk_radius);
  }
  // si les points sont dans l'image, affiche un cercle jaune sur l'écran
  for (i = 0; i < LD_LANDMARK_NB; i++) {
    if (is_clamped[i])
      continue;
    UTIL_LCD_FillCircle(x[i], y[i], disk_radius, UTIL_LCD_COLOR_YELLOW);
  }
  // si deux points reliés sont dans l'écran, affiche une ligne noire les liants
  for (i = 0; i < LD_BINDING_NB; i++) {
    if (is_clamped[ld_bindings_idx[i][0]] || is_clamped[ld_bindings_idx[i][1]])
      continue;
    UTIL_LCD_DrawLine(x[ld_bindings_idx[i][0]], y[ld_bindings_idx[i][0]],
                      x[ld_bindings_idx[i][1]], y[ld_bindings_idx[i][1]],
                      UTIL_LCD_COLOR_BLACK);
  }
}

#define GESTURE_MARGIN 100.f
static gesture_t classify_gesture (hand_info_t *hand){
	   const int disk_radius = DISK_RADIUS;
	   roi_t *roi = &hand->roi;
	  //LD_LANDMARK_NB correspond au nb de points repères
	  int x[LD_LANDMARK_NB];
	  int y[LD_LANDMARK_NB];
	  int is_clamped[LD_LANDMARK_NB];
	  ld_point_t decoded;
	  int i;

	  for (i = 0; i < LD_LANDMARK_NB; i++) {
	    decode_ld_landmark(roi, &hand->ld_landmarks[i], &decoded);
	    // decode mets les coordonnées en x,y utilisable
	    x[i] = (int)decoded.x;
	    y[i] = (int)decoded.y;
	    // teste si le point x[i], y[i] est dans le cadre
	    is_clamped[i] = clamp_point_with_margin(&x[i], &y[i], disk_radius);
	  	  }
	    gesture_t g = G_NONE;
	  if (is_clamped[0]==0 && is_clamped[4]==0 && is_clamped[8]==0 && is_clamped[12]==0 && is_clamped[16]==0 && is_clamped[20]==0) {
		   // prend la position en x et y de la paume
		   float wrist_x = x[0];
		   float wrist_y = y[0];

		   // calcule les positions en x et y moyenne des bouts de chaque doigt
		   float tips_y = (y[4] + y[8] + y[12] + y[16] + y[20]) / 5.0f;
		   float tips_x = (x[4] + x[8] + x[12] + x[16] + x[20]) / 5.0f;


		    // paume ouverte: stop, vers le bas: backward, vers l'avant: forward
		    // les +/- 100 permettent de donner plus d'espace de mouvement
		    //(pour ne pas devoir être extrèmement précis pour pouvoir avancer)
			if (tips_y < wrist_y - GESTURE_MARGIN) g = G_STOP;
			else if (tips_y > wrist_y + GESTURE_MARGIN) g = G_BACKWARD;
			else g= G_FORWARD;
			// l'inclinaison gauche/droite est choisie après le choix de déplacement pour qu'une main
			//inclinée à droite ne soit pas considérée comme un forward(on priorise le fait de tourner à
			// celui d'avancer)
			// de même ici les +/- 100 permmettent de ne pas toujours finir par obtenir left ou right
			//tant que la main est à peu près centrée
			if (tips_x < wrist_x - GESTURE_MARGIN) g = G_LEFT;
			else if (tips_x > wrist_x + GESTURE_MARGIN) g = G_RIGHT;
	  	  }
	  //pour stopper la détection, on maintient la position stop
	  if (g==G_STOP){
		  stop_count+=1;
	  }
	  else{
		  stop_count=0;
	  }
	  if (stop_count==200){
		  stop_count=0;
		  hand_recognition=0;
		  lock_state=1;
	  }
	  return g; }


void  gesture_start (hand_info_t *hand){
	   const int disk_radius = DISK_RADIUS;
	   roi_t *roi = &hand->roi;
	  //LD_LANDMARK_NB correspond au nb de points repères
	  int x[LD_LANDMARK_NB];
	  int y[LD_LANDMARK_NB];
	  int is_clamped[LD_LANDMARK_NB];
	  ld_point_t decoded;
	  int i;
	  for (i = 0; i < LD_LANDMARK_NB; i++) {
	    decode_ld_landmark(roi, &hand->ld_landmarks[i], &decoded);
	    // decode mets les coordonnées en x,y utilisable
	    x[i] = (int)decoded.x;
	    y[i] = (int)decoded.y;
	    // teste si le point x[i], y[i] est dans le cadre
	    is_clamped[i] = clamp_point_with_margin(&x[i], &y[i], disk_radius);
	  	  }
	  // pour débloquer la détection de mouvement, on observe uniquement l'index et la paume
	  // pour débloquer, il  faut faire passer l'index de gauche à droite de la paume( avec une marge
	  // pour ne pas débloquer par accident)
	  if (is_clamped[0]==0 && is_clamped[8]==0 ) {
		  if (hand_recognition==0){
			   if(x[8]<x[0] -GESTURE_MARGIN){
				   hand_recognition=1;
			   }
		   }
		   if (hand_recognition==1){
			   if(y[8]<y[0] - GESTURE_MARGIN){
				   hand_recognition=2;
		   			}
		   }
		   if (hand_recognition==2){
			   if(x[8]>x[0] + GESTURE_MARGIN){
				   hand_recognition=3;
		 		   lock_state=0;
		 		   			}
		 		   }
	  	 }
	  //si aucune main n'est détectée sur l'écran, on réinitialise le blocage
	  else{
		  hand_recognition=0;
	  }
}


void display_hand(display_info_t *info, hand_info_t *hand)
{
  if (info->is_pd_displayed) {
    display_pd_hand(&hand->pd_hands);
    display_roi(&hand->roi);
  }
  if (info->is_ld_displayed)
    display_ld_hand(hand);
}

static void Display_NetworkOutput(display_info_t *info)
{
  float cpu_load_one_second;
  int line_nb = 0;
  float nn_fps;
  int i;

  /* clear previous ui */
  UTIL_LCD_FillRect(lcd_fg_area.X0, lcd_fg_area.Y0, lcd_fg_area.XSize, lcd_fg_area.YSize, 0x00000000); /* Clear previous boxes */

  /* cpu load */
  cpuload_update(&cpu_load);
  cpuload_get_info(&cpu_load, NULL, &cpu_load_one_second, NULL);
  /* command */
  gesture_t gesture = classify_gesture(&disp.info.hands[0]);
  /* draw metrics */
  nn_fps = 1000.0 / info->nn_period_ms;
  // on ajoute l'affichage de l'état de l'écran à gauche
  UTIL_LCDEx_PrintfAt(0, LINE(line_nb), LEFT_MODE, "State");
  UTIL_LCDEx_PrintfAt(0, LINE(line_nb),  RIGHT_MODE, "Cpu load");
  line_nb += 1;
  UTIL_LCDEx_PrintfAt(0, LINE(line_nb),  RIGHT_MODE, "   %.1f%%", cpu_load_one_second);
  if (lock_state==0){
	UTIL_LCD_SetTextColor(UTIL_LCD_COLOR_GREEN);
  	UTIL_LCDEx_PrintfAt(0, LINE(line_nb), LEFT_MODE, "unlocked");}
  if(lock_state==1){
	  UTIL_LCD_SetTextColor(UTIL_LCD_COLOR_RED);
  	UTIL_LCDEx_PrintfAt(0, LINE(line_nb), LEFT_MODE, "locked");
    }
  UTIL_LCD_SetTextColor(UTIL_LCD_COLOR_WHITE);
  line_nb += 2;
  // ainsi que la commande perçue si la détection est débloquée
  if (lock_state==0){
	  UTIL_LCDEx_PrintfAt(0, LINE(line_nb),  LEFT_MODE, "Command");}
  UTIL_LCDEx_PrintfAt(0, LINE(line_nb), RIGHT_MODE, "Inferences");
  line_nb += 1;
  if (lock_state==0){
	  switch (gesture)
		{
		  case G_STOP:
			UTIL_LCDEx_PrintfAt(0, LINE(line_nb),  LEFT_MODE, "STOP");
			break;
		  case G_BACKWARD:
			UTIL_LCDEx_PrintfAt(0, LINE(line_nb),  LEFT_MODE, "BACKWARD");
			break;
		  case G_FORWARD:
			UTIL_LCDEx_PrintfAt(0, LINE(line_nb),  LEFT_MODE, "FORWARD");
			break;
		  case G_LEFT:
			UTIL_LCDEx_PrintfAt(0, LINE(line_nb),  LEFT_MODE, "LEFT");
			break;
		  case G_RIGHT:
			UTIL_LCDEx_PrintfAt(0, LINE(line_nb),  LEFT_MODE, "RIGHT");
			break;
		  default:
			UTIL_LCDEx_PrintfAt(0, LINE(line_nb),  LEFT_MODE, "NONE");
			break;

		}
  }
  UTIL_LCDEx_PrintfAt(0, LINE(line_nb), RIGHT_MODE, " pd %2ums", info->pd_ms);
  line_nb += 1;
  UTIL_LCDEx_PrintfAt(0, LINE(line_nb), RIGHT_MODE, " hl %2ums", info->hl_ms);
  line_nb += 2;
  UTIL_LCDEx_PrintfAt(0, LINE(line_nb), RIGHT_MODE, "  %.1f FPS", nn_fps);
  line_nb +=2;
  if (DBG_INFO) {
    UTIL_LCDEx_PrintfAt(0, LINE(line_nb), RIGHT_MODE, "Display");
    line_nb += 1;
    UTIL_LCDEx_PrintfAt(0, LINE(line_nb), RIGHT_MODE, "   %ums", info->disp_ms);
    line_nb += 1;
  }

  /* display palm detector output */
  for (i = 0; i < info->pd_hand_nb; i++) {
    if (info->hands[i].is_valid)
      display_hand(info, &info->hands[i]);
  }

  if (DBG_INFO)
    UTIL_LCDEx_PrintfAt(0, LINE(line_nb),  RIGHT_MODE, "pd : %5.1f %%", info->pd_max_prob * 100);
}

static void palm_detector_init(pd_model_info_t *info)
{
  const LL_Buffer_InfoTypeDef *nn_out_info = LL_ATON_Output_Buffers_Info_palm_detector();
  const LL_Buffer_InfoTypeDef *nn_in_info = LL_ATON_Input_Buffers_Info_palm_detector();
  int ret;

  /* model info */
  info->nn_in_len = LL_Buffer_len(&nn_in_info[0]);
  info->prob_out = (float *) LL_Buffer_addr_start(&nn_out_info[0]);
  info->prob_out_len = LL_Buffer_len(&nn_out_info[0]);
  assert(info->prob_out_len == AI_PD_MODEL_PP_TOTAL_DETECTIONS * sizeof(float));
  info->boxes_out = (float *) LL_Buffer_addr_start(&nn_out_info[1]);
  info->boxes_out_len = LL_Buffer_len(&nn_out_info[1]);
  assert(info->boxes_out_len == AI_PD_MODEL_PP_TOTAL_DETECTIONS * sizeof(float) * 18);

  /* post processor info */
  ret = app_postprocess_init(&info->static_param, &NN_Instance_palm_detector);
  assert(ret == AI_PD_POSTPROCESS_ERROR_NO);
}

static int palm_detector_run(uint8_t *buffer, pd_model_info_t *info, uint32_t *pd_exec_time)
{
  uint32_t start_ts;
  int hand_nb;
  int ret;
  int i;

  start_ts = HAL_GetTick();
  /* Note that we don't need to clean/invalidate those input buffers since they are only access in hardware */
  ret = LL_ATON_Set_User_Input_Buffer_palm_detector(0, buffer, info->nn_in_len);
  assert(ret == LL_ATON_User_IO_NOERROR);

  LL_ATON_RT_Main(&NN_Instance_palm_detector);

  ret = app_postprocess_run((void * []){info->prob_out, info->boxes_out}, 2, &info->pd_out, &info->static_param);
  assert(ret == AI_PD_POSTPROCESS_ERROR_NO);
  hand_nb = MIN(info->pd_out.box_nb, PD_MAX_HAND_NB);

  for (i = 0; i < hand_nb; i++) {
    cvt_pd_coord_to_screen_coord(&info->pd_out.pOutData[i]);
    pd_box_to_roi(&info->pd_out.pOutData[i], &rois[i]);
  }

  /* Discard nn_out region (used by pp_outputs variables) to avoid Dcache evictions during nn inference */
  CACHE_OP(SCB_InvalidateDCache_by_Addr(info->prob_out, info->prob_out_len));
  CACHE_OP(SCB_InvalidateDCache_by_Addr(info->boxes_out, info->boxes_out_len));

  *pd_exec_time = HAL_GetTick() - start_ts;

  return hand_nb;
}

static void hand_landmark_init(hl_model_info_t *info)
{
  const LL_Buffer_InfoTypeDef *nn_out_info = LL_ATON_Output_Buffers_Info_hand_landmark();
  const LL_Buffer_InfoTypeDef *nn_in_info = LL_ATON_Input_Buffers_Info_hand_landmark();

  info->nn_in = LL_Buffer_addr_start(&nn_in_info[0]);
  info->nn_in_len = LL_Buffer_len(&nn_in_info[0]);
  info->prob_out = (float *) LL_Buffer_addr_start(&nn_out_info[2]);
  info->prob_out_len = LL_Buffer_len(&nn_out_info[2]);
  assert(info->prob_out_len == sizeof(float));
  info->landmarks_out = (float *) LL_Buffer_addr_start(&nn_out_info[3]);
  info->landmarks_out_len = LL_Buffer_len(&nn_out_info[3]);
  assert(info->landmarks_out_len == sizeof(float) * 63);
}

#if HAS_ROTATION_SUPPORT == 0
static int hand_landmark_prepare_input(uint8_t *buffer, roi_t *roi, hl_model_info_t *info)
{
  float corners_f[4][2];
  int corners[4][2];
  uint8_t* out_data;
  size_t height_out;
  uint8_t *in_data;
  size_t height_in;
  size_t width_out;
  size_t width_in;
  int is_clamped;

  /* defaults when no clamping occurs */
  out_data = info->nn_in;
  width_out = LD_WIDTH;
  height_out = LD_HEIGHT;

  roi_to_corners(roi, corners_f);
  is_clamped = clamp_corners(corners_f, corners);

  /* If clamp perform a partial resize */
  if (is_clamped) {
    int offset_x;
    int offset_y;

    /* clear target memory since resize will partially write it */
    memset(info->nn_in, 0, info->nn_in_len);

    /* compute start address of output buffer */
    offset_x = (int)(((corners[0][0] - corners_f[0][0]) * LD_WIDTH) / (corners_f[2][0] - corners_f[0][0]));
    offset_y = (int)(((corners[0][1] - corners_f[0][1]) * LD_HEIGHT) / (corners_f[2][1] - corners_f[0][1]));
    out_data += offset_y * (int)LD_WIDTH * DISPLAY_BPP + offset_x * DISPLAY_BPP;

    /* compute output width and height */
    width_out = (int)((corners[2][0] - corners[0][0]) / (corners_f[2][0] - corners_f[0][0]) * LD_WIDTH);
    height_out = (int)((corners[2][1] - corners[0][1]) / (corners_f[2][1] - corners_f[0][1]) * LD_HEIGHT);

    assert(width_out > 0);
    assert(height_out > 0);
    {
      uint8_t* out_data_end;

      out_data_end = out_data + (int)LD_WIDTH * DISPLAY_BPP * (height_out - 1) + DISPLAY_BPP * width_out - 1;

      assert(out_data_end >= info->nn_in);
      assert(out_data_end < info->nn_in + info->nn_in_len);
    }
  }

  in_data = buffer + corners[0][1] * LCD_BG_WIDTH * DISPLAY_BPP + corners[0][0]* DISPLAY_BPP;
  width_in = corners[2][0] - corners[0][0];
  height_in = corners[2][1] - corners[0][1];

  assert(width_in > 0);
  assert(height_in > 0);
  {
    uint8_t* in_data_end;

    in_data_end = in_data + LCD_BG_WIDTH * DISPLAY_BPP * (height_in - 1) + DISPLAY_BPP * width_in - 1;

    assert(in_data_end >= buffer);
    assert(in_data_end < buffer + LCD_BG_WIDTH * LCD_BG_HEIGHT * DISPLAY_BPP);
  }

  IPL_resize_bilinear_iu8ou8_with_strides_RGB(in_data, out_data, LCD_BG_WIDTH * DISPLAY_BPP, LD_WIDTH * DISPLAY_BPP,
                                              width_in, height_in, width_out, height_out);

  return 0;
}
#else
static void app_transform(nema_matrix3x3_t t, app_v3_t v)
{
  app_v3_t r;
  int i;

  for (i = 0; i < 3; i++)
    r[i] = t[i][0] * v[0] + t[i][1] * v[1] + t[i][2] * v[2];

  for (i = 0; i < 3; i++)
    v[i] = r[i];
}

static int hand_landmark_prepare_input(uint8_t *buffer, roi_t *roi, hl_model_info_t *info)
{
  app_v3_t vertex[] = {
    {           0,             0, 1},
    {LCD_BG_WIDTH,             0, 1},
    {LCD_BG_WIDTH, LCD_BG_HEIGHT, 1},
    {           0, LCD_BG_HEIGHT, 1},
  };
  GFXMMU_BuffersTypeDef buffers = { 0 };
  nema_matrix3x3_t t;
  int ret;
  int i;

  buffers.Buf0Address = (uint32_t) info->nn_in;
  ret = HAL_GFXMMU_ModifyBuffers(&hgfxmmu, &buffers);
  assert(ret == HAL_OK);

  /* bind destination texture */
  nema_bind_dst_tex(GFXMMU_VIRTUAL_BUFFER0_BASE, LD_WIDTH, LD_HEIGHT, NEMA_RGBA8888, -1);
  nema_set_clip(0, 0, LD_WIDTH, LD_HEIGHT);
  nema_clear(0);
  /* bind source texture */
  nema_bind_src_tex((uintptr_t) buffer, LCD_BG_WIDTH, LCD_BG_HEIGHT, NEMA_RGBA8888, -1, NEMA_FILTER_BL);
  nema_enable_tiling(1);
  nema_set_blend_blit(NEMA_BL_SRC);

  /* let's go */
  nema_mat3x3_load_identity(t);
  nema_mat3x3_translate(t, -roi->cx, -roi->cy);
  nema_mat3x3_rotate(t, nema_rad_to_deg(-roi->rotation));
  nema_mat3x3_scale(t, LD_WIDTH / roi->w, LD_HEIGHT / roi->h);
  nema_mat3x3_translate(t, LD_WIDTH / 2, LD_HEIGHT / 2);
  for (i = 0 ; i < 4; i++)
    app_transform(t, vertex[i]);
  nema_blit_quad_fit(vertex[0][0], vertex[0][1], vertex[1][0], vertex[1][1],
                     vertex[2][0], vertex[2][1], vertex[3][0], vertex[3][1]);

  nema_cl_submit(&cl);
  nema_cl_wait(&cl);
  HAL_ICACHE_Invalidate();

  assert(!nema_get_error());

  return 0;
}
#endif

static int hand_landmark_run(uint8_t *buffer, hl_model_info_t *info, roi_t *roi,
                             ld_point_t ld_landmarks[LD_LANDMARK_NB])
{
  int is_clamped;
  int is_valid;

  is_clamped = hand_landmark_prepare_input(buffer, roi, info);
  CACHE_OP(SCB_CleanInvalidateDCache_by_Addr(info->nn_in, info->nn_in_len));
  if (is_clamped)
    return 0;

  LL_ATON_RT_Main(&NN_Instance_hand_landmark);

  is_valid = ld_post_process(info->prob_out, info->landmarks_out, ld_landmarks);

  /* Discard nn_out region (used by pp_input and pp_outputs variables) to avoid Dcache evictions during nn inference */
  CACHE_OP(SCB_InvalidateDCache_by_Addr(info->prob_out, info->prob_out_len));
  CACHE_OP(SCB_InvalidateDCache_by_Addr(info->landmarks_out, info->landmarks_out_len));

  return is_valid;
}

#if HAS_ROTATION_SUPPORT == 1
static void app_rot_init(hl_model_info_t *info)
{
  GFXMMU_PackingTypeDef packing = { 0 };
  int ret;

  printf("init nema\n");
  nema_init();
  assert(!nema_get_error());
  nema_ext_hold_enable(2);
  nema_ext_hold_irq_enable(2);
  nema_ext_hold_enable(3);
  nema_ext_hold_irq_enable(3);
  printf("init nema DONE %s\n", nema_get_sw_device_name());

  hgfxmmu.Instance = GFXMMU;
  hgfxmmu.Init.BlockSize = GFXMMU_12BYTE_BLOCKS;
  hgfxmmu.Init.AddressTranslation = DISABLE;
  ret = HAL_GFXMMU_Init(&hgfxmmu);
  assert(ret == HAL_OK);

  packing.Buffer0Activation = ENABLE;
  packing.Buffer0Mode       = GFXMMU_PACKING_MSB_REMOVE;
  packing.DefaultAlpha      = 0xff;
  ret = HAL_GFXMMU_ConfigPacking(&hgfxmmu, &packing);
  assert(ret == HAL_OK);

  cl = nema_cl_create_sized(8192);
  nema_cl_bind_circular(&cl);
}
#endif

static float ld_compute_rotation(ld_point_t lm[LD_LANDMARK_NB])
{
  float x0, y0, x1, y1;
  float rotation;

  x0 = lm[0].x;
  y0 = lm[0].y;
  x1 = lm[9].x;
  y1 = lm[9].y;

  rotation = M_PI * 0.5 - atan2f(-(y1 - y0), x1 - x0);

  return pd_cook_rotation(pd_normalize_angle(rotation));
}

static void ld_to_roi(ld_point_t lm[LD_LANDMARK_NB], roi_t *roi, pd_pp_box_t *next_pd)
{
  const int pd_to_ld_idx[AI_PD_MODEL_PP_NB_KEYPOINTS] = {0, 5, 9, 13, 17, 1, 2};
  const int indices[] = {0, 1, 2, 3, 5, 6, 9, 10, 13, 14, 17, 18};
  float max_x, max_y, min_x, min_y;
  int i;

  max_x = max_y = -10000;
  min_x = min_y =  10000;

  roi->rotation = ld_compute_rotation(lm);

  for (i = 0; i < ARRAY_NB(indices); i++) {
    max_x = MAX(max_x, lm[indices[i]].x);
    max_y = MAX(max_y, lm[indices[i]].y);
    min_x = MIN(min_x, lm[indices[i]].x);
    min_y = MIN(min_y, lm[indices[i]].y);
  }

  roi->cx = (max_x + min_x) / 2;
  roi->cy = (max_y + min_y) / 2;
  roi->w = (max_x - min_x);
  roi->h = (max_y - min_y);

  next_pd->x_center = roi->cx;
  next_pd->y_center = roi->cy;
  next_pd->width = roi->w;
  next_pd->height = roi->h;
  for (i = 0; i < AI_PD_MODEL_PP_NB_KEYPOINTS; i++) {
    next_pd->pKps[i].x = lm[pd_to_ld_idx[i]].x;
    next_pd->pKps[i].y = lm[pd_to_ld_idx[i]].y;
  }
}

static void compute_next_roi(roi_t *src, ld_point_t lm_in[LD_LANDMARK_NB], roi_t *next, pd_pp_box_t *next_pd)
{
  const float shift_x = 0;
  const float shift_y = -0.1;
  const float scale = 2.0;
  ld_point_t lm[LD_LANDMARK_NB];
  roi_t roi;
  int i;

  for (i = 0; i < LD_LANDMARK_NB; i++)
    decode_ld_landmark(src, &lm_in[i], &lm[i]);

  ld_to_roi(lm, &roi, next_pd);
  roi_shift_and_scale(&roi, shift_x, shift_y, scale, scale);

#if HAS_ROTATION_SUPPORT == 0
  /* In that case we can cancel rotation. This ensure corners are corrected oriented */
  roi.rotation = 0;
#endif

  *next = roi;
}

static void nn_thread_fct(void *arg)
{
  float nn_period_filtered_ms = 0;
  float pd_filtered_ms = 0;
  float ld_filtered_ms = 0;
  hl_model_info_t hl_info;
  pd_model_info_t pd_info;
  uint32_t nn_period_ms;
  uint32_t nn_period[2];
  uint8_t *nn_pipe_dst;
  pd_pp_point_t box_next_keypoints[AI_PD_MODEL_PP_NB_KEYPOINTS];
  pd_pp_box_t box_next;
  int is_tracking = 0;
  roi_t roi_next;
  uint32_t pd_ms;
  uint32_t hl_ms;
  int ret;
  int j;

  /* Current tracking algo only support single hand */
  assert(PD_MAX_HAND_NB == 1);

  /* setup models buffer info */
  palm_detector_init(&pd_info);
  box_next.pKps = box_next_keypoints;
  hand_landmark_init(&hl_info);

#if HAS_ROTATION_SUPPORT == 1
  app_rot_init(&hl_info);
#endif

  /*** App Loop ***************************************************************/
  nn_period[1] = HAL_GetTick();
  nn_pipe_dst = bqueue_get_free(&nn_input_queue, 0);
  assert(nn_pipe_dst);
  CAM_NNPipe_Start(nn_pipe_dst, CMW_MODE_CONTINUOUS);
  while (1)
  {
    uint8_t *capture_buffer;
    int idx_for_resize;

    nn_period[0] = nn_period[1];
    nn_period[1] = HAL_GetTick();
    nn_period_ms = nn_period[1] - nn_period[0];
    nn_period_filtered_ms = USE_FILTERED_TS ? (15 * nn_period_filtered_ms + nn_period_ms) / 16 : nn_period_ms;

    capture_buffer = bqueue_get_ready(&nn_input_queue);
    assert(capture_buffer);
    idx_for_resize = frame_event_nb_for_resize % DISPLAY_BUFFER_NB;

    /* Only start palm detector when not tracking hand */
    if (!is_tracking) {
      is_tracking = palm_detector_run(capture_buffer, &pd_info, &pd_ms);
      box_next.prob = pd_info.pd_out.pOutData[0].prob;
    } else {
      rois[0] = roi_next;
      copy_pd_box(&pd_info.pd_out.pOutData[0], &box_next);
      pd_ms = 0;
    }
    pd_filtered_ms = USE_FILTERED_TS ? (7 * pd_filtered_ms + pd_ms) / 8 : pd_ms;
    bqueue_put_free(&nn_input_queue);

    /* then run hand landmark detector if needed */
    if (is_tracking) {
      hl_ms = HAL_GetTick();
      is_tracking = hand_landmark_run(lcd_bg_buffer[idx_for_resize], &hl_info, &rois[0], ld_landmarks[0]);
      CACHE_OP(SCB_InvalidateDCache_by_Addr(lcd_bg_buffer[idx_for_resize], sizeof(lcd_bg_buffer[idx_for_resize])));
      if (is_tracking) {
        compute_next_roi(&rois[0], ld_landmarks[0], &roi_next, &box_next);
        if (hand_recognition==3){
        	gesture_t gesture = classify_gesture(&disp.info.hands[0]);
        	send_gesture_command(gesture);}
        else{
        	gesture_start(&disp.info.hands[0]);
        }

      }
      hl_ms = HAL_GetTick() - hl_ms;
    } else {
      hl_ms = 0;
    }
    ld_filtered_ms = USE_FILTERED_TS ? (7 * ld_filtered_ms + hl_ms) / 8 : hl_ms;

    /* update display stats */
    ret = xSemaphoreTake(disp.lock, portMAX_DELAY);
    assert(ret == pdTRUE);
    disp.info.pd_ms = is_tracking ? 0 : (int)pd_filtered_ms;
    disp.info.hl_ms = is_tracking ? (int)ld_filtered_ms : 0;
    disp.info.nn_period_ms = nn_period_filtered_ms;
    disp.info.pd_hand_nb = is_tracking;
    disp.info.pd_max_prob = pd_info.pd_out.pOutData[0].prob;
    disp.info.hands[0].is_valid = is_tracking;
    copy_pd_box(&disp.info.hands[0].pd_hands, &pd_info.pd_out.pOutData[0]);
    disp.info.hands[0].roi = rois[0];
    for (j = 0; j < LD_LANDMARK_NB; j++)
      disp.info.hands[0].ld_landmarks[j] = ld_landmarks[0][j];
    ret = xSemaphoreGive(disp.lock);
    assert(ret == pdTRUE);

    /* It's possible xqueue is empty if display is slow. So don't check error code that may by pdFALSE in that case */
    xSemaphoreGive(disp.update);
  }
}

static void dp_update_drawing_area()
{
  int ret;

  __disable_irq();
  ret = SCRL_SetAddress_NoReload(lcd_fg_buffer[lcd_fg_buffer_rd_idx], SCRL_LAYER_1);
  assert(ret == HAL_OK);
  __enable_irq();
}

static void dp_commit_drawing_area()
{
  int ret;

  __disable_irq();
  ret = SCRL_ReloadLayer(SCRL_LAYER_1);
  assert(ret == HAL_OK);
  __enable_irq();
  lcd_fg_buffer_rd_idx = 1 - lcd_fg_buffer_rd_idx;
}

static void on_ld_toggle_button_click(void *args)
{
  display_t *disp = (display_t *) args;
  int ret;

  ret = xSemaphoreTake(disp->lock, portMAX_DELAY);
  assert(ret == pdTRUE);
  disp->info.is_ld_displayed = !disp->info.is_ld_displayed;
  ret = xSemaphoreGive(disp->lock);
  assert(ret == pdTRUE);
}

static void on_pd_toggle_button_click(void *args)
{
  display_t *disp = (display_t *) args;
  int ret;

  ret = xSemaphoreTake(disp->lock, portMAX_DELAY);
  assert(ret == pdTRUE);
  disp->info.is_pd_displayed = !disp->info.is_pd_displayed;
  ret = xSemaphoreGive(disp->lock);
  assert(ret == pdTRUE);
}

static void dp_thread_fct(void *arg)
{
  button_t ld_toggle_button;
  button_t hd_toggle_button;
  uint32_t disp_ms = 0;
  display_info_t info;
  uint32_t ts;
  int ret;

#ifdef STM32N6570_DK_REV
  button_init(&ld_toggle_button, BUTTON_USER1, on_ld_toggle_button_click, &disp);
  button_init(&hd_toggle_button, BUTTON_TAMP, on_pd_toggle_button_click, &disp);
#else
  button_init(&ld_toggle_button, BUTTON_USER, on_ld_toggle_button_click, &disp);
  button_init(&hd_toggle_button, BUTTON_USER, on_pd_toggle_button_click, &disp);
#endif
  while (1)
  {
    ret = xSemaphoreTake(disp.update, portMAX_DELAY);
    assert(ret == pdTRUE);

    button_process(&ld_toggle_button);
    button_process(&hd_toggle_button);

    ret = xSemaphoreTake(disp.lock, portMAX_DELAY);
    assert(ret == pdTRUE);
    info = disp.info;
    ret = xSemaphoreGive(disp.lock);
    assert(ret == pdTRUE);
    info.disp_ms = disp_ms;

    ts = HAL_GetTick();
    dp_update_drawing_area();
    Display_NetworkOutput(&info);
    SCB_CleanDCache_by_Addr(lcd_fg_buffer[lcd_fg_buffer_rd_idx], LCD_FG_WIDTH * LCD_FG_HEIGHT* 2);
    dp_commit_drawing_area();
    disp_ms = HAL_GetTick() - ts;
  }
}

static void isp_thread_fct(void *arg)
{
  int ret;

  while (1) {
    ret = xSemaphoreTake(isp_sem, portMAX_DELAY);
    assert(ret == pdTRUE);

    CAM_IspUpdate();
  }
}

static void Display_init()
{
  SCRL_LayerConfig layers_config[2] = {
    {
      .origin = {lcd_bg_area.X0, lcd_bg_area.Y0},
      .size = {lcd_bg_area.XSize, lcd_bg_area.YSize},
#if HAS_ROTATION_SUPPORT == 0
      .format = SCRL_RGB888,
#else
      .format = SCRL_ARGB8888,
#endif
      .address = lcd_bg_buffer[lcd_bg_buffer_disp_idx],
    },
    {
      .origin = {lcd_fg_area.X0, lcd_fg_area.Y0},
      .size = {lcd_fg_area.XSize, lcd_fg_area.YSize},
      .format = SCRL_ARGB4444,
      .address = lcd_fg_buffer[1],
    },
  };
  SCRL_ScreenConfig screen_config = {
    .size = {lcd_bg_area.XSize, lcd_bg_area.YSize},
#ifdef SCR_LIB_USE_SPI
    .format = SCRL_RGB565,
#else
     .format = SCRL_YUV422, /* Use SCRL_RGB565 if host support this format to reduce cpu load */
#endif
    .address = screen_buffer,
    .fps = CAMERA_FPS,
  };
  int ret;

  ret = SCRL_Init((SCRL_LayerConfig *[2]){&layers_config[0], &layers_config[1]}, &screen_config);
  assert(ret == 0);

  UTIL_LCD_SetLayer(SCRL_LAYER_1);
  UTIL_LCD_Clear(UTIL_LCD_COLOR_TRANSPARENT);
  UTIL_LCD_SetFont(&LCD_FONT);
  UTIL_LCD_SetTextColor(UTIL_LCD_COLOR_WHITE);
}


void send_gesture_command(gesture_t gesture)
{
  char *cmd;
  switch (gesture)
  {
    case G_STOP:
      cmd = "STOP\r\n";
      break;
    case G_BACKWARD:
      cmd = "BACKWARD\r\n";
      break;
    case G_FORWARD:
      cmd = "FORWARD\r\n";
      break;
    case G_LEFT:
      cmd = "LEFT\r\n";
      break;
    case G_RIGHT:
      cmd = "RIGHT\r\n";
      break;
    default:
      cmd = "NONE\r\n";
      break;
  }
  printf("Sending over BT: %s", cmd);
  HAL_UART_Transmit(&huart2, (uint8_t*)cmd, strlen(cmd), HAL_MAX_DELAY);
}

void app_run()
{
  UBaseType_t isp_priority = FREERTOS_PRIORITY(2);
  UBaseType_t dp_priority = FREERTOS_PRIORITY(-2);
  UBaseType_t nn_priority = FREERTOS_PRIORITY(1);
  TaskHandle_t hdl;
  int ret;

  printf("Init application\n");


  /* Enable DWT so DWT_CYCCNT works when debugger not attached */
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

  /* screen init */
  memset(lcd_bg_buffer, 0, sizeof(lcd_bg_buffer));
  CACHE_OP(SCB_CleanInvalidateDCache_by_Addr(lcd_bg_buffer, sizeof(lcd_bg_buffer)));
  memset(lcd_fg_buffer, 0, sizeof(lcd_fg_buffer));
  CACHE_OP(SCB_CleanInvalidateDCache_by_Addr(lcd_fg_buffer, sizeof(lcd_fg_buffer)));
  Display_init();

  /* create buffer queues */
  ret = bqueue_init(&nn_input_queue, 2, (uint8_t *[2]){nn_input_buffers[0], nn_input_buffers[1]});
  assert(ret == 0);

  cpuload_init(&cpu_load);

  /*** Camera Init ************************************************************/  
  CAM_Init();

  /* sems + mutex init */
  isp_sem = xSemaphoreCreateCountingStatic(1, 0, &isp_sem_buffer);
  assert(isp_sem);
  disp.update = xSemaphoreCreateCountingStatic(1, 0, &disp.update_buffer);
  assert(disp.update);
  disp.lock = xSemaphoreCreateMutexStatic(&disp.lock_buffer);
  assert(disp.lock);

  /* Start LCD Display camera pipe stream */
  CAM_DisplayPipe_Start(lcd_bg_buffer[0], CMW_MODE_CONTINUOUS);

  /* threads init */
  hdl = xTaskCreateStatic(nn_thread_fct, "nn", configMINIMAL_STACK_SIZE * 2, NULL, nn_priority, nn_thread_stack,
                          &nn_thread);
  assert(hdl != NULL);
  hdl = xTaskCreateStatic(dp_thread_fct, "dp", configMINIMAL_STACK_SIZE * 2, NULL, dp_priority, dp_thread_stack,
                          &dp_thread);
  assert(hdl != NULL);
  hdl = xTaskCreateStatic(isp_thread_fct, "isp", configMINIMAL_STACK_SIZE * 2, NULL, isp_priority, isp_thread_stack,
                          &isp_thread);
  assert(hdl != NULL);
}

int CMW_CAMERA_PIPE_FrameEventCallback(uint32_t pipe)
{
  if (pipe == DCMIPP_PIPE1)
    app_main_pipe_frame_event();
  else if (pipe == DCMIPP_PIPE2)
    app_ancillary_pipe_frame_event();

  return HAL_OK;
}

int CMW_CAMERA_PIPE_VsyncEventCallback(uint32_t pipe)
{
  if (pipe == DCMIPP_PIPE1)
    app_main_pipe_vsync_event();

  return HAL_OK;
}
