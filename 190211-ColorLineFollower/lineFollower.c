/* lineFollower.c */ 
#include "kernel.h"
#include "kernel_id.h"
#include "ecrobot_interface.h"
#include "stdlib.h"

/* OSEK Declarations */
DeclareCounter(SysTimerCnt);
DeclareTask(ReadColour);
DeclareTask(ReadButtons);
DeclareTask(ManageState);

/* Definitions */
#define MOTOR_LEFT  NXT_PORT_A
#define MOTOR_RIGHT NXT_PORT_C
#define LIGHT_PORT  NXT_PORT_S1

// Color IDs
#define BLACK       (0)
#define WHITE       (1)
#define YELLOW      (2)
#define RED         (3)
#define GREEN       (4)
#define BLUE        (5)
#define LIGHT_BLUE  (6)
#define UNKNOWN     (99)

// Number of colors
#define NUM_COLORS  (7)

// Robot actions
#define CORNER    (6)
#define FORWARD   (5)
#define LEFT      (4)
#define RIGHT     (3)
#define FOLLOW    (2)
#define SEARCH    (1)
#define UNSURE    (0)

// States
#define START         (0)
#define FOLLOW_LINE   (1)
#define CALIBRATE     (2)
#define PAUSED        (3)
#define SEARCH_CORNER (4)
#define TURN          (5)
#define FINISHED      (6)

// Follow states
#define FOLLOW_RIGHT  (1)
#define FOLLOW_LEFT   (0)

// Orientation
#define NORTH         (0)
#define EAST          (1)
#define SOUTH         (2)
#define WEST          (3)

// CHANGE POSITION

#define TEST          (0)
#define ALTER         (1)

//Rand
#define RAND_MAX  (3);

// Time to hold switch
#define SWITCH_WAIT (3)

static S8 motor_right = 30;
static S8 motor_left = 30;

static U8 state = 0;
static U8 old_color;
static U8 color_cal_state = 0;
static U8 color;

static U32 enter_button = 0;
static U32 run_button = 0;

static  S16 colorCal[8][6];

static U8 random = 0;
static U8 line_side_G = 0;
static U8 make_turn;
static U8 line_found;

static U16 search_time = 0;
static U16 black_count = 0;

static U8 map[100][100];
static U8 x_position = 50;
static U8 y_position = 50;
static U8 orientation = NORTH;

static U8 checked[6];

void set_both_motor_speed(S8 left, S8 right) {

    nxt_motor_set_speed(MOTOR_RIGHT, right, 1);
    nxt_motor_set_speed(MOTOR_LEFT, left, 1);

}

/* nxtOSEK hooks */
void ecrobot_device_initialize(void) {

  /* Initialise color sensor */ 
  ecrobot_init_nxtcolorsensor(LIGHT_PORT, NXT_COLORSENSOR);   

}

void ecrobot_device_terminate(void) {

  /* Turn off motors */
  set_both_motor_speed(0,0);

  /* Terminate the I2C bus for color sensor */
  ecrobot_term_nxtcolorsensor(LIGHT_PORT);

}

void user_1ms_isr_type2(void) {

  /* Increment system counter through a 1ms alarm */
  (void)SignalCounter(SysTimerCnt); 

}

/* Query if it is viable to make a turn according to Tremauxes' Algorithm */
int query_turn(U8 x, U8 y) {

  // Check turn
  
  if ( map[x][y] < 2 ) {
  
    return true;

  }

  else { return false; }

}

