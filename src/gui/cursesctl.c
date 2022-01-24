/** ncurses terminal control functions */
#include "../base/cnode.h"      /* cnode base */
#include "gui.h"

#include <string.h>             /* strlen */

void gui_wclearscr(WINDOW *nwin) {
    wmove(nwin, 0, 0);
    wclrtobot(nwin);
    wrefresh(nwin);
}

void gui_wclearline(WINDOW *nwin, dim_t y, dim_t x) {
    wmove(nwin, y, x);
    wclrtoeol(nwin);
    wrefresh(nwin);
}

/**
 * @brief Clears multiple lines in a window.
 * Lines are specified with, dim_t max, dim_t min
 * 
 * @param nwin      window to operate on
 * @param max       first line to clear
 * @param min       last line to clear
 */
void gui_wclearlines(WINDOW *nwin, dim_t max, dim_t min) {
    for (dim_t i = min; i <= max ; i++) {
        gui_wclearline(nwin, i, 0);
    }

    wrefresh(nwin);
}

void menu_print_in_middle(WINDOW *win, int starty, int startx, int width, char *string) {
    int length, x, y;
	float temp;

	if(win == NULL)
		win = stdscr;
	getyx(win, y, x);
	if(startx != 0)
		x = startx;
	if(starty != 0)
		y = starty;
	if(width == 0)
		width = 80;

	length = strlen(string);
	temp = (width - length)/ 2;
	x = startx + (int)temp;
	mvwprintw(win, y, x, "%s", string);
	wrefresh(win);
}

void gui_killgui(WINDOW *nwin) {
    echo();         /* Restore echo */
    curs_set(1);    /* Restore cursor visibility */
    delwin(nwin);   /* Delete main window */
    endwin();       /* End main window */
    refresh();      /* Last-ever refresh */
}

void gui_paint_win_bg(WINDOW *nwin, dim_t maxx, dim_t maxy) {
    PAIRON_COLOR_BLACK(nwin);
    move(0, 0);
    for (int i = 0; i < maxy; i++) {
        for (int j = 0; j < maxx; j++) {
            waddch(nwin, 0x20); /* whitespace */
        }
    }
    PAIROFF_COLOR_BLACK(nwin);
}

/**
 * @brief Converts RGB to ncurses color values and applies it to provided color. 
 * 
 * @param nodecolor     color to alter
 * @param r             Red value
 * @param g             Green value
 * @param b             Blue value
 * @return EXIT_SUCCESS
 * @return EXIT_FAILURE
 */
int ncurses_apply_rgb(struct color *nodecolor, color_t r, color_t g, color_t b) {
    /* rgb = x / 1000 * 255 */
    /* x = rgb * 1000 / 255 */
    if (nodecolor == NULL) {
        NODE_SETLASTERR(CURRENT_ADDR, NODE_NOINFO, ENULLPTR);
        log_write_trace("ncurses_apply_rgb()", "nodecolor");
        return EXIT_FAILURE;
    }

    nodecolor->red      = (r * CLR_MAX_VAL) / 255;
    nodecolor->green    = (g * CLR_MAX_VAL) / 255;
    nodecolor->blue     = (b * CLR_MAX_VAL) / 255;

    return EXIT_SUCCESS;
}

int ncurses_apply_ncolor(struct color *nodecolor, color_t ncr, color_t ncg, color_t ncb) {
    if (nodecolor == NULL) {
        NODE_SETLASTERR(CURRENT_ADDR, NODE_NOINFO, ENULLPTR);
        log_write_trace("ncurses_apply_ncolor()", "nodecolor");
        return EXIT_FAILURE;
    }

    nodecolor->red      = ncr;
    nodecolor->green    = ncg;
    nodecolor->blue     = ncb;

    return EXIT_SUCCESS;
}

