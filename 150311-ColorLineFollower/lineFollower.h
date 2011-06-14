/**
 *
 * lineFollower.h
 *
 * Holds static variables
 *
 * Jack Mitchell <jgm11@le.ac.uk>
 *
 **/

#ifndef _LINEFOLLOWER_H_
#define _LINEFOLLOWER_H_

U8 state = 0;
U8 old_color;
U8 color_cal_state = 0;
U8 color;

U32 enter_button = 0;
U32 run_button = 0;

S16 colorCal[8][6];

U8 random = 0;
U8 line_side = FOLLOW_LEFT;
U8 make_turn;
U8 line_found;
U8 time_lost = 1;

U8 checked[6];

U16 search_time = 0;
U16 black_count = 0;
U16 white_count = 0;

extern U16 light_value;
extern U8 x_position;
extern U8 y_position;
extern U8 map[100][100];

#endif
