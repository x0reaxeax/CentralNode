#ifndef _CNODE_CURSESCTL_H_
#define _CNODE_CURSESCTL_H_

#include "gui.h"

/** TUI ncurses function macros */

/* Turn on echo and get user str input. Turns echo off after */
#define ECHO_MVGETNSTR(y, x, str, n)        { echo(); mvgetnstr((y), (x), (str), (n)); noecho();}
/* Turn on echo and cursor and get user str input. Turns off echo and cursor after */
#define CURSORECHO_MVGETNSTR(y, x, str, n)  { echo(); curs_set(1); mvgetnstr((y), (x), (str), (n)); curs_set(0); noecho();}
/* Clear specific terminal lines, containing user input */
#define CLEARUSERINFO                       wclearnline(LINES - 8, 0); wclearnline(LINES - 9, 0);
/* Refresh everything */
#define WREFRESHALL(nwin)                   refresh(); wrefresh(win);  

/**
 * @brief Clears output starting at given x,y coordinates to the on of line
 * Refreshes the window afterwards.
 * 
 * @param nwin  window to operate on
 * @param y     terminal y column
 * @param x     terminal x row
 */
void gui_wclearline(WINDOW *nwin, dim_t y, dim_t x);

/**
 * @brief Clear all output in a window.
 * Moves cursor to X:0, Y:0 and clears all output in a window to the bottom of screen.
 * Refreshes the window afterwards.
 * 
 * @param nwin  - ncurses window to operate on
 */
void gui_wclearscr(WINDOW *nwin);

/**
 * @brief Paint the terminal window with black background
 * 
 * @param nwin  mainwin
 * @param maxx  max screen x for loop painting
 * @param maxy  max screen y for loop painting
 */
void gui_paint_win_bg(WINDOW *nwin, dim_t maxx, dim_t maxy);

/**
 * @brief End main window end exit "GUI"
 * 
 * @param nwin      window to perform the surgery on
 */
void gui_killgui(WINDOW *nwin);

/**
 * @brief For printing MENU title in the middle of menu box
 * 
 * @param win       MENU window
 * @param starty    starty..
 * @param startx    startx..
 * @param width     width..
 * @param string    title text to print in the middle
 */
void menu_print_in_middle(WINDOW *win, int starty, int startx, int width, char *string);

/**
 * @brief Restores a color previously swapped with ncurses_swap_color() to it's predefined values
 * 
 * @param color_backup      pointer to backup struct with backed-up RGB values
 * @param color_to_restore  color ID to restore
 * @return EXIT_SUCCESS
 * @return EXIT_FAILURE  
 */
int ncurses_restore_color(struct color *color_backup, color_t color_to_restore);

/**
 * @brief Swaps default predefined ncurses color with custom values. Saves the original values for later restoration.
 * 
 * @param color_backup      pointer to color struct where original values are backed up
 * @param color_to_swap     color ID to swap
 * @param color_r           Red ncurses value
 * @param color_g           Green ncurses value
 * @param color_b           Blue ncurses value
 * @return EXIT_SUCCESS
 * @return EXIT_FAILURE  
 */
int ncurses_swap_color(struct color *color_backup, color_t color_to_swap, color_t color_r, color_t color_g, color_t color_b);

/**
 * @brief Applies given ncurses color values to provided color. 
 * 
 * @param nodecolor     color to alter
 * @param ncr           Red ncurses value
 * @param ncg           Green ncurses value
 * @param ncb           Blue ncurses value
 * @return EXIT_SUCCESS
 * @return EXIT_FAILURE  
 */
int ncurses_apply_ncolor(struct color *nodecolor, color_t ncr, color_t ncg, color_t ncb); 

/**
 * @brief Initializes ncurses mainwin, colors and color pairs
 * 
 * @param mainwin       main gui win
 * @param nodecolors    cnode color struct
 * @return EXIT_SUCCESS
 * @return EXIT_FAILURE  
 */
int ncurses_init_gui(WINDOW **mainwin, struct color nodecolors[CLR_USR_MAX]);

#endif