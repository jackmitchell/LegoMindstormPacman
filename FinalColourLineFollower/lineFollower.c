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

  /* Initialise light sensor */
  ecrobot_set_light_sensor_active(LIGHT2_PORT);

}


/** 
* @brief code to run when the program is terminated
*/
void ecrobot_device_terminate(void) {

  /* Turn off motors */
  set_both_motor_speed(0,0);

  /* Terminate the I2C bus for color sensor */
  ecrobot_term_nxtcolorsensor(LIGHT_PORT);

  /* Terminate the I2C bus for light sensor */
  ecrobot_set_light_sensor_inactive(LIGHT2_PORT);

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
     
      search_time++;

      if (search_time < PIVOT_TIME) {

        set_both_motor_speed(20,20);

      }

      else if (search_time <= (SEARCH_TIME+PIVOT_TIME)) {
        
        set_both_motor_speed(25,-25);

        if (search_time > (SEARCH_TIME+50) || search_time < (SEARCH_TIME+PIVOT_TIME)) {

          if (color == BLACK && light_value < colorCal[LIGHT][0]) {

            make_turn = RIGHT;
            change_position(ALTER);
            state = FOLLOW_LINE;
            search_time = 0;

          }
        }
      }

      else if (search_time <= ((SEARCH_TIME*2)+PIVOT_TIME)) {

        set_both_motor_speed(-25,25);

      }

      else if (search_time <= ((SEARCH_TIME*3)+PIVOT_TIME)) {

        set_both_motor_speed(-25, 25);

        if (search_time > ((SEARCH_TIME*3)+50) || search_time < (SEARCH_TIME+PIVOT_TIME)) {

          if (color == BLACK && light_value < colorCal[LIGHT][0]) {

            make_turn = LEFT;
            change_position(ALTER);
            state = FOLLOW_LINE;
            search_time = 0;

          }  
        }
      }

      else if (search_time < ((SEARCH_TIME*4)+PIVOT_TIME)) { 
        
        set_both_motor_speed(25,-25); 
      
      }

      else if (search_time < ((SEARCH_TIME*4)+PIVOT_TIME+((PIVOT_TIME/4)*3))) {
          
        set_both_motor_speed(-20,-20);
          
      }
        
      else { search_time = 0; }

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
      
      switch(color) {

        case BLACK:
          
          robot_follow(FOLLOW);
          break;

        case WHITE:
          
          robot_follow(SEARCH);
          break;

        case YELLOW: 

          set_map_junction(YELLOW); 

          do {
              
            random = rand()%(3-1)+1;

            switch(orientation) {
            
              case NORTH:

                if (checked[FORWARD] && checked[RIGHT]) {

                  state = FINISHED;
                  break;

                }
              
                switch(random) {

                  case 1:

                    make_turn = FORWARD;
                    break;
      
                  case 2:
      
                    make_turn = RIGHT;
                    break;

                }

                break;

              case SOUTH:

                if (checked[LEFT] && checked[RIGHT]) {

                  state = FINISHED;
                  break;

                }
                
                switch(random) {

                  case 1:

                    make_turn = LEFT;
                    break;

                  case 2:

                    make_turn = FORWARD;
                    break;

                }

                break;

              case WEST:

                if (checked[LEFT] && checked[RIGHT]) {

                  state = FINISHED;
                  break;

                }

                switch(random) {

                  case 1:

                    make_turn = LEFT;
                    break;

                  case 2:

                    make_turn = RIGHT;
                    break;

                }

                break;

            }

          } while ( !change_position(TEST));

          clear_checked_turns();

					if ( state != FINISHED ) {
						
            state = TURN;
            change_position(ALTER);

          }

          break;

				case ORANGE:

					set_map_junction(ORANGE);

					do {

						random = rand()%(3-1)+1;

						switch (orientation) {
            
              case NORTH:

                if (checked[LEFT] && checked[FORWARD]) {

                  state = FINISHED;
                  break;

                }
              
                switch(random) {

	  				  		case 1:

		  				  		make_turn = LEFT;
			  				  	break;

  			  				case 2:

	  			  				make_turn = FORWARD;
		  			  			break;

			  			  }

                break;

              case SOUTH:

                if (checked[RIGHT] && checked[FORWARD]) {

                  state = FINISHED;
                  break;

                }

                switch(random) {

                  case 1:

                    make_turn = RIGHT;
                    break;

                  case 2:

                    make_turn = FORWARD;
                    break;

                }

                break;

              case EAST:

                if (checked[LEFT] && checked[RIGHT]) {

                  state = FINISHED;
                  break;

                }

                switch(random) {

                  case 1:

                    make_turn = LEFT;
                    break;

                  case 2:

                    make_turn = RIGHT;
                    break;

                }

                break;

            }

					} while ( !change_position(TEST));

					clear_checked_turns();
					
					if (state != FINISHED) {
				
            state = TURN;
            change_position(ALTER);

					}

					break;

        case RED:

					set_map_junction(RED);

					do {

	          random = rand()%(3-1)+1;

  	        switch(orientation) {

							case NORTH:

                if (checked[LEFT] && checked[RIGHT]) {

                  state = FINISHED;
                  break;

                }
					
								switch(random) {

    			        case 1:

        			      make_turn = LEFT;
            			  break;

		          	  case 2:
	
  		          	  make_turn = RIGHT;
      		        	break;

	          		}

								break;

							case EAST:

                if (checked[FORWARD] && checked[RIGHT]) {

                  state = FINISHED;
                  break;

                }
	
								switch(random) {

									case 1:

										make_turn = FORWARD;
										break;

									case 2:

										make_turn = RIGHT;
										break;
	
								}

								break;

							case WEST:

                if (checked[LEFT] && checked[FORWARD]) {

                  state = FINISHED;
                  break;

                } 

								switch(random) {

									case 1:

										make_turn = LEFT;
										break;
	
									case 2:

										make_turn = FORWARD;
										break;

								}

								break;

						}

					} while ( !change_position(TEST));

          clear_checked_turns();
					
          if ( state != FINISHED ) {
            
            state = TURN;
            change_position(ALTER);

          }

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

         clear_checked_turns();
         inc_map_value();

         if (state != FINISHED) {

           state = TURN;

         }

         break;

        case BLUE:

				  set_map_junction(BLUE);

					do {

						if (checked[FORWARD] && checked[LEFT] && checked[RIGHT]) {

							state = FINISHED;
							break;

						}

            random=rand()%(4-1)+1;

            switch(random) {

     	        case 1:

       	        make_turn = FORWARD;
                break;

              case 2:

                make_turn = LEFT;
                break;

	  	  			case 3:

  			  			make_turn = RIGHT;
                break;

            }

					} while ( !change_position(TEST));

          if (state != FINISHED) {
						state = TURN;
					}

					change_position(ALTER);
					checked[FORWARD] = 0;
					checked[LEFT] = 0;
					checked[RIGHT] = 0;

          break;

        case LIGHT_BLUE:

					set_map_junction(LIGHT_BLUE);

          state = SEARCH_CORNER;  
          break;

        case UNKNOWN:
         
				 	robot_follow(UNSURE);
          break;
      }

      break;

    case TURN:

      check_color();

      display_clear(0);
      display_goto_xy(0,0);
      display_string("TURNING");
      display_goto_xy(0,2);
      
      search_time++;

      if (search_time < PIVOT_TIME && make_turn != FORWARD) {

        set_both_motor_speed(20,20);

      }

      else {

        switch(make_turn) {

          case RIGHT:
            //right
            set_both_motor_speed(20,-20);
            display_string("RIGHT");
            break;
    
          case LEFT:
            //left
            set_both_motor_speed(-20,20);
            display_string("LEFT");
            break;
    
          case FORWARD:
            //forward 
           
            display_string("FORWARD");
            switch(color) {
    
              case BLACK:
                robot_follow(FOLLOW);
                break;

              case WHITE:
                robot_follow(SEARCH);
                break;

              case UNKNOWN:
                robot_follow(UNSURE);
                break;
            }

            break;

        }

      }
  
      display_update();
      
      if (make_turn != FORWARD) {
      
        if (color == BLACK && light_value < colorCal[LIGHT][0] && search_time > 300) {

          search_time = 0;
          state = FOLLOW_LINE;

        }

      }

      else {

        if (color == BLACK || color == UNKNOWN || color == WHITE) {

          state = FOLLOW_LINE;
          black_count = 0;
          search_time = 0;

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

          case ORANGE:
            display_string("PINK");
            break;

          case LIGHT:
            display_string("LIGHT (WHITE)");
            break;

          default:
            display_string("NOT SET");
            break;
      
        }

        S16 rgb[3];
        ecrobot_get_nxtcolorsensor_rgb(LIGHT_PORT, rgb);

        display_goto_xy(0,4);
        display_string("============");
        display_goto_xy(0,5);
        display_string("R:");
        display_goto_xy(2,5);
        display_int(rgb[0],3);
        display_goto_xy(0,6);
        display_string("G:");
        display_goto_xy(2,6);
        display_int(rgb[1],3);
        display_goto_xy(0,7);
        display_string("B:");
        display_goto_xy(2,7);
        display_int(rgb[2],3);

        if (enter_button == 3) {

          if (color_cal_state != LIGHT) {
            
            U8 variation = 0;

            // Give black and white states less leway to the colours
            // for accurate line following
            if (color_cal_state == BLACK || color_cal_state == WHITE) {

              variation = 10;

            }

            else {

              variation = 40;

            }


            
            colorCal[color_cal_state][0] = (rgb[0]-variation);
            colorCal[color_cal_state][1] = (rgb[0]+variation);
            colorCal[color_cal_state][2] = (rgb[1]-variation);
            colorCal[color_cal_state][3] = (rgb[1]+variation);
            colorCal[color_cal_state][4] = (rgb[2]-variation);
            colorCal[color_cal_state][5] = (rgb[2]+variation);

          }

          else {

            light_value = ecrobot_get_light_sensor(LIGHT2_PORT);
            colorCal[color_cal_state][0] = light_value+40;

          }

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
