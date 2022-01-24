#ifndef _CNODE_GUI_H_
#define _CNODE_GUI_H_

/*
 * Includes NCURSES headers into Central Node.
 * NCURSES TUI/GUI is the default way of launching when included.
*/

#include <ncurses.h>

/* Terminal screen x,y coordinates type */
typedef unsigned short dim_t;
/* ncurses color type */
//typedef unsigned short color_t;
typedef short color_t;

#define TERM_MIN_X          120U                    /* minimal supported terminal horizontal resolution (x - row) */
#define TERM_MIN_Y          28U                     /* minimal supported terminal vertical resolution (y - column) */

/* RGB color struct */
struct color {                                      /* Color struct for RGB values of individual colors */
    color_t red;
    color_t green;
    color_t blue;
};

/*
 *  Terminals usually have 16 default colors, which we don't want to alter.
 *  Therefore we will define all color IDs to (COLOR_MIN + COLOR_ID) 
*/
#define ID_COLOR_MIN        17

#define CLR_USR_MAX         16                      /* Max number of user defined colors */
#define CLR_BG_DEFAULT      10                      /* Default terminal background color */
#define CLR_MAX_VAL         1000                    /* max color value */

/* For custom "extra" color pairs */
#define NC_PAIR_EXTRA       (ID_COLOR_MIN + CLR_USR_MAX)


/*
 * ncurses color IDs
 * 0-7 are default ncurses-defined colors.
 * 8-14 are custom defined colors.
*/
#define ID_COLOR_BLACK      (ID_COLOR_MIN + 0) 
#define ID_COLOR_RED        (ID_COLOR_MIN + 1) 
#define ID_COLOR_GREEN      (ID_COLOR_MIN + 2) 
#define ID_COLOR_YELLOW     (ID_COLOR_MIN + 3) 
#define ID_COLOR_BLUE       (ID_COLOR_MIN + 4) 
#define ID_COLOR_MAGENTA    (ID_COLOR_MIN + 5) 
#define ID_COLOR_CYAN       (ID_COLOR_MIN + 6) 
#define ID_COLOR_WHITE      (ID_COLOR_MIN + 7) 
#define ID_COLOR_PURPLE     (ID_COLOR_MIN + 8) 
#define ID_COLOR_BRICKRED   (ID_COLOR_MIN + 9) 
#define ID_COLOR_ORANGE     (ID_COLOR_MIN + 10)
#define ID_COLOR_RGBLOOP    (ID_COLOR_MIN + 11)     /* This is a variable based color, meaning it is never a specific color, rather a loop of changing RGB values. */
#define ID_COLOR_DARKBLACK  (ID_COLOR_MIN + 12)
#define ID_COLOR_INVLGRAY   (ID_COLOR_MIN + 13)
#define ID_COLOR_WHITERST   (ID_COLOR_MIN + 14)     /* possibly to-remove */

#define COLOR_ARR_ID(COLOR) (COLOR - ID_COLOR_MIN)  /* for accessing color struct arrays */
#define CLR_NDEF_COLORS     (COLOR_ARR_ID(ID_COLOR_WHITERST) + 1) /* Number of user defined colors. Has to be last defined color + 1 ) */


/* ncurses color pairs IDs
 * from start_color() manpage: 
 * start_color() initializes the special color pair 0 to the default foreground and background colors.
 * No other color pairs are initialized. 
 * We will initialize each color pair to it's respective defined color ID.
 * Extra (special) pairs will be initialized incrementally with NC_PAIR_EXTRA
*/ 

#define ID_PAIR_BLACK       ID_COLOR_BLACK
#define ID_PAIR_RED         ID_COLOR_RED
#define ID_PAIR_GREEN       ID_COLOR_GREEN
#define ID_PAIR_YELLOW      ID_COLOR_YELLOW
#define ID_PAIR_BLUE        ID_COLOR_BLUE
#define ID_PAIR_MAGENTA     ID_COLOR_MAGENTA
#define ID_PAIR_CYAN        ID_COLOR_CYAN
#define ID_PAIR_WHITE       ID_COLOR_WHITE
#define ID_PAIR_PURPLE      ID_COLOR_PURPLE
#define ID_PAIR_BRICKRED    ID_COLOR_BRICKRED
#define ID_PAIR_ORANGE      ID_COLOR_ORANGE
#define ID_PAIR_RGBLOOP     ID_COLOR_RGBLOOP
#define ID_PAIR_DARKBLACK   ID_COLOR_DARKBLACK
#define ID_PAIR_INVLGRAY    ID_COLOR_INVLGRAY
#define ID_PAIR_RED_INV     (NC_PAIR_EXTRA + 1)
#define ID_PAIR_RESET       NULL                    // unused - remove

