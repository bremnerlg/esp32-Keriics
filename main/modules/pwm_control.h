#ifndef DRIVER_LEDC_H
#define DRIVER_LEDC_H

#include "driver/ledc.h"

#endif


ledc_timer_config_t timer = {
	.speed_mode		= LEDC_LOW_SPEED_MODE,
	.duty_resolution	= LEDC_TIMER_10_BIT,
	.timer_num		= LEDC_TIMER_0,
	.freq_hz		= 500,
	.clk_cfg		= LEDC_AUTO_CLK,
};
ledc_timer_config(&timer);

ledc_channel_config_t ch = {
	.gpio_num		= 2,
	.speed_mode		= LEDC_LOW_SPEED_MODE,
	. channel		= LEDC_CHANNEL_0,
	.timer_sel		= LEDC_TIMER_0,
	.duty			= 0,
	.hpoint			= 0,
};

void init_ledc_pwm()
{
	ledc_channel_config(&ch);

	ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 512);
	ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}
