/* 
 * tab:4
 *
 * maze.h - maze generation and display header file
 *
 * "Copyright (c) 2004-2009 by Steven S. Lumetta."
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without written agreement is
 * hereby granted, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 * 
 * IN NO EVENT SHALL THE AUTHOR OR THE UNIVERSITY OF ILLINOIS BE LIABLE TO 
 * ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL 
 * DAMAGES ARISING OUT  OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, 
 * EVEN IF THE AUTHOR AND/OR THE UNIVERSITY OF ILLINOIS HAS BEEN ADVISED 
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * THE AUTHOR AND THE UNIVERSITY OF ILLINOIS SPECIFICALLY DISCLAIM ANY 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE 
 * PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND NEITHER THE AUTHOR NOR
 * THE UNIVERSITY OF ILLINOIS HAS ANY OBLIGATION TO PROVIDE MAINTENANCE, 
 * SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS."
 *
 * Author:        Steve Lumetta
 * Version:       2
 * Creation Date: Thu Sep  9 22:09:54 2004
 * Filename:      maze.h
 * History:
 *    SL    1    Thu Sep  9 22:09:54 2004
 *        First written.
 *    SL    2    Sat Sep 12 14:01:15 2009
 *        Integrated original release back into main code base.
 */

#ifndef MAZE_H
#define MAZE_H

#include "blocks.h"
#include "modex.h"

#define SHOW_MIN       6  /* hide the last six pixels of boundary */

/*
 * Define maze minimum and maximum dimensions.  The description of make_maze
 * in maze.c gives details on the layout of the maze.  Minimum values are
 * chosen to ensure that a maze fills the scrolling region of the screen.
 * Maximum values are somewhat arbitrary.
 */
#define MAZE_MIN_X_DIM ((SCROLL_X_DIM + (BLOCK_X_DIM - 1) + 2 * SHOW_MIN) / (2 * BLOCK_X_DIM))
#define MAZE_MAX_X_DIM 50
#define MAZE_MIN_Y_DIM ((SCROLL_Y_DIM + (BLOCK_Y_DIM - 1) + 2 * SHOW_MIN) / (2 * BLOCK_Y_DIM))
#define MAZE_MAX_Y_DIM 30

/* bit vector of properties for spaces in the maze */
typedef enum {
    MAZE_NONE           = 0,    /* empty                                    */
    MAZE_WALL           = 1,    /* wall                                     */
    MAZE_FRUIT_1        = 2,    /* fruit (3-bit field, with 0 for no fruit) */
    MAZE_FRUIT_2        = 4,
    MAZE_FRUIT_3        = 8,  
    LAST_MAZE_FRUIT_BIT = MAZE_FRUIT_3,
    MAZE_FRUIT          = (MAZE_FRUIT_1 | MAZE_FRUIT_2 | MAZE_FRUIT_3),
    MAZE_EXIT           = 16,   /* exit from maze                           */
    MAZE_REACH          = 128   /* seen already (not shrouded in mist)      */
} maze_bit_t;

/* create a maze and place some fruits inside it */
extern int make_maze(int x_dim, int y_dim, int start_fruits);

/* fill a buffer with the pixels for a horizontal line of the maze */
extern void fill_horiz_buffer(int x, int y, unsigned char buf[SCROLL_X_DIM]);

/* fill a buffer with the pixels for a vertical line of the maze */
extern void fill_vert_buffer(int x, int y, unsigned char buf[SCROLL_Y_DIM]);

/* mark a maze location as reached and draw it onto the screen if necessary */
extern void unveil_space(int x, int y);

/* consume fruit at a space, if any; returns the fruit number consumed */
extern int check_for_fruit(int x, int y);

/* check whether the player has reached the exit with no fruits left */
extern int check_for_win(int x, int y);

/* add a new fruit randomly in the maze */
extern int add_a_fruit();

/* get pointer to the player's block image; depends on direction of motion */
extern unsigned char* get_player_block(dir_t cur_dir);

/* get pointer to the player's mask block; depends on direction of motion*/
extern unsigned char* get_mask(dir_t cur_dir);

/* get pointer to the player's mask image; depends on direction of motion */
extern unsigned char* get_player_mask(dir_t cur_dir);

/* determine which directions are open to movement from a given maze point */
extern void find_open_directions(int x, int y, int op[NUM_DIRS]);


/*
 * get_num_fruit
 *   DESCRIPTION: return the number of fruit
 *   INPUTS:  none
 *   OUTPUTS: none
 *   SIDE EFFECTS: none
 */
extern int get_num_fruit();

#endif /* MAZE_H */
