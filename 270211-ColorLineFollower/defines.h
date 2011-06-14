/**
 * defines.h
 *
 * This file details all the defines used throughout the code.
 *
 * Jack Mitchell <jgm11@le.ac.uk>
 *
 **/

#ifndef _DEFINES_H_
#define	_DEFINES_H_

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
#define FOLLOW_RIGHT  (0)
#define FOLLOW_LEFT   (1)

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

#endif
