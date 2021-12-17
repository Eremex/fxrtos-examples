#ifndef BSP_CONFIG_H
#define BSP_CONFIG_H
/*
*
*
*/

extern const unsigned timer_irq;

void led_on(void);
void led_off(void);

void timer_restart(void);

void demo_bsp_init(void);

#endif