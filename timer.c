#include "demo_bsp.h"
#include <FXRTOS.h>
#include "stdio.h"
#include "string.h"

fx_timer_t timer_0;
fx_timer_t timer_1;
fx_timer_t timer_2;
fx_timer_t timer_ctrl;

extern void led_on(void);
extern void led_off(void);

int
never_called_func(void* args)
{
    printf("Never_called_func called...\n\r");
}

int
canceler(void* target)
{
    printf("Canceler called so never_called_func will be never called\n\r");
    fx_timer_cancel(target);
}

int
i_led_on(void* args)
{
    led_on();
}

int
i_led_off(void* args)
{
    led_off();
}

// Because of this function, there is no need to really wait ~300000 ticks
// before LED blinking start.
int
go_to_future(void* args){
    printf("Going almost to blink start\n\r");
    fx_timer_set_tick_count(295000);
}

// This function reinits timers with new callback functions.
int
reinit(void* args)
{
    printf("Reinit called at %u ticks\n\r", fx_timer_get_tick_count());
    fx_timer_deinit(&timer_0);
    fx_timer_deinit(&timer_1);

    // Timer_0 and timer_1 prepared for blinking.
    // Timer_2 prepared for skipping time.
    fx_timer_init(&timer_0, i_led_on, NULL);
    fx_timer_init(&timer_1, i_led_off, NULL);
    fx_timer_init(&timer_2, go_to_future, NULL);

    fx_timer_set_abs(&timer_0, 300000, 2000);
    fx_timer_set_abs(&timer_1, 301000, 2000);
    fx_timer_set_rel(&timer_2, 5000, 0);
}

void
fx_app_init(void)
{
    // Initializing timers.
    fx_timer_init(&timer_0, never_called_func, NULL);
    fx_timer_init(&timer_1, canceler, &timer_0);
    fx_timer_init(&timer_ctrl, reinit, NULL);

    // Setting call time for timers
    fx_timer_set_rel(&timer_1, 1000, 0);
    fx_timer_set_rel(&timer_0, 5000, 0);
    fx_timer_set_rel(&timer_ctrl, 6000, 0);
}

void
fx_intr_handler(void)
{
    ;
}

int
main(void)
{
    demo_bsp_init();
    //
    // Kernel start. This function must be called with interrupts disabled.
    //
    fx_kernel_entry();
}
