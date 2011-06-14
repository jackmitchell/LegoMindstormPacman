/** 
* @file lineFollower.c
* @brief Main code for an Autonomous Colour Line Following Robot
* with Area Mapping and Decision Making Abilities
* @author Jack Mitchell
* @date 2011-02-21
*/

#include "kernel.h"
#include "kernel_id.h"
#include "ecrobot_interface.h"
#include "stdlib.h"

#include "defines.h"
#include "functions.h"
#include "lineFollower.h"

// ========================================================
// OSEK Task Declarations
// ========================================================

DeclareCounter(SysTimerCnt);
DeclareTask(ReadColour);
DeclareTask(ReadButtons);
DeclareTask(ManageState);

// ========================================================
// nxtOSEK system hook functions
// ========================================================


/** 
* @brief code to run on the initialisation of the program
*/
void ecrobot_device_initialize(void) {

  /* Initialise color sensor */ 
  ecrobot_init_nxtcolorsensor(LIGHT_PORT, NXT_COLORSENSOR);   

}


/** 
* @brief code to run when the program is terminated
*/
void ecrobot_device_terminate(void) {

  /* Turn off motors */
  set_both_motor_speed(0,0);

  /* Terminate the I2C bus for color sensor */
  ecrobot_term_nxtcolorsensor(LIGHT_PORT);

}


/** 
* @brief code to run at every 1ms system alarm
*/
void user_1ms_isr_type2(void) {

  /* Increment system counter through a 1ms alarm */
  (void)SignalCounter(SysTimerCnt); 

}

// ========================================================
// nxtOSEK Tasks 
// ========================================================


/** 
* @brief a task which manages the different states of the system
* 			 and alters them according to button presses or static
*				 variable changes.
* 
* @param ManageState name of task
*/
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
     
      line_found = 1;

      if (color != BLACK && line_found == 0) {

        set_both_motor_speed(-30,-30);

      }
      
      else if (color != LIGHT_BLUE && line_found == 0) {

        line_found=1;

      }

      else if (line_found == 1) {

        search_time++;

        if (search_time <= 140) {
        
          set_both_motor_speed(50,-20);

          if (search_time == 140) {

            if (color != LIGHT_BLUE && color != WHITE) {

              make_turn = RIGHT;
              motor_right = 0;
              motor_left = 0;
              line_side = FOLLOW_LEFT;
              state = FOLLOW_LINE;
              search_time = 0;
              black_count = 0;
              line_found = 0;

              change_position(ALTER);

            }
          }
        }

        else if (search_time <= 280) {

          set_both_motor_speed(-50,20);

        }

        else if (search_time <= 420) {

          set_both_motor_speed(-20, 50);

          if (search_time == 420) {

            if (color != LIGHT_BLUE && color != WHITE) {

              make_turn = LEFT;
              motor_right = 0;
              motor_left = 0;
              line_side = FOLLOW_RIGHT;
              state = FOLLOW_LINE;
              search_time = 0;
              line_found = 0;
              black_count = 0;

              change_position(ALTER);

            }  
          }
        }

        else if (search_time < 560) { 
        
          set_both_motor_speed(20,-50); 
      
        }

        else if (search_time < 590) {
          
          set_both_motor_speed(20,20);
          
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
      
      U8 switch_color;

      if (old_color == color) {

        switch_color = color;
      }

      if (switch_color == WHITE) {

        white_count++;
      }
      else { white_count = 0; }

      switch(switch_color) {

        case BLACK:
          robot_follow(FOLLOW, line_side);
          break;

        case WHITE:
          robot_follow(SEARCH, line_side);
          
          if (white_count > 30) {
          
            line_side = !line_side;
            white_count = 0;

          }
          
          break;

        case YELLOW: 

          set_map_junction(YELLOW); 

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

          checked[LEFT] = 0;
          checked[RIGHT] = 0;
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

          do {

            if (checked[FORWARD]) {

              state = FINISHED;
              break;

            }

          make_turn = FORWARD;

         } while ( !change_position(TEST));

         change_position(ALTER);

         checked[FORWARD] = 0;
         inc_map_value();

         if (state != FINISHED) {
           state = TURN;
         }

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
          robot_follow(UNSURE, line_side);
          break;
      }

      break;

    case TURN:

      check_color();

      if (old_color == color) {

        switch_color = color;

      }

      display_clear(0);
      display_goto_xy(0,0);
      display_string("TURNING");
      display_goto_xy(0,1);
      display_int(make_turn,1);
      display_update();
      
      if (line_found == 0 && color != BLACK && color != WHITE && color != UNKNOWN && make_turn != FORWARD) {

        set_both_motor_speed(-30,-30);
      
			}
      
      else {

        line_found=1;
      }

      if (line_found == 1) {
      
        switch(make_turn) {

          case RIGHT:
            //right
            set_both_motor_speed(45,-20);
            line_side = FOLLOW_LEFT;
	 				  break;
    
          case LEFT:
            //left
            set_both_motor_speed(-20,45);
            line_side = FOLLOW_RIGHT;
            break;
    
          case FORWARD:
            //forward 
           
            switch(switch_color) {
    
              case BLACK:
                robot_follow(FOLLOW, line_side);
                break;

              case WHITE:
                robot_follow(SEARCH, line_side);
                break;

              case UNKNOWN:
               robot_follow(UNSURE, line_side);
               break;
            }

            break;

        }

        if (color == BLACK || color == UNKNOWN) {
        
          black_count++;

          if (black_count > 2) {
        
            state = FOLLOW_LINE;
            line_found = 0;
            black_count = 0;

          }
        }
      }

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


/** 
* @brief Checks for buttons presses
* 
* @param ReadButtons name of task
*/
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


/** 
* @brief Keeps a constant line of communication with the colour sensor to
*				 allow instantaneous updates of the RGB values.
* 
* @param ReadColour
*/
TASK(ReadColour) {

  while(1)
  {
    // communicates with NXT Color Sensor (this must be executed repeatedly in a background Task)
    ecrobot_process_bg_nxtcolorsensor(); 
  }

}