/* Change the position of the robot in the map array */
int change_position(U8 check) {

  U8 temp_x_pos = x_position;
  U8 temp_y_pos = y_position;
  U8 temp_orientation;

	switch(orientation) {
				
		case NORTH:
	
			switch(make_turn) {

				case RIGHT:

					temp_x_pos++;
          temp_orientation = EAST;   
					break;

				case LEFT:

					temp_x_pos--;
					temp_orientation = WEST;
					break;

				case FORWARD:

					temp_y_pos++;
					break;

			}

		break;

		case EAST:

			switch(make_turn) {

				case RIGHT:

					temp_y_pos--;
					temp_orientation = SOUTH;
					break;

				case LEFT:

					temp_y_pos++;
					temp_orientation = NORTH; 
					break;

				case FORWARD:

					temp_x_pos++;
					break;

			}

		break;

		case SOUTH:

			switch(make_turn) {

				case RIGHT:

					temp_x_pos--;
					temp_orientation = WEST;
					break;

				case LEFT:

					temp_x_pos++;
					temp_orientation = EAST; 
					break;

				case FORWARD:

					temp_y_pos--;
					break;

			}

		break;

		case WEST:

			switch(make_turn) {

				case RIGHT:

					temp_y_pos++;
					temp_orientation = NORTH;
					break;

				case LEFT:

					temp_y_pos--;
					temp_orientation = SOUTH;
					break;

				case FORWARD:

					temp_x_pos--;
					break;

			}

		break;

	}

  /* Check if we are testing the position or altering the position */
  if (query_turn(temp_x_pos,temp_y_pos)) {

    if (check == ALTER) {

      x_position = temp_x_pos;
      y_position = temp_y_pos;
      orientation = temp_orientation;

    }
    
    return true;

  }
  else {

    checked[make_turn] = 1;
    return false;
  
  }  

}

void robot_follow(U8 direction, U8 line_side) {

  if (line_side == 0) {

    /* Set motor speeds depending on direction */
    switch(direction) {

      case UNSURE:

        motor_right = motor_right - 1;
        motor_left = motor_left - 1;
        break;

      case SEARCH:
      
        motor_right = motor_right + 15;
        motor_left = 20;
        break;

      case FOLLOW:

        if (old_color != BLACK || old_color != UNSURE) {
          motor_right = 25;
          motor_left = 28;
        }

        motor_right = motor_right + 1;
        motor_left = motor_left + 2;
        break;

      default:
        //change nothing
        break;

    }

    /* Stop motors going above or below certain speed thresholds */
    if (motor_right < 25) { 
      motor_right = 25; 
    } 
    else if (motor_right > 50) {
      motor_right = 50; 
    }

    if (motor_left < 25) {
      motor_left = 25;
    }
    else if (motor_left > 60) {
      motor_left = 60;
    }

  }

  else if (line_side == 1) {

    /* Set motor speeds depending on direction */
    switch(direction) {

      case UNSURE:
  
        motor_right = motor_right - 1;
        motor_left = motor_left - 1;
        break;

      case SEARCH:
      
        motor_right = 20;
        motor_left = motor_left + 15;
        break;

      case FOLLOW:

        if (old_color != BLACK || old_color != UNSURE) {
          motor_right = 28;
          motor_left = 25;
        }

        motor_right = motor_right + 2;
        motor_left = motor_left + 1;
        break;

      default:
        //change nothing
        break;

    }

    /* Stop motors going above or below certain speed thresholds */
    if (motor_right < 25) { 
      motor_right = 25; 
    } 
    else if (motor_right > 60) {
      motor_right = 60; 
    }

    if (motor_left < 25) {
      motor_left = 25;
    }
    else if (motor_left > 50) {
      motor_left = 50;
    }

  }

  /* set motor speeds */
  set_both_motor_speed(motor_left,motor_right);
}

void check_color() {

  S16 rgb[3];

  /* get RGB values from sensor */
  ecrobot_get_nxtcolorsensor_rgb(LIGHT_PORT, rgb);
  
  S16 red = rgb[0];
  S16 green = rgb[1];
  S16 blue = rgb[2];

  /* Keep information on the last known color */
  if ( color != UNKNOWN ) {
    old_color = color;
  }

  U8 x=0;

  /* Loop through colors and check values */
  while ( x < NUM_COLORS ) {

    if  ((  red   > colorCal[x][0]  &&  red   < colorCal[x][1]) && 
        (   green > colorCal[x][2]  &&  green < colorCal[x][3]) && 
        (   blue  > colorCal[x][4]  &&  blue  < colorCal[x][5]))
    {
      color = x;
    }

    x++;

  }

}

