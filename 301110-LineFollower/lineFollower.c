/* lineFollower.c */ 
#include "kernel.h"
#include "kernel_id.h"
#include "ecrobot_interface.h"

/* OSEK Declarations */
DeclareCounter(SysTimerCnt);
DeclareTask(TaskLCD);
DeclareTask(TaskColour);

/* Definitions */
#define MOTOR_LEFT	NXT_PORT_A
#define MOTOR_RIGHT	NXT_PORT_C
#define LIGHT_PORT	NXT_PORT_S1

/* nxtOSEK hooks */
void ecrobot_device_initialize(void) {

}

void ecrobot_device_terminate(void) {

	colorsensor_term(LIGHT_PORT);

	nxt_motor_set_speed(MOTOR_RIGHT, 0, 1);
	nxt_motor_set_speed(MOTOR_LEFT, 0, 1);
}

static int colour;
static int red, green, blue;

/* nxtOSEK hook to be invoked from an ISR in category 2 */
void user_1ms_isr_type2(void) {  

    StatusType ercd;

    ercd = SignalCounter(SysTimerCnt); /* Increment OSEK Alarm Counter */
    if(ercd != E_OK)
    {
    	ShutdownOS(ercd);
    }
  	

}

TASK(TaskLCD) {

	/* U8 light_level;

		display_clear(0);

		light_level = ecrobot_get_light_sensor(LIGHT_PORT);

		display_goto_xy(0,0);
		display_string("Light Level: ");
		display_int(light_level, 0);

		display_update();
		systick_wait_ms(20);

		if (light_level > 150) {
			nxt_motor_set_speed(MOTOR_RIGHT, 50, 1);
			nxt_motor_set_speed(MOTOR_LEFT, 0, 1);
		}
		else if (light_level <= 150) {
			nxt_motor_set_speed(MOTOR_RIGHT, 50, 1);
			nxt_motor_set_speed(MOTOR_LEFT, 50, 1);
	*/

			
	display_clear(0);
		
	display_goto_xy(0,0);
	display_string("Red: ");
	display_int(red, 0);
	display_goto_xy(0,1);
	display_string("Green: ");
	display_int(green, 0);
	display_goto_xy(0,2);
	display_string("Blue: ");
	display_int(blue, 0);

	display_update();

	TerminateTask();

	
}

TASK(TaskColour) {

	colorsensor_init(LIGHT_PORT, COLORSENSOR);

	red = colorsensor_color_get(LIGHT_PORT);

	TerminateTask();

}
