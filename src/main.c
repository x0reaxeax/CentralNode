#include "base/cnode.h"     /* cnode base */
#include "sys/logging.h"    /* logging */

#include "gui/gui.h"        /* gui stuff */
#include "gui/cursesctl.h"  /* curses ctl */

#include "net/ifs.h"        /* if_get_ifnet_names() - maybe move this to cnode_init_instance() w/ heap allocation */
#include "gui/menu.h"       /* menu ctl */

int main(int argc, char const *argv[]) {
    /* code */

    /* Main Window */
    WINDOW *mainwin = NULL;

    /* net interfaces */
    struct sys_ifnets ifns;
    
    /* color values struct */
    struct color nodecolors[CLR_USR_MAX];

    int cnode_init_status = cnode_init_instance(argc, argv);
    if (cnode_init_status != EXIT_SUCCESS) {
        /* Failed to initalize cnode, or it was never supposed to start (CNODE_EXECUTION_EXIT) */
        cnode_cleanup();
        return cnode_init_status;
    }

    int init_nc_status = ncurses_init_gui(&mainwin, nodecolors);
    if (init_nc_status != EXIT_SUCCESS) {
        /* Failed to initialize gui, or it was never supposed to start (CNODE_EXECUTION_EXIT) */
        cnode_cleanup();
        return init_nc_status;
    }

    /* fetch network interfaces */
    if (if_get_ifnet_names(&ifns) != EXIT_SUCCESS) {
        /* Failed to retrieve network interfaces */
        cnode_cleanup();
        return EXIT_FAILURE;
    }

    /* This is the final function to call here, from now on, everything will be handled from inside the menu */
    if (menu_init_all_menus(MENU_ID_MAIN) != EXIT_SUCCESS) {
        /* Failed to initialize menus */
        cnode_cleanup();
        return EXIT_FAILURE;
    }

    /* cleanup */
    cnode_cleanup();
    return EXIT_SUCCESS;
}