/* Task to manage state */
TASK (ManageState) {

  /* When robot is first switched on enter calibration mode */
  if (enter_button == SWITCH_WAIT && (state == START)) {

    state = CALIBRATE;
    enter_button++;

  }

  /* Pause robot once started */
  if (enter_button == SWITCH_WAIT && (state == FOLLOW_LINE)) {

    state = PAUSED;
    enter_button++;

  }

  /* Start robot once paused */
  if (enter_button == SWITCH_WAIT && (state == PAUSED)) {

    state = FOLLOW_LINE;
    enter_button++;

  }

  /* Re-calibrate once started */
  if (run_button == SWITCH_WAIT && ((state != START) || (state != CALIBRATE))) {

    state = CALIBRATE;
    run_button++;

  }
  
  switch (state) {

    case START:

      display_clear(0);
      display_goto_xy(0,0);
      display_string("ENTER TO START");
      display_update();
      break;

    case PAUSED:

      display_clear(0);
      display_goto_xy(0,0);
      display_string("ROBOT PAUSED");
      display_goto_xy(0,1);
      display_string("ENTER TO START");
      display_goto_xy(0,3);
      display_string("RIGHT ARROW");
      display_goto_xy(0,4);
      display_string("TO CALIBRATE");
      display_update();

      set_both_motor_speed(0,0);

      break;

    case SEARCH_CORNER:

      check_color();

      display_clear(0);
      display_goto_xy(0,0);
      display_string("SEARCHING");
      display_update();
      
      if (color != BLACK && line_found == 0) {

        set_both_motor_speed(-30,-30);

      }
      
      else if (color == BLACK && line_found == 0) {

        line_found=1;

      }

      else if (line_found == 1) {

        search_time++;

        if (search_time < 150) {
        
          set_both_motor_speed(50,-15);

          if (color == BLACK) {

            black_count++;

            if (black_count > 35) {

              state = FOLLOW_LINE;
              search_time = 0;
              black_count = 0;
              line_found = 0;

            }
          }
        }

        else if (search_time < 300) {

          set_both_motor_speed(-50,-15);

        }

        else if (search_time < 450) {

          set_both_motor_speed(-15, 50);

          if (color == BLACK) {

            black_count++;

            if (black_count > 35) {

              state = FOLLOW_LINE;
              search_time = 0;
              line_found = 0;
              black_count = 0;

            }  
          }
        }

        else if (search_time < 600) { 
        
          set_both_motor_speed(15,-50); 
      
        }

        else { search_time = 0; }

      }

      break;

    case FOLLOW_LINE:

      display_clear(0);
      display_goto_xy(0,0);
      display_string("ENTER TO PAUSE");
      display_goto_xy(0,2);
      display_string("RIGHT ARROW");
      display_goto_xy(0,3);
      display_string("TO CALIBRATE");
      display_update();

      if (enter_button == SWITCH_WAIT) {

        state = START;
        enter_button++;

      }
      
      /* Check color and act accordingly */
      check_color();
      
      int switch_color;

      if (old_color == color) {

        switch_color = color;
      }

      switch(switch_color) {

        case BLACK:
          robot_follow(FOLLOW, line_side_G);
          break;

        case WHITE:
          robot_follow(SEARCH, line_side_G);
          break;

        case YELLOW: 

          do {

            if (checked[LEFT] && checked[RIGHT]) {

              state = FINISHED;
              break;

            }
              
            random = rand()%(3-1)+1;

            switch(random) {

              case 1:

                make_turn = LEFT;
                break;
      
              case 2:
      
                make_turn = RIGHT;
                break;

            }

          } while ( !change_position(TEST));

          change_position(ALTER);

          state = TURN;

          break;

        case RED:

          if (old_color != RED) {
            random = rand()%(3-1)+1;
          }

          switch(random) {

            case 1:

              make_turn = FORWARD;
              break;

            case 2:

              make_turn = RIGHT;
              break;

          }

          state = TURN;

          break;
        
        case GREEN:

          make_turn = FORWARD;
          break;

        case BLUE:

          if (old_color != BLUE) {
            random=rand()%(3-1)+1;
          }

          switch(random) {

            case 1:

              make_turn = FORWARD;
              break;

            case 2:

              make_turn = LEFT;
              break;

          }

          state = TURN;

          break;

        case LIGHT_BLUE:

          state = SEARCH_CORNER;  
          break;

        case UNKNOWN:
          robot_follow(UNSURE, line_side_G);
          break;
      }

      break;

    case TURN:

      check_color();

      display_clear(0);
      display_goto_xy(0,0);
      display_string("TURNING");
      display_goto_xy(0,1);
      display_int(make_turn,1);
      display_update();
      
      if (line_found == 0 && color != BLACK && color != WHITE && color != UNKNOWN) {

        motor_left=-30;
        motor_right=-30;
      }
      
      else {

        line_found=1;
      }

      if (line_found == 1) {
      
        switch(make_turn) {

          case RIGHT:
            //right
            motor_right=-20;
            motor_left=45;
            line_side_G = FOLLOW_LEFT;
	 				  break;
    
          case LEFT:
            //left
            motor_right=45;
            motor_left=-20;
            line_side_G = FOLLOW_RIGHT;
            break;
    
          case FORWARD:
            //forward 
            motor_right=25;
            motor_left=25;
            break;

        }

        if (color == BLACK) {
        
          black_count++;

          if (black_count > 80) {
        
            state = FOLLOW_LINE;
            line_found = 0;
            black_count = 0;

          }
        }
      }

      /* Set motor speed */
      set_both_motor_speed(motor_left,motor_right);

      break;

    case CALIBRATE:

      set_both_motor_speed(0,0);

      display_clear(0);

      display_goto_xy(0,0);
      display_string("COLOR");
      display_goto_xy(0,1);
      display_string("CALIBRATION");
      display_goto_xy(0,2);
      display_string("============");

      if (color_cal_state < NUM_COLORS) {

        display_goto_xy(0,3);

        switch(color_cal_state) {

          case BLACK:
            display_string("BLACK");
            break;

          case WHITE:
            display_string("WHITE");
            break;

          case YELLOW:
            display_string("YELLOW");
            break;

          case RED:
            display_string("RED");
            break;

          case GREEN:
            display_string("GREEN");
            break;

          case BLUE:
            display_string("BLUE");
            break;

          case LIGHT_BLUE:
            display_string("LIGHT BLUE");
            break;

          default:
            display_string("NOT SET");
            break;
      
        }

        if (enter_button == 3) {

          S16 rgb[3];
          ecrobot_get_nxtcolorsensor_rgb(LIGHT_PORT, rgb);

          colorCal[color_cal_state][0] = (rgb[0]-50);
          colorCal[color_cal_state][1] = (rgb[0]+50);
          colorCal[color_cal_state][2] = (rgb[1]-50);
          colorCal[color_cal_state][3] = (rgb[1]+50);
          colorCal[color_cal_state][4] = (rgb[2]-50);
          colorCal[color_cal_state][5] = (rgb[2]+50);
        
          color_cal_state++;
          enter_button++;

        }

      }

      if (color_cal_state == (NUM_COLORS)) {

        display_clear(0);
        display_goto_xy(0,0);
        display_string("PLACE ON LINE");
        display_goto_xy(0,1);
        display_string("ENTER WHEN READY");
        
        if (enter_button == SWITCH_WAIT) {

          color_cal_state++;
          enter_button++;

        }

      }

      else if (color_cal_state == (NUM_COLORS+1)) {

        state = FOLLOW_LINE;
        color_cal_state = 0;
        display_clear(0);

      }

      display_update();
      break;

    case FINISHED:

      display_clear(0);

      display_goto_xy(0,0);
      display_string("FINISHED");

      set_both_motor_speed(0,0);
      break;

  }

  TerminateTask();

}

TASK (ReadButtons) {

  U8 readButton;

  readButton = ecrobot_is_ENTER_button_pressed();

  if (readButton) {

    enter_button++;

  }

  else {

    enter_button = 0;

  }

  readButton = ecrobot_is_RUN_button_pressed();

  if (readButton) {

    run_button++;

  }

  else {

    run_button = 0;

  }

  TerminateTask();

}

TASK(ReadColour) {

  while(1)
  {
    // communicates with NXT Color Sensor (this must be executed repeatedly in a background Task)
    ecrobot_process_bg_nxtcolorsensor(); 
  }

}
