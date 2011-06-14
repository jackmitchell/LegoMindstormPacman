/** 
* @file functions.c
* @brief Functions for controlling robot
* with Area Mapping and Decision Making Abilities
* @author Jack Mitchell
* @date 2011-02-21
*/

#include "kernel.h"
#include "kernel_id.h"
#include "ecrobot_interface.h"

#include "defines.h"
#include "functions.h"

/** 
* @brief Set both motor speeds in one easy function 
* 
* @param left		left motor speed	
* @param right	right motor speed
*/
void set_both_motor_speed(S8 left, S8 right) {

    nxt_motor_set_speed(MOTOR_RIGHT, right, 1);
    nxt_motor_set_speed(MOTOR_LEFT, left, 1);

}

/** 
* @brief Sets the upper and lower motor speed thresholds 
* 
* @param l_min left motor lower
* @param l_max left motor upper
* @param r_min right motor lower
* @param r_max right motor upper
*/
void set_motor_threshold(U8 l_min, U8 l_max, U8 r_min, U8 r_max) {

	if (motor_left < l_min) {	motor_left = l_min; }

	if (motor_left > l_max) { motor_left = l_max; }

	if (motor_right < r_min) { motor_right = r_min; }

	if (motor_right > r_max) { motor_right = r_max; }

}

/** 
* @brief Query if it is viable to make a turn according to Tremauxes' Algorithm
* 
* @param x proposed x co-ordinate
* @param y proposed y co-ordinate
* 
* @return viable move according to Tremauxes' Algorithm
*/
int query_turn(U8 x, U8 y) {

  // Check turn
  
  if ( map[x][y] != 2) {
  
    return true;

  }

  else { return false; }

}

void set_map_junction(U8 junction) {

  map[x_position][y_position] = junction++;

}

void inc_map_value() {

  map[x_position][y_position] = map[x_position][y_position]++;

}

void clear_checked_turns() {

	checked[FORWARD] = 0;
	checked[LEFT] = 0;
	checked[RIGHT] = 0;

}

/** 
* @brief Change the position of the robot in the map array
* 
* @param check if this is a dry run of an actual change of position
* 
* @return viable move according to Tremauxes' Algorithm 
*/
int change_position(U8 check) {

  U8 temp_x_pos;
  U8 temp_y_pos;
  U8 temp_orientation;

  temp_orientation = orientation;
  temp_x_pos = x_position;
  temp_y_pos = y_position;

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

	/* Check if we are testing the position or altering the position 
	and that the move is valid according to the algorithm */
  
  if (state == FINISHED) {

    return true;

  }
  
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


/** 
* @brief sets the motor speeds according to the current colour detected
* 
* @param direction what mode the robot is in
*/
void robot_follow(U8 direction) {

  U16 light; 
  light = colorCal[LIGHT][0] - 35;

  /* Set motor speeds depending on direction */
  switch(direction) {

   case FOLLOW:

      if (light_value < light) {

        motor_right = 25;
        motor_left = 25;

      }

      else {

        motor_right = 25;
        motor_left = 30;

      }

      break;

    case SEARCH:

      if (light_value < light) {

        motor_right = 50;
        motor_left = 30;

      }

      else {

        motor_right = 30;
        motor_left = 50;

      }

      break;

    case UNSURE:

      if (light_value < light ) {
    
        motor_right = 25;
        motor_left = 24;
      }

      else {

        motor_right = 25;
        motor_left = 50;
    
      }
      
      break;

    default:
    
      //change nothing
      break;

  } 

  /* set motor speeds */
  set_both_motor_speed(motor_left,motor_right);

}


/** 
* @brief queries the colour sensor and sets the global colour variables
*/
void check_color() {

  S16 rgb[3];
  
  light_value = ecrobot_get_light_sensor(LIGHT2_PORT);

  display_goto_xy(0,5);
  display_string("X:");
  display_goto_xy(1,5);
  display_int(x_position,1);
  display_goto_xy(0,6);
  display_string("Y:");
  display_goto_xy(1,6);
  display_int(y_position,1);
  display_goto_xy(4,5);
  display_string("COUNT:");
  display_goto_xy(10,5);
  display_int(map[x_position][y_position],2);
  display_goto_xy(4,6);
  
  switch(orientation) {

    case NORTH:

      display_string("NORTH");
      break;

    case EAST:

      display_string("EAST");
      break;

    case SOUTH:

      display_string("SOUTH");
      break;

    case WEST:

      display_string("WEST");
      break;

  }

  display_update();

  /* get RGB values from sensor */
  ecrobot_get_nxtcolorsensor_rgb(LIGHT_PORT, rgb);
  
  S16 red = rgb[0];
  S16 green = rgb[1];
  S16 blue = rgb[2];

  /* Keep information on the last known color */
  //if ( color != UNKNOWN ) {
  old_color = color;
  //}

  color = UNKNOWN;

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