int ncurses_swap_color(struct color *color_backup, color_t color_to_swap, color_t color_r, color_t color_g, color_t color_b) {
    if (color_backup == NULL) {
        NODE_SETLASTERR(CURRENT_ADDR, NODE_NOINFO, ENULLPTR);
        log_write_trace("ncurses_swap_colors()", "color_backup");
        return EXIT_FAILURE;
    }

    /* save color rgb, so it can be restored later */
    /* from manpages: All  other  routines  return the integer ERR upon failure and an OK (SVr4 specifies only “an integer value
       other than ERR”) upon successful completion.
    */
    if (color_content(color_to_swap, &color_backup->red, &color_backup->green, &color_backup->blue) == ERR) {
        NODE_SETLASTERR(CURRENT_ADDR, NODE_NOINFO, ECLRSAVE);
        //log_write(LOG_ERROR, "Unable to save predefined color values");
        log_write_trace("ncurses_swap_colors()", "color_content()");
        return EXIT_FAILURE;
    }

    if (init_color(color_to_swap, color_r, color_g, color_b) == ERR) {
        NODE_SETLASTERR(CURRENT_ADDR, NODE_NOINFO, EINITCLR);
        log_write(LOG_ERROR, "Unable to initialize color ID %d", color_to_swap);
        log_write(LOG_DEBUG, "init_color(%d, %4hd, %4hd, %4hd)", color_to_swap, color_r, color_g, color_b);
        log_write_trace("ncurses_swap_color()", "init_color()");
        return EXIT_FAILURE;
    }

    log_write(LOG_DEBUG, "Swapped default color ID %d with custom values: R: %4hd G: %4hd B: %4hd", color_to_swap, color_r, color_g, color_b);

    return EXIT_SUCCESS;
}

int ncurses_restore_color(struct color *color_backup, color_t color_to_restore) {
    if (color_backup == NULL) {
        NODE_SETLASTERR(CURRENT_ADDR, NODE_NOINFO, ENULLPTR);
        log_write_trace("ncurses_restore_color()", "color_backup");
        return EXIT_FAILURE;
    }

    if (init_color(color_to_restore, color_backup->red, color_backup->green, color_backup->blue) == ERR) {
        NODE_SETLASTERR(CURRENT_ADDR, NODE_NOINFO, EINITCLR);
        log_write(LOG_ERROR, "Unable to restore color ID %d, some terminals may look funky because of this", color_to_restore);
        log_write(LOG_DEBUG, "init_color(%d, %hd, %hd, %hd)", color_to_restore, color_backup->red, color_backup->green, color_backup->blue);
        log_write_trace("ncurses_restore_color()", "init_color()");
        return EXIT_FAILURE;
    }

    log_write(LOG_DEBUG, "Restored default color ID %d to predefined values: R: %4hd G: %4hd B: %4hd", color_to_restore, color_backup->red, color_backup->green, color_backup->blue);


    return EXIT_SUCCESS;
}

