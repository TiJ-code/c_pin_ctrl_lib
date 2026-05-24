#include "pin_ctrl.h"
#include <unistd.h>
#include <pigpiod_if2.h>

#define ENSURE_PC_INIT(pi_ctrl)             \
	do {                                        \
		if (!(pi_ctrl) || (pi_ctrl)->pi_handle < 0) \
			return PC_ERROR;                            \
	} while (false)

#define PC_LOG(pi_ctrl, fmt, ...)   \
	do {                                \
		if ((pi_ctrl)->log)                 \
			(pi_ctrl)->log(fmt, ##__VA_ARGS__); \
	} while (false)

static inline void _pc_sleep_ms(int millis) {
	usleep(millis * 1000);
}

static inline const char *_pc_state_to_str(PinCtrlState_t state) {
	return state == PC_STATE_ON
		? "ON"
		: "OFF";
}

static inline const char *_pc_mode_to_str(PinCtrlMode_t mode) {
	return mode == PC_MODE_IN
		? "IN"
		: "OUT";
}

static inline int _pc_state_to_gpio(PinCtrl_t *ctrl, PinCtrlState_t state) {
	if (ctrl->low_active) {
		return state == PC_STATE_ON
			? PI_LOW
			: PI_HIGH;
	}

	return state == PC_STATE_ON
		? PI_HIGH
		: PI_LOW;
}

static inline PinCtrlState_t _pc_gpio_to_state(PinCtrl_t *ctrl, int gpio) {
	if (ctrl->low_active) {
		return gpio == PI_LOW
			? PC_STATE_ON
			: PC_STATE_OFF;
	}

	return gpio == PI_HIGH
		? PC_STATE_ON
		: PC_STATE_OFF;
}

PinCtrlResult_t pc_set_mode(PinCtrl_t *ctrl, int pin, PinCtrlMode_t mode) {
	ENSURE_PC_INIT(ctrl);

	PC_LOG(ctrl, "pc_set_mode: pin=%d mode=%s",
		pin,
		_pc_mode_to_str(mode));

	int actualMode;

	switch (mode) {
		case PC_MODE_IN:
			actualMode = PI_INPUT;
			break;

		case PC_MODE_OUT:
			actualMode = PI_OUTPUT;
			break;

		default:
			return PC_ERROR;
	}

	int result = set_mode(ctrl->pi_handle, pin, actualMode);
	return result == 0 ? PC_OK : PC_ERROR;
}

PinCtrlResult_t pc_turn_pin(PinCtrl_t *ctrl, int pin, PinCtrlState_t state) {
	ENSURE_PC_INIT(ctrl);

	PC_LOG(ctrl, "pc_turn_pin: pin=%d state=%s",
		pin,
		_pc_state_to_str(state));

	if (pc_set_mode(ctrl, pin, PC_MODE_OUT) != PC_OK)
		return PC_ERROR;

	int gpioState = _pc_state_to_gpio(ctrl, state);

	int result = gpio_write(ctrl->pi_handle, pin, gpioState);
	return result == 0 ? PC_OK : PC_ERROR;
}

PinCtrlResult_t pc_toggle_pin(PinCtrl_t *ctrl, int pin) {
	ENSURE_PC_INIT(ctrl);

	int raw = gpio_read(ctrl->pi_handle, pin);
	if (raw < 0)
		return PC_ERROR;

	PinCtrlState_t current = _pc_gpio_to_state(ctrl, raw);

	PinCtrlState_t next =
		current == PC_STATE_ON
			? PC_STATE_OFF
			: PC_STATE_ON;

	PC_LOG(ctrl,
		"pc_toggle_pin: pin=%d %s -> %s",
		pin,
		_pc_state_to_str(current),
		_pc_state_to_str(next));

	return pc_turn_pin(ctrl, pin, next);
}

PinCtrlResult_t pc_trigger_pin(
	PinCtrl_t *ctrl,
	int pin,
	int durationMillis,
	PinCtrlState_t state
) {
	ENSURE_PC_INIT(ctrl);

	PinCtrlState_t restore =
		state == PC_STATE_ON
			? PC_STATE_OFF
			: PC_STATE_ON;

	PC_LOG(ctrl,
		"pc_trigger_pin: pin=%d pulse=%s duration=%dms",
		pin,
		_pc_state_to_str(state),
		durationMillis);

	PinCtrlResult_t r = pc_turn_pin(ctrl, pin, state);
	if (r != PC_OK)
		return r;

	_pc_sleep_ms(durationMillis);

	return pc_turn_pin(ctrl, pin, restore);
}

const char *pc_pin_to_str(PinCtrl_t *ctrl, int pin) {
	ENSURE_PC_INIT(ctrl);

	int raw = gpio_read(ctrl->pi_handle, pin);
	if (raw < 0)
		return NULL;

	PinCtrlState_t state = _pc_gpio_to_state(ctrl, raw);

	const char *result = _pc_state_to_str(state);

	PC_LOG(ctrl,
		"pc_pin_to_str: pin=%d -> %s",
		pin,
		result);

	return result;
}

PinCtrlResult_t pc_init(
	void (*log)(char *fmt, ...),
	bool low_active,
	PinCtrl_t *out
) {
	if (!out)
		return PC_ERROR;

	int pi_handle = pigpio_start(NULL, NULL);
	if (pi_handle < 0)
		return PC_ERROR;

	out->pi_handle  = pi_handle;
	out->log        = log;
	out->low_active = low_active;

	return PC_OK;
}

void pc_free(PinCtrl_t *ctrl) {
	if (!ctrl || ctrl->pi_handle < 0)
		return;

	pigpio_stop(ctrl->pi_handle);

	ctrl->pi_handle  = -1;
	ctrl->log        = NULL;
	ctrl->low_active = false;
}
