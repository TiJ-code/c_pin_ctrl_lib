#include "pin_ctrl.h"
#include <pigpiod_if2.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define PIN 17

static inline void loggerf(char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	printf("\n");
	va_end(args);
}

int main(void) {
	PinCtrl_t ctrl = {
		.pi_handle  = -1,
		.log        = NULL	
		.low_active = false
	};
	if (pc_init(&loggerf, true, &ctrl) == PC_ERROR) {
		printf("Error: pc_init\n");
		return 1;
	}

	printf("pi_handle = %d\n", ctrl.pi_handle);

	if (pc_set_mode(&ctrl, PIN, PC_MODE_OUT) == PC_ERROR) {
		printf("Error: pc_set_mode\n");
		pc_free(&ctrl);
		return 1;
	}

	printf("Testing pin %d (%s)\n", PIN, pc_pin_to_str(&ctrl, PIN));

	for (int i = 0; i < 5; ++i) {
		printf("ON\n");
		pc_turn_pin(&ctrl, PIN, PC_STATE_ON);
		usleep(500 * 1000);

		printf("OFF\n");
		pc_turn_pin(&ctrl, PIN, PC_STATE_OFF);
		usleep(500 * 1000);
	}

	printf("TOGGLE\n");
	pc_toggle_pin(&ctrl, PIN);
	usleep(500 * 1000);
	pc_toggle_pin(&ctrl, PIN);
	usleep(500 * 1000),

	printf("TRIGGER\n");
	pc_trigger_pin(&ctrl, PIN, 1000, PC_STATE_ON);

	pc_free(&ctrl);
	return 0;
}