#ifdef __FUTURE_IS_NOW
/* 
 * This function is supposed to restore default color values for `COLOR_WHITE` and `COLOR_BLACK`,
 * just like it's done in `menu_loadmenu()`, so when switching from curses to terminal mode, 
 * original color values could be switched too. Although I'm too lazy to implement this now, since it's completely
 * a visual only function and `menu_execoption()` would need to accept variadic arguments for pointers, 
 * in order to pass the saved values. So this is an idea for the future.
*/ 
int ncurses_pause_curses(WINDOW *main_window, struct color *default_color) {
    if (NULL == default_color || NULL == main_window) {
        NODE_SETLASTERR(CURRENT_ADDR, ENULLPTR);
        log_write_trace("ncurses_pause_curses", "default_color || main_window");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
#endif

/**
 * @brief Initializes colors and color pairs to preset values.
 * Colors are to be stored in Central Node color struct, which is a required function argument.
 * 
 * @param nodecolors    Central Node color struct that holds color information
 * @return EXIT_SUCCESS
 * @return EXIT_FAILURE
 */
int ncurses_init_colors(struct color nodecolors[CLR_USR_MAX]) {
    /* safety first, it's C nevertheless.. it's beautiful */
    if (nodecolors == NULL) {
        NODE_SETLASTERR(CURRENT_ADDR, NODE_NOINFO, EINVAL);
        log_write_trace("ncurses_init_colors()", "nodecolors");
        return EXIT_FAILURE;
    }

    /* Initialize all the colors to desired values */
    ncurses_apply_ncolor(&nodecolors[COLOR_ARR_ID(ID_COLOR_BLACK)],       CLR_BG_DEFAULT, CLR_BG_DEFAULT, CLR_BG_DEFAULT);
    ncurses_apply_ncolor(&nodecolors[COLOR_ARR_ID(ID_COLOR_RED)],         CLR_MAX_VAL,    CLR_BG_DEFAULT, CLR_BG_DEFAULT);
    ncurses_apply_ncolor(&nodecolors[COLOR_ARR_ID(ID_COLOR_GREEN)],       CLR_BG_DEFAULT, CLR_MAX_VAL,    CLR_BG_DEFAULT);
    ncurses_apply_ncolor(&nodecolors[COLOR_ARR_ID(ID_COLOR_YELLOW)],      CLR_MAX_VAL,    CLR_MAX_VAL,    CLR_BG_DEFAULT);
    ncurses_apply_ncolor(&nodecolors[COLOR_ARR_ID(ID_COLOR_BLUE)],        CLR_BG_DEFAULT, CLR_BG_DEFAULT, CLR_MAX_VAL);
    ncurses_apply_ncolor(&nodecolors[COLOR_ARR_ID(ID_COLOR_MAGENTA)],     CLR_MAX_VAL,    CLR_BG_DEFAULT, 467);
    ncurses_apply_ncolor(&nodecolors[COLOR_ARR_ID(ID_COLOR_CYAN)],        CLR_BG_DEFAULT, CLR_MAX_VAL,    CLR_MAX_VAL);
    ncurses_apply_ncolor(&nodecolors[COLOR_ARR_ID(ID_COLOR_WHITE)],       CLR_MAX_VAL,    CLR_MAX_VAL,    CLR_MAX_VAL);
    ncurses_apply_ncolor(&nodecolors[COLOR_ARR_ID(ID_COLOR_PURPLE)],      703,            51,             753);
    ncurses_apply_ncolor(&nodecolors[COLOR_ARR_ID(ID_COLOR_BRICKRED)],    560,            78,             CLR_BG_DEFAULT);
    ncurses_apply_ncolor(&nodecolors[COLOR_ARR_ID(ID_COLOR_ORANGE)],      CLR_MAX_VAL,    275,            CLR_BG_DEFAULT);
    ncurses_apply_ncolor(&nodecolors[COLOR_ARR_ID(ID_COLOR_RGBLOOP)],     CLR_BG_DEFAULT, CLR_BG_DEFAULT, CLR_BG_DEFAULT);
    ncurses_apply_ncolor(&nodecolors[COLOR_ARR_ID(ID_COLOR_DARKBLACK)],   0,              0,              0);
    ncurses_apply_ncolor(&nodecolors[COLOR_ARR_ID(ID_COLOR_INVLGRAY)],    784,            784,            784);
    ncurses_apply_ncolor(&nodecolors[COLOR_ARR_ID(ID_COLOR_WHITERST)],    CLR_MAX_VAL,    CLR_MAX_VAL,    CLR_MAX_VAL);


    for (color_t clritr = ID_COLOR_MIN; clritr < ID_COLOR_MIN + CLR_NDEF_COLORS; clritr++) {
        /* Iterate through custom defined colors and initialize set values */
        log_write(LOG_DEBUG, "Initializing color ID %d - nodecolor[%2d] with R: %4d G: %4d B: %4d", clritr, COLOR_ARR_ID(clritr), nodecolors[COLOR_ARR_ID(clritr)].red, nodecolors[COLOR_ARR_ID(clritr)].green, nodecolors[COLOR_ARR_ID(clritr)].blue);
        int cret = init_color(clritr, nodecolors[COLOR_ARR_ID(clritr)].red, nodecolors[COLOR_ARR_ID(clritr)].green, nodecolors[COLOR_ARR_ID(clritr)].blue);
        if (cret != OK) {
            NODE_SETLASTERR(CURRENT_ADDR, NODE_NOINFO, EINITCLR);
            log_write(LOG_ERROR, "Unable to initialize ncurses color ID %hu - E%d", clritr, cret);
            log_write(LOG_DEBUG, "I: %hu - ARR_ID: %hu", cret, clritr, COLOR_ARR_ID(clritr));
            log_write_trace("ncurses_init_colors()", "cret");
            return EXIT_FAILURE;
        }
    }

    int defined_pairs[PAIR_NDEF_PAIRS] = { 0 };

    defined_pairs[0]    = init_pair(ID_PAIR_BLACK,    ID_COLOR_BLACK,     ID_COLOR_BLACK);
    defined_pairs[1]    = init_pair(ID_PAIR_RED,      ID_COLOR_RED,       ID_COLOR_BLACK);
    defined_pairs[2]    = init_pair(ID_PAIR_GREEN,    ID_COLOR_GREEN,     ID_COLOR_BLACK);
    defined_pairs[3]    = init_pair(ID_PAIR_YELLOW,   ID_COLOR_YELLOW,    ID_COLOR_BLACK);
    defined_pairs[4]    = init_pair(ID_PAIR_BLUE,     ID_COLOR_BLUE,      ID_COLOR_BLACK);
    defined_pairs[5]    = init_pair(ID_PAIR_MAGENTA,  ID_COLOR_MAGENTA,   ID_COLOR_BLACK);
    defined_pairs[6]    = init_pair(ID_PAIR_CYAN,     ID_COLOR_CYAN,      ID_COLOR_BLACK);
    defined_pairs[7]    = init_pair(ID_PAIR_WHITE,    ID_COLOR_WHITE,     ID_COLOR_BLACK);
    defined_pairs[8]    = init_pair(ID_PAIR_PURPLE,   ID_COLOR_PURPLE,    ID_COLOR_BLACK);
    defined_pairs[9]    = init_pair(ID_PAIR_BRICKRED, ID_COLOR_BRICKRED,  ID_COLOR_BLACK);
    defined_pairs[10]   = init_pair(ID_PAIR_ORANGE,   ID_COLOR_ORANGE,    ID_COLOR_BLACK);
    defined_pairs[11]   = init_pair(ID_PAIR_RGBLOOP,  ID_COLOR_RGBLOOP,   ID_COLOR_BLACK);
    defined_pairs[12]   = init_pair(ID_PAIR_DARKBLACK,ID_COLOR_DARKBLACK, ID_COLOR_DARKBLACK);
    defined_pairs[13]   = init_pair(ID_PAIR_INVLGRAY, ID_COLOR_BLACK,     ID_COLOR_INVLGRAY);
    defined_pairs[14]   = init_pair(ID_PAIR_RED_INV,  ID_COLOR_BLACK,     ID_COLOR_RED);

    /* now verify if all pairs have been successfully initialized */
    for (int i = 0; i < PAIR_NDEF_PAIRS; i++) {
        if (defined_pairs[i] == ERR) {
            NODE_SETLASTERR(CURRENT_ADDR, NODE_NOINFO, EINITPAIR);
            log_write(LOG_ERROR, "Unable to initialize color pair ID %d - ARR_ID: %d - E%d", i, i + ID_COLOR_MIN, defined_pairs[i]);
            log_write_trace("ncurses_init_colors()", "defined_pairs[]");
            return EXIT_FAILURE;
        }
    }

    log_write(LOG_DEBUG, "Successfully initialized ncurses colors and color pairs");
    return EXIT_SUCCESS;
}

/**
 * @brief Initializes ncurses mainwin, colors and color pairs
 * 
 * @param mainwin       main gui win
 * @param nodecolors    cnode color struct
 * @return EXIT_SUCCESS
 * @return EXIT_FAILURE
 * @return CNODE_EXECUTION_EXIT - Central Node finished SARG tasks and wants to exit
 */
int ncurses_init_gui(WINDOW **mainwin, struct color nodecolors[CLR_USR_MAX]) {
    if ((*mainwin = initscr()) == NULL) {
        NODE_SETLASTERR(CURRENT_ADDR, "Unable to initialize ncurses window",  EINITWIN);
        
        print_message_format_color(STDERR_FILENO, CNODE_NOTIF_ERR);
        puts("Unable to initialize ncurses window");
        return EXIT_FAILURE;
    }

    /* Set winrunning to true and set pointer to mainwin */
    nodecfg.winrunning = true;
    nodecfg.pmainwin = (void *) *mainwin;

    if (!has_colors()) {
        NODE_SETLASTERR(NO_ADDR, "Unable to initialize ncurses colors", ENOCOLOR);
        log_write(LOG_DEBUG, "The target doesn't support ncurses colors");
        log_write_trace("ncurses_init_gui()", "has_colors()");

        print_message_format_color(STDERR_FILENO, CNODE_NOTIF_ERR);
        puts("Unable to initialize ncurses colors");
        return EXIT_FAILURE;
    }

    int start_color_ret = start_color();
    if (start_color_ret != OK) {
        NODE_SETLASTERR(NO_ADDR, NODE_NOINFO, ESTARTCLR);
        log_write(LOG_ERROR, "Unable to start ncurses colors: E%d", start_color_ret);
        log_write_trace("ncurses_init_gui()", "start_color()");

        print_message_format_color(STDERR_FILENO, CNODE_NOTIF_ERR);
        puts("Unable to start ncurses colors");
        return EXIT_FAILURE;
    }

    if (!can_change_color()) {
        NODE_SETLASTERR(NO_ADDR, NODE_NOINFO, ENOCOLOR);
        log_write(LOG_ERROR, "E%d: This terminal doesn't support color redefinition. See '%s --help' for more information.", ENOCOLOR, nodecfg.execname);
        log_write_trace("ncurses_init_gui()", "can_change_color()");

        print_message_format_color(STDERR_FILENO, CNODE_NOTIF_ERR);
        printf("This terminal doesn't support custom colors. See '%s --help' for more information\n", nodecfg.execname);
        return EXIT_FAILURE;
    }

    log_write(LOG_DEBUG, "Terminal includes support for color redefinition");

    /* initialize ncurses colors */
    int nc_color_init_ret = ncurses_init_colors(nodecolors);

    if (nc_color_init_ret != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    /* get terminal screen resolution xy */
    dim_t dim_termx = getmaxx(*mainwin);
    dim_t dim_termy = getmaxy(*mainwin);

    if (dim_termx < TERM_MIN_X || dim_termy < TERM_MIN_Y) {
        NODE_SETLASTERR(NO_ADDR, NODE_NOINFO, EWINSZ);
        log_write(LOG_ERROR, "Terminal size too small! MIN: %ux%u [current: %hux%hu]", TERM_MIN_X, TERM_MIN_Y, dim_termx, dim_termy);
        log_write_trace("ncurses_init_gui()", "dim_termx || dim_termy");

        return EXIT_FAILURE;
    }

    /* Get rid of default terminal background color and explicitly paint it black */
    gui_paint_win_bg(*mainwin, dim_termx, dim_termy);
    wrefresh(*mainwin);

    log_write(LOG_DEBUG, "Running ncurses version %s (%s)", NCURSES_VERSION, curses_version());

    /* this SARG is used to test correct behavior of ncurses colors. the test should show orange text on black background. black bg was already set with the function above */
    if (nodecfg.nc_color_test || nodecfg.first_time_launch) {
        PAIRON_COLOR_MAGENTA(*mainwin);
        for (dim_t iy = 0; iy < dim_termy; iy++) {
            for (dim_t ix = 0; ix < dim_termx; ix++) {
                if (iy == 1 || (iy == dim_termy - 2)) {
                    if (ix > 1 && (ix < dim_termx - 1)) {
                        mvwaddch(*mainwin, iy, ix, '*');
                    }
                }

                if (iy > 1 && iy < (dim_termy - 2)) {
                    if (ix == 2 || ix == (dim_termx - 2)) {
                        mvwaddch(*mainwin, iy, ix, '*');
                    }
                }
            }
        }
        
        char *msg0 = "[ YOU ARE SEEING THIS CALIBRATION MESSAGE BECUASE CENTRAL NODE HAS DETECTED A FIRST TIME LAUNCH ]";
        char *msg1 = "This text is PINK and the background is BLACK";
        char *msg2 = "PRESS ANY KEY TO CONTINUE";
        char *msg3 = "[ -- CENTRAL NODE COLOR TEST  -- ]";
        if (nodecfg.first_time_launch) {
            mvwaddstr(*mainwin, 3, (dim_termx / 2) - (strlen(msg0) / 2), msg0);
        }
        mvwaddstr(*mainwin, 5, (dim_termx / 2) - (strlen(msg1) / 2), msg1);
        mvwaddstr(*mainwin, dim_termy - 5, (dim_termx / 2) - (strlen(msg2) / 2), msg2);
        mvwaddstr(*mainwin, dim_termy / 2, (dim_termx / 2) - (strlen(msg3) / 2), msg3);
        mvwgetch(*mainwin, 1, 1);
        PAIROFF_COLOR_MAGENTA(*mainwin);
        move(0, 0);
        wclrtobot(*mainwin);
        wrefresh(*mainwin);

        return (nodecfg.first_time_launch) ? EXIT_SUCCESS : CNODE_EXECUTION_EXIT;
    }


    return EXIT_SUCCESS;
}