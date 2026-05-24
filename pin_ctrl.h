#pragma once

#ifndef PIN_CTRL_H
#define PIN_CTRL_H

#include <stdbool.h>
#include <stdarg.h>

typedef enum pin_ctrl_result {
	PC_OK,
	PC_ERROR
} PinCtrlResult_t;

typedef enum pin_ctrl_state {
	PC_STATE_ON = 0,
	PC_STATE_OFF = 1
} PinCtrlState_t;

typedef enum pin_ctrl_mode {
	PC_MODE_IN,
	PC_MODE_OUT
} PinCtrlMode_t;

typedef struct pin_ctrl {
	int pi_handle;
	void (*log)(char *fmt, ...);
} PinCtrl_t;

PinCtrlResult_t pc_set_mode(PinCtrl_t *ctrl, int pin, PinCtrlMode_t mode);
PinCtrlResult_t pc_turn_pin(PinCtrl_t *ctrl, int pin, PinCtrlState_t state);
PinCtrlResult_t pc_toggle_pin(PinCtrl_t *ctrl, int pin);
PinCtrlResult_t pc_trigger_pin(PinCtrl_t *ctrl, int pin, int durationMillis, PinCtrlState_t state);

const char *pc_pin_to_str(PinCtrl_t *ctrl, int pin);

PinCtrlResult_t pc_init(void (*log)(char *fmt, ...), PinCtrl_t *out);
void pc_free(PinCtrl_t *ctrl);

#endif
