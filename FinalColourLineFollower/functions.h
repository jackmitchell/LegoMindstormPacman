/**
 * functions.h
 *
 * Header file defining functions
 *
 * Jack Mitchell <jgm11@le.ac.uk>
 *
 **/

#ifndef _FUNCTIONS_H_
#define _FUNCTIONS_H_

#include "kernel.h"
#include "kernel_id.h"
#include "ecrobot_interface.h"

/* Setting static variables */

extern U8 make_turn;
extern U8 checked[6];
extern U8 old_color;
extern U8 color;
extern S16 colorCal[8][6];
extern U8 state;

S8 motor_right;
S8 motor_left;

U8 map[100][100];
U8 x_position = 50;
U8 y_position = 50;
U8 orientation = NORTH;

U16 light_value;

/* Setting functions */

extern void inc_map_value();
extern void set_both_motor_speed(S8 left, S8 right);
extern void set_motor_threshold(U8 l_min, U8 l_max, U8 r_min, U8 r_max);
extern int query_turn(U8 x, U8 y);
extern int change_position(U8 check);
extern void robot_follow(U8 direction);
extern void check_color();
extern void set_map_junction(U8 junction);
extern void	clear_checked_turns();

#endif


