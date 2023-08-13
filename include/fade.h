#ifndef _SWAYLOCK_FADE_H
#define _SWAYLOCK_FADE_H

#include <stdbool.h>
#include <stdint.h>

struct swaylock_fade {
	float current_time;
	float target_time;
	uint32_t old_time;
	double alpha;
};

void fade_update(struct swaylock_fade *fade, uint32_t time);
bool fade_is_complete(struct swaylock_fade *fade);

#endif