#define PAIR_NDEF_PAIRS     (15)   /* number of user defined pairs, manual for now */

/* Macros for enabling and disabling color pairs */
#define PAIRON_COLOR_BLACK(nwin)      wattron(nwin, COLOR_PAIR(ID_PAIR_BLACK));
#define PAIRON_COLOR_RED(nwin)        wattron(nwin, COLOR_PAIR(ID_PAIR_RED));
#define PAIRON_COLOR_GREEN(nwin)      wattron(nwin, COLOR_PAIR(ID_PAIR_GREEN));
#define PAIRON_COLOR_YELLOW(nwin)     wattron(nwin, COLOR_PAIR(ID_PAIR_YELLOW));
#define PAIRON_COLOR_BLUE(nwin)       wattron(nwin, COLOR_PAIR(ID_PAIR_BLUE));
#define PAIRON_COLOR_MAGENTA(nwin)    wattron(nwin, COLOR_PAIR(ID_PAIR_MAGENTA));
#define PAIRON_COLOR_CYAN(nwin)       wattron(nwin, COLOR_PAIR(ID_PAIR_CYAN));
#define PAIRON_COLOR_WHITE(nwin)      wattron(nwin, COLOR_PAIR(ID_PAIR_WHITE));
#define PAIRON_COLOR_PURPLE(nwin)     wattron(nwin, COLOR_PAIR(ID_PAIR_PURPLE));
#define PAIRON_COLOR_BRICKRED(nwin)   wattron(nwin, COLOR_PAIR(ID_PAIR_BRICKRED));
#define PAIRON_COLOR_ORANGE(nwin)     wattron(nwin, COLOR_PAIR(ID_PAIR_ORANGE));
#define PAIRON_COLOR_RGBLOOP(nwin)    wattron(nwin, COLOR_PAIR(ID_PAIR_RGBLOOP));
#define PAIRON_COLOR_DARKBLACK(nwin)  wattron(nwin, COLOR_PAIR(ID_PAIR_DARKBLACK));
#define PAIRON_COLOR_INVLGRAY(nwin)   wattron(nwin, COLOR_PAIR(ID_PAIR_INVLGRAY));
#define PAIRON_COLOR_INVRED(nwin)     wattron(nwin, COLOR_PAIR(ID_PAIR_RED_INV));


#define PAIROFF_COLOR_BLACK(nwin)      wattroff(nwin, COLOR_PAIR(ID_PAIR_BLACK));
#define PAIROFF_COLOR_RED(nwin)        wattroff(nwin, COLOR_PAIR(ID_PAIR_RED));
#define PAIROFF_COLOR_GREEN(nwin)      wattroff(nwin, COLOR_PAIR(ID_PAIR_GREEN));
#define PAIROFF_COLOR_YELLOW(nwin)     wattroff(nwin, COLOR_PAIR(ID_PAIR_YELLOW));
#define PAIROFF_COLOR_BLUE(nwin)       wattroff(nwin, COLOR_PAIR(ID_PAIR_BLUE));
#define PAIROFF_COLOR_MAGENTA(nwin)    wattroff(nwin, COLOR_PAIR(ID_PAIR_MAGENTA));
#define PAIROFF_COLOR_CYAN(nwin)       wattroff(nwin, COLOR_PAIR(ID_PAIR_CYAN));
#define PAIROFF_COLOR_WHITE(nwin)      wattroff(nwin, COLOR_PAIR(ID_PAIR_WHITE));
#define PAIROFF_COLOR_PURPLE(nwin)     wattroff(nwin, COLOR_PAIR(ID_PAIR_PURPLE));
#define PAIROFF_COLOR_BRICKRED(nwin)   wattroff(nwin, COLOR_PAIR(ID_PAIR_BRICKRED));
#define PAIROFF_COLOR_ORANGE(nwin)     wattroff(nwin, COLOR_PAIR(ID_PAIR_ORANGE));
#define PAIROFF_COLOR_RGBLOOP(nwin)    wattroff(nwin, COLOR_PAIR(ID_PAIR_RGBLOOP));
#define PAIROFF_COLOR_DARKBLACK(nwin)  wattroff(nwin, COLOR_PAIR(ID_PAIR_DARKBLACK));
#define PAIROFF_COLOR_INVLGRAY(nwin)   wattroff(nwin, COLOR_PAIR(ID_PAIR_INVLGRAY));
#define PAIROFF_COLOR_INVRED(nwin)     wattroff(nwin, COLOR_PAIR(ID_PAIR_RED_INV));

#endif