#include "../base/cnode.h"              /* cnode base */
#include "../sys/config.h"
#include "../sys/nodeWD/wdctl.h"

#include "gui.h"                        /* cnode gui */
#include "menu.h"
#include "cursesctl.h"                  /* ncurses ctl */
#include "../net/netops.h"              /* network list */
#include "../net/ifs.h"                 /* net interface list */

#include <menu.h>                       /* menu */
#include <string.h>

#define MENU_MAX_ENTRIES    64          /* maximum number of entries in a menu */

#define MENU_ROWS           5
#define MENU_COLS           1

typedef unsigned int menuid_t;          /* Menu unique IDs */
typedef struct _menu_semaphore {
    menuid_t current_menu;
    menuid_t next_menu;
} msemaphore;


int menu_semaphore(menuid_t start_menu, struct menu_info *menuinfos[MENU_NMENUS]);
msemaphore menu_loadmenu(struct menu_info *minfo);

/**
 * @brief Evaluates ncurses error code and returns it.
 * This function may look pointless, but it's purpose is to check for a specific ncurses return code `E_SYSTEM_ERROR`,
 * by which ncurses indicates that the error code was set in errno. It also looks nicer having a function for this, rather than evaluating it inside the menu_loadmenu().
 * 
 * @param retcode   return code to evaluate
 * @return an error code 
 */
int menu_geterror(int retcode) {
    switch (retcode) {
        case E_SYSTEM_ERROR:
            return errno;
            break;

        case E_BAD_ARGUMENT:
            return E_BAD_ARGUMENT;
            break;

        case E_POSTED:
            return E_POSTED;
            break;

        case E_NOT_CONNECTED:
            return E_NOT_CONNECTED;
            break;

        default:
            return EMENUGEN;
            break;
    }

    return EMENUGEN;
}


/**
 * @brief Displays Central Node's informational messages at the bottom of the screen and lists network interfaces.
 * nodecfg.pmainwin is used for these messages. -- I'm still not satisfied with this, it should be using a separate, new window. Fix Later
 * 
 */
void gui_displayinfo(void) {
    /* net interfaces table - very ugly */

    size_t colx = COLS - ((NET_IPV4_MAX) + (NET_IPV4_MAX * 2) + 10);    /* interface name length + 2x IP address max, + 10 spaces */
    size_t rowy = LINES - nodecfg.pifnets->if_nifs - 4;                 /* Display from the bottom of the window, +4 separator lines */

    nodecfg.ifnet_table_offset = colx - 4;

    PAIRON_COLOR_ORANGE(nodecfg.pmainwin);
    mvwprintw(nodecfg.pmainwin, rowy++, colx - 2, "-------------------------------------------------------");
    mvwprintw(nodecfg.pmainwin, rowy++, colx - 2, "|    INTERFACE    |    INET_ADDR    |     NETMASK     |");
    mvwprintw(nodecfg.pmainwin, rowy++, colx - 2, "|-----------------------------------------------------|");

    int i = 0;
    for (i = 0; i < nodecfg.pifnets->if_nifs; i++) {
        mvwprintw(nodecfg.pmainwin, rowy + i, colx - 2, "| %15s | %15s | %15s |", nodecfg.pifnets->if_name[i], nodecfg.pifnets->if_addr[i], nodecfg.pifnets->if_mask[i]);
    }

    mvwprintw(nodecfg.pmainwin, (rowy + i), colx - 2, "-------------------------------------------------------");
    //mvwprintw(nodecfg.pmainwin, rowy++ + i + 3, colx - 1, "TAP_IFNET: %s - %s", nodecfg.tincifnet, IF_DEV_TUN_DEFAULT_PATH);
    PAIROFF_COLOR_ORANGE(nodecfg.pmainwin);

    /* Informational messages at the bottom of the screen are displayed on mainwindow, since they don't change with menus */
    PAIRON_COLOR_GREEN(nodecfg.pmainwin);
    mvwaddstr(nodecfg.pmainwin, LINES - 2, 0, "Arrow Keys to navigate [END key to exit]");
    /* add wsl detection here */
    char *wsl_text = (nodecfg.is_sys_wsl) ? "(WSL)" : "";
    mvwprintw(nodecfg.pmainwin, LINES - 1, 0, "Central Node %d.%d build %d  -  %s  %s", CNODE_VERSION_MAJOR, CNODE_VERSION_MINOR, CNODE_BUILD_VERSION, curses_version(), wsl_text);
    PAIROFF_COLOR_GREEN(nodecfg.pmainwin);
    wrefresh(nodecfg.pmainwin);
}

static void gui_clear_output_rows(WINDOW *nwin) {
    for (dim_t i = MENU_OUTPUT_ROW_0; i < MENU_OUTPUT_ROW_6; i++) {
        for (dim_t j = 0; j < nodecfg.ifnet_table_offset; j++) {
            mvwaddch(nwin, i, j, ' ');
        }
        /* gui_wclearline(nwin, i, 0); */
    }
    wrefresh(nwin);
}

/**
 * @brief Reads set config values and formats them in array of (char *) pointers as menu entries/items.
 * Items need to be free'd when done.
 * 
 * @param menu_config_entries   - Array of pointers to store config menu entries/items
 * @return EXIT_SUCCESS || EXIT_FAILURE  
 */
int menu_config_formatentries(char *menu_config_entries[CFG_NENTRIES]) {
    if (menu_config_entries == NULL) {
        NODE_SETLASTERR(CURRENT_ADDR, NODE_NOINFO, ENULLPTR);
        log_write_trace("menu_config_formatentries()", "menu_config_entries");
        return EXIT_FAILURE;
    }


   for (int i = 0; i < CFG_NENTRIES; i++) {
       menu_config_entries[i] = malloc(sizeof(char) * (nodecfg.longest_cfg_entry + CFG_ENTRYFMT_MAXLEN + 1));
       if (menu_config_entries[i] == NULL) {
           NODE_SETLASTERR(CURRENT_ADDR, "Unable to allocate memory:", errno);
           log_write(LOG_DEBUG, "Erroneous index: %d", i);
           log_write_trace("menu_config_formatentries()", "menu_config_entries[%d]");
           
           /* Free already allocated memory */
           for (int j = 0; j < i; j++) {
               free(menu_config_entries[j]);
           }
           
           return EXIT_FAILURE;
       }
   }

    snprintf(menu_config_entries[CFG_AUTOFIX],  nodecfg.longest_cfg_entry + CFG_ENTRYFMT_MAXLEN, "%s = \"%d\"",  config_entry_to_str(CFG_AUTOFIX,  CFG_FMT_MACHINE), nodecfg.autofix);
    snprintf(menu_config_entries[CFG_GODMODE],  nodecfg.longest_cfg_entry + CFG_ENTRYFMT_MAXLEN, "%s = \"%d\"",  config_entry_to_str(CFG_GODMODE,  CFG_FMT_MACHINE), nodecfg.godmode);
    snprintf(menu_config_entries[CFG_NWDMODE],  nodecfg.longest_cfg_entry + CFG_ENTRYFMT_MAXLEN, "%s = \"%d\"",  config_entry_to_str(CFG_NWDMODE,  CFG_FMT_MACHINE), nodecfg.nodewd_required);
    snprintf(menu_config_entries[CFG_NETLOGO],  nodecfg.longest_cfg_entry + CFG_ENTRYFMT_MAXLEN, "%s = \"%d\"",  config_entry_to_str(CFG_NETLOGO,  CFG_FMT_MACHINE), nodecfg.netlogos);
    snprintf(menu_config_entries[CFG_ALLOWSU],  nodecfg.longest_cfg_entry + CFG_ENTRYFMT_MAXLEN, "%s = \"%d\"",  config_entry_to_str(CFG_ALLOWSU,  CFG_FMT_MACHINE), nodecfg.allow_root_execution);
    snprintf(menu_config_entries[CFG_NOHTTPS],  nodecfg.longest_cfg_entry + CFG_ENTRYFMT_MAXLEN, "%s = \"%d\"",  config_entry_to_str(CFG_NOHTTPS,  CFG_FMT_MACHINE), nodecfg.net_nohttps);
    snprintf(menu_config_entries[CFG_NLOGLVL],  nodecfg.longest_cfg_entry + CFG_ENTRYFMT_MAXLEN, "%s = \"%d\"",  config_entry_to_str(CFG_NLOGLVL,  CFG_FMT_MACHINE), nodecfg.log_level);
    snprintf(menu_config_entries[CFG_TIMEOUT],  nodecfg.longest_cfg_entry + CFG_ENTRYFMT_MAXLEN, "%s = \"%ld\"", config_entry_to_str(CFG_TIMEOUT,  CFG_FMT_MACHINE), nodecfg.net_timeout);
    snprintf(menu_config_entries[CFG_TUNDEV],   nodecfg.longest_cfg_entry + CFG_ENTRYFMT_MAXLEN, "%s = \"%s\"",  config_entry_to_str(CFG_TUNDEV,   CFG_FMT_MACHINE), nodecfg.tincifnet);
    snprintf(menu_config_entries[CFG_PROXY],    nodecfg.longest_cfg_entry + CFG_ENTRYFMT_MAXLEN, "%s = \"%s\"",  config_entry_to_str(CFG_PROXY,    CFG_FMT_MACHINE), nodecfg.proxyaddr);
    snprintf(menu_config_entries[CFG_NETREPO],  nodecfg.longest_cfg_entry + CFG_ENTRYFMT_MAXLEN, "%s = \"%s\"",  config_entry_to_str(CFG_NETREPO,  CFG_FMT_MACHINE), nodecfg.netrepo);

    return EXIT_SUCCESS;
}

static char **menu_entries_load_networks(unsigned int *num_entries_out) {
    if (NULL == num_entries_out) {
        NODE_SETLASTERR(CURRENT_ADDR, NODE_NOINFO, ENULLPTR);
        log_write_trace("menu_entries_load()", "num_entries_out");
        return NULL;
    }
    
    char **menu_entries = NULL;
    network_info_st *loop = nodecfg.network_info;

    unsigned int num_networks = 0;
    while (loop) {
        if (NULL == loop->network_name) {
            continue;
        }

        char **realloc_menu_entries = reallocarray(menu_entries, sizeof(char *), (num_networks + 1));
        if (NULL == realloc_menu_entries) {
            NODE_SETLASTERR(CURRENT_ADDR, "Unable to reallocate memory:", errno);
            log_write_trace("menu_entries_load()", "realloc_menu_entries");
            break;
        }

        menu_entries = realloc_menu_entries;
        
        /* This is already allocated */
        menu_entries[num_networks] = loop->network_name;
        num_networks++;

        loop = loop->next;
    }

    *num_entries_out = num_networks;
    return menu_entries;
}

int menu_init_all_menus(menuid_t start_menu) {
    /* Main menu */
    char *menu_main_items[] = { (nodecfg.netrepo_status) ? "Browse networks" : "Network Offline", "nodeWD CTL", "Config CTL", "tincd CTL", "Check for Updates", "Reset Logfile", "Contact Info", "Exit" };

    /* nodeWD CTL items */
    char *menu_nodewd_items[] = { "Load nodeWD", "Unload nodeWD", "Show process blacklist", "Set scan interval", (nodecfg.godmode) ? "Disable GodMode" : "Enable GodMode", "Return" };
    /* Number of config entries */
    char *menu_config_items[CFG_NENTRIES];
    if (menu_config_formatentries(menu_config_items) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    size_t netrepo_len = strlen(nodecfg.netrepo);

    /* tincd ctl items */
    char *menu_tincdctl_items[] = { "Check daemon status", "Generate new keypair", "Display UUID", "Return" };

    struct menu_info menu_main;
    menu_main.mid               = MENU_ID_MAIN;
    menu_main.prev_mid          = MENU_ID_MAIN;
    menu_main.menu_entries      = menu_main_items;
    menu_main.nentries          = ARRAY_SIZE(menu_main_items);
    menu_main.menu_title        = "Central Node 7.1";
    menu_main.maxy              = 10;
    menu_main.maxx              = MENU_ENTRY_MAXLEN;

    struct menu_info menu_networks;
    menu_networks.mid           = MENU_ID_NETWORKS;
    menu_networks.prev_mid      = MENU_ID_MAIN;
    menu_networks.menu_entries  = menu_entries_load_networks(&menu_networks.nentries);
    menu_networks.menu_title    = nodecfg.netrepo;
    menu_networks.maxy          = 10;
    menu_networks.maxx          = (netrepo_len > nodecfg.longest_net_entry) ? (netrepo_len + 8) : (((nodecfg.longest_net_entry + 8) > MENU_ENTRY_MAXLEN) ? nodecfg.longest_net_entry + 8 : MENU_ENTRY_MAXLEN);  /* i cant believe this actually works LOL.. please don't kill me whoever is reading this. nested ternary operator for 2 checks */

    struct menu_info menu_nodewd;
    menu_nodewd.mid             = MENU_ID_NODEWDCTL;
    menu_nodewd.prev_mid        = MENU_ID_MAIN;
    menu_nodewd.menu_entries    = menu_nodewd_items;
    menu_nodewd.nentries        = ARRAY_SIZE(menu_nodewd_items);
    menu_nodewd.menu_title      = "nodeWD Control Panel";
    menu_nodewd.maxy            = 10;
    menu_nodewd.maxx            = MENU_ENTRY_MAXLEN;

    struct menu_info menu_config;
    menu_config.mid             = MENU_ID_CONFIGCTL;
    menu_config.prev_mid        = MENU_ID_MAIN;
    menu_config.menu_entries    = menu_config_items;
    menu_config.nentries        = CFG_NENTRIES;
    menu_config.menu_title      = "Config Control Panel";
    menu_config.maxy            = 10;
    menu_config.maxx            = ((nodecfg.longest_cfg_entry + CFG_ENTRYFMT_MAXLEN) > MENU_ENTRY_MAXLEN) ? (nodecfg.longest_cfg_entry + CFG_ENTRYFMT_MAXLEN) : MENU_ENTRY_MAXLEN;

    struct menu_info menu_tincd;
    menu_tincd.mid              = MENU_ID_TINCDCTL;
    menu_tincd.prev_mid         = MENU_ID_MAIN;
    menu_tincd.menu_entries     = menu_tincdctl_items;
    menu_tincd.nentries         = ARRAY_SIZE(menu_tincdctl_items);
    menu_tincd.menu_title       = "Tinc Daemon CTL";
    menu_tincd.maxy             = 10;
    menu_tincd.maxx             = (MENU_ENTRY_MAXLEN + 8);

    /* This will be passed to menu semaphore, so the semaphore can call pre-initialized menus */
    struct menu_info *menuinfos[MENU_NMENUS] = {
        &menu_main,
        &menu_networks,
        &menu_nodewd,
        &menu_config,
        &menu_tincd
    };

    /* Now launch semaphore */
    /* int mret =  */
    menu_semaphore(start_menu, menuinfos);    

    /* Free config entries - allocated memory */
    for (int i = 0; i < CFG_NENTRIES; i++) {
        free(menu_config_items[i]);
    }

    /* Free network entries */
    free(menu_networks.menu_entries);

    return EXIT_SUCCESS;
}


int menu_semaphore(menuid_t start_menu, struct menu_info *menuinfos[MENU_NMENUS]) {
    if (menuinfos == NULL) {
        NODE_SETLASTERR(CURRENT_ADDR, NODE_NOINFO, ENULLPTR);
        log_write_trace("menu_semaphore()", "struct menu_info **menuinfos");
        return EXIT_FAILURE;
    }

    if (start_menu > MENU_NMENUS) {
        NODE_SETLASTERR(CURRENT_ADDR, NODE_NOINFO, EINVAL);
        log_write_trace("menu_semaphore()", "start_menu");
        return EXIT_FAILURE;
    }

    int ret = EXIT_SUCCESS;
    gui_displayinfo();
    msemaphore semaphore = menu_loadmenu(menuinfos[start_menu]);

    /* Evaluate which menu contains the next menu id */
    while (semaphore.next_menu != MENU_EXIT && semaphore.next_menu != MENU_ERROR) {
        /* clean up main window - gets rid of color artifacts when changing menus */
        gui_wclearscr(nodecfg.pmainwin);
        gui_paint_win_bg(nodecfg.pmainwin, getmaxx((WINDOW *) nodecfg.pmainwin), getmaxy((WINDOW *) nodecfg.pmainwin));
        
        /* info needs to be "redisplayed" LOL */
        gui_displayinfo();

        semaphore = menu_loadmenu(menuinfos[semaphore.next_menu]);
    }

    log_write(LOG_DEBUG, "Menu Semaphore - Exiting");

    ret = (semaphore.next_menu == MENU_ERROR) ? EXIT_FAILURE : EXIT_SUCCESS;

    return ret;
}


int menu_execoption(struct menu_info *minfo, msemaphore *semaphore, unsigned int item_id) {

    if (NULL == minfo || NULL == semaphore) {
        NODE_SETLASTERR(CURRENT_ADDR, NODE_NOINFO, ENULLPTR);
        log_write_trace("menu_execoption()", "minfo || semaphore");
        return EXIT_FAILURE;
    }

    /* log_write(LOG_NOTIF, "Evaluating menu ID: %u prev%u '%s' [%p]", minfo->mid, minfo->prev_mid, minfo->menu_title, &minfo); */
    switch (minfo->mid) {
        case MENU_ID_MAIN:
            if (item_id == (minfo->nentries - 1)) {
                /* Last item is EXIT */
                semaphore->next_menu = MENU_EXIT;
                break;
            }
            
            /* This can be evaluated if (item_id < MENU_ID_CONFIGCTL - 1) { semaphore->next_menu = item_id + 1; } */
            switch (item_id) {
                case (MENU_ID_NETWORKS - 1):
                    /* network menu - only allow menu switching after checking repo status */
                    if (nodecfg.netrepo_status) {
                        semaphore->next_menu = MENU_ID_NETWORKS;
                    }
                    break;
            
                case (MENU_ID_NODEWDCTL - 1):
                    /* nodewd ctl menu */
                    if (nodecfg.is_sys_wsl) {
                        PAIRON_COLOR_ORANGE(nodecfg.pmainwin);
                        mvwaddstr(nodecfg.pmainwin, MENU_OUTPUT_ROW_0, 0, "[!] nodeWD is not available for Windows Subsystem Linux");
                        wrefresh(nodecfg.pmainwin);
                        break;
                    }

                    semaphore->next_menu = MENU_ID_NODEWDCTL;
                    break;

                case (MENU_ID_CONFIGCTL - 1):
                    /* configctl */
                    semaphore->next_menu = MENU_ID_CONFIGCTL;
                    break;

                case (MENU_ID_TINCDCTL - 1):
                    /* tincdctl */
                    semaphore->next_menu = MENU_ID_TINCDCTL;
                    break;

                case MENU_MAIN_ENTRY_UPDATE:
                    /* check for updates */
                    PAIRON_COLOR_YELLOW(nodecfg.pmainwin);
                    
                    mvwaddstr(nodecfg.pmainwin, MENU_OUTPUT_ROW_0, 0, "Checking for updates..");
                    PAIROFF_COLOR_YELLOW(nodecfg.pmainwin);
                    wrefresh(nodecfg.pmainwin);
                    
                    PAIRON_COLOR_GREEN(nodecfg.pmainwin);
                    char *msg = (net_checkupdate()) ? "Update available" : "No updates available";
                    gui_wclearline(nodecfg.pmainwin, MENU_OUTPUT_ROW_0, 0);
                    mvwaddstr(nodecfg.pmainwin, MENU_OUTPUT_ROW_0, 0, msg);
                    PAIROFF_COLOR_GREEN(nodecfg.pmainwin);
                    
                    wrefresh(nodecfg.pmainwin);
                    break;

                case MENU_MAIN_ENTRY_RESETLOG:
                    PAIRON_COLOR_ORANGE(nodecfg.pmainwin);
                    mvwaddstr(nodecfg.pmainwin, MENU_OUTPUT_ROW_0, 0, "Reset logfile? [y/N]");
                    wrefresh(nodecfg.pmainwin);
    
                    char choice = mvwgetch(nodecfg.pmainwin, MENU_OUTPUT_ROW_0, 23);
                    if (choice != 'Y' && choice != 'y') {
                        gui_wclearline(nodecfg.pmainwin, MENU_OUTPUT_ROW_0, 0);
                        mvwaddstr(nodecfg.pmainwin, MENU_OUTPUT_ROW_0, 0, "Aborted");
                        wrefresh(nodecfg.pmainwin);

                        break;
                    }

                    int retval = log_reset();
                    char *infostr = (retval == EXIT_SUCCESS) ? "Logfile has been reset" : "Unable to reset logfile";
                    mvwaddstr(nodecfg.pmainwin, MENU_OUTPUT_ROW_0, 0, infostr);

                    PAIROFF_COLOR_ORANGE(nodecfg.pmainwin);
                    wrefresh(nodecfg.pmainwin);
                    break;

                case MENU_MAIN_ENTRY_CONTACT:
                    PAIRON_COLOR_MAGENTA(nodecfg.pmainwin);
                    mvwaddstr(nodecfg.pmainwin, MENU_OUTPUT_ROW_0, 0, "Discord: xoreaxeax#9705");
                    mvwaddstr(nodecfg.pmainwin, MENU_OUTPUT_ROW_1, 0, "GitHub:  github.com/x0reaxeax");
                    mvwaddstr(nodecfg.pmainwin, MENU_OUTPUT_ROW_2, 0, "E-Mail:  x0reaxeax@sigaint.net");
                    PAIROFF_COLOR_MAGENTA(nodecfg.pmainwin);
                    wrefresh(nodecfg.pmainwin);
                    break;

                default:
                    break;
            }
            break;
    
        case MENU_ID_NODEWDCTL:
            if (item_id == (minfo->nentries - 1)) {
                /* Last item is EXIT */
                semaphore->next_menu = minfo->prev_mid;
                break;
            }

            break;

        case MENU_ID_TINCDCTL:
            if (item_id == (minfo->nentries - 1)) {
                /* Last item is EXIT */
                semaphore->next_menu = minfo->prev_mid;
                break;
            }

            switch (item_id) {
                case MENU_TINCDCTL_ENTRY_DSTATUS:
                    /* end curses mode temporarily */
                    endwin();
                    printf("[+] Example status check outside curses mode\n");
                    getchar();

                    /* Switch back to curses mode */
                    wrefresh(nodecfg.pmainwin);
                    break;
                
                case MENU_TINCDCTL_ENTRY_SHOWUUID:
                    log_write(LOG_DEBUG, "Reading UUID from '%s'..", CNODE_UUID_PATH);
                    char uuidstr[UUID_STR_LEN + 1] = { 0 };
                    if (config_read_uuid(uuidstr) == EXIT_SUCCESS) {
                        PAIRON_COLOR_ORANGE(nodecfg.pmainwin);
                        mvwprintw(nodecfg.pmainwin, MENU_OUTPUT_ROW_0, 0, "UUID: '%s'", uuidstr);
                        PAIROFF_COLOR_ORANGE(nodecfg.pmainwin);
                        wrefresh(nodecfg.pmainwin);
                    }

                    break;
            
                default:
                    break;
            }

        default:
            break;
    }

    return EXIT_SUCCESS;
}

/*
  ·  ·    .  .
 ·    ·  .    .
  ·  ·    .  .
*/


/**
 * @brief Sets up and loads a new menu
 * 
 * @param minfo pointer to a menu_info struct, with necessary parameters to set up a new menu according to this recipe.
 *
 * @return ID of next menu to be called 
 * @return `MENU_ERROR` An error has occured and the menu couldn't be set up
 */
msemaphore menu_loadmenu(struct menu_info *minfo) {
    msemaphore semaphore;

    /* Safety first */
    if (minfo == NULL) {
        NODE_SETLASTERR(CURRENT_ADDR, NODE_NOINFO, ENULLPTR);
        log_write_trace("menu_loadmenu()", "minfo");
        semaphore.current_menu  = MENU_ERROR;
        semaphore.next_menu     = MENU_ERROR;
        return semaphore;
    }

    semaphore.next_menu         = minfo->mid;
    semaphore.current_menu      = minfo->mid;

    log_write(LOG_DEBUG, "Loading menu ID '%u' titled '%s'", minfo->mid, minfo->menu_title);

    int rstatus = E_OK;                     /* return status for ncurses functions */
    MENU *menu_menu         = NULL;         /* Menu instance */
    ITEM **item_allitems    = NULL;         /* All menu items */

    WINDOW *menu_window     = NULL;         /* menu window */
    

    /* ncurses menu library stubbornly creates menus with white foreground and black background, even if wattron(CP(ID)) is called.
     * For this reason, these default colors need to be redefined temporarily. Their predefined values are saved in the following two structs,
     * which are later used for restoration of the original colors, to prevent screwing terminal schemes. Yes, I'm talking to you WSL terminal.. */ 
    struct color color_white_backup;
    struct color color_black_backup;

    /* color white is used for menu items/entries and menu borders, cyan sounds better.
     * color cyan is already a custom defined color, so the RGB values are extracted from it 
    */
    color_t cyan_red, cyan_green, cyan_blue;
    if (color_content(ID_COLOR_CYAN, &cyan_red, &cyan_green, &cyan_blue) == ERR) {
        NODE_SETLASTERR(CURRENT_ADDR, NODE_NOINFO, ECLRSAVE);
        log_write(LOG_ERROR, "Unable to extract color values from color ID %d", ID_COLOR_CYAN);
        log_write_trace("menu_loadmenu()", "color_content()");
        
        /* !~MENU_ERROR has to be returned instead of goto EXIT_ROUTINE, because EXIT_ROUTINE contains variables which were supposed to be initialized,
         * but weren't because an error has occured.
        */
        semaphore.next_menu     = MENU_ERROR;
        return semaphore;
    }
    
    if (ncurses_swap_color(&color_white_backup, COLOR_WHITE, cyan_red, cyan_green, cyan_blue) != EXIT_SUCCESS) {
        /* unable to swap colors, return MENU_ERROR for reasons explained above */
        semaphore.next_menu     = MENU_ERROR;
        return semaphore;
    }
    
    /* custom color ID_COLOR_BLACK uses CLR_BG_DEFAULT as for all three RGB values */
    if (ncurses_swap_color(&color_black_backup, COLOR_BLACK, CLR_BG_DEFAULT, CLR_BG_DEFAULT, CLR_BG_DEFAULT) != EXIT_SUCCESS) {
        /* unable to swap colors, return MENU_ERROR for reasons explained above */
        semaphore.next_menu     = MENU_ERROR;
        return semaphore;
    }

    cbreak();               /* Enable cbreak, so signals can still be sent */

    /* Now enable ncurses properties */
    noecho();               /* disable echo */
    curs_set(0);            /* disable terminal cursor */


    /* allocate memory for array of menu entries */
    item_allitems = calloc((minfo->nentries) + 1, sizeof(ITEM *));
    if (item_allitems == NULL) {
        NODE_SETLASTERR(CURRENT_ADDR, "Unable to allocate memory:", errno);
        log_write_trace("menu_loadmenu()", "item_allitems");
        semaphore.next_menu     = MENU_ERROR;
        
        goto END_ROUTINE;
    }

    /* initialize all entries/items */
    for (size_t i = 0; i < minfo->nentries; i++) {
        item_allitems[i] = new_item(minfo->menu_entries[i], NULL);
    }

    /* Explicitly set last entry to NULL, because ncurses.. */
    item_allitems[minfo->nentries] = NULL;

    /* Create new menu */
    menu_menu = new_menu(item_allitems);

    if (menu_menu == NULL) {
        NODE_SETLASTERR(CURRENT_ADDR, NODE_NOINFO, errno);
        log_write(LOG_ERROR, "Unable to initialize new menu with ID %d - E%d", minfo->mid, NODE_READLASTERR);
        goto END_ROUTINE;
    }

    /* now create new window for the menu: 
        WINDOW *newwin(
             int nlines, int ncols,
             int begin_y, int begin_x);
    */
    menu_window = newwin(minfo->maxy, minfo->maxx, 4, 4);

    if (menu_window == NULL) {
        NODE_SETLASTERR(CURRENT_ADDR, NODE_NOINFO, EINITWIN);
        log_write(LOG_ERROR, "Unable to initialize new menu window for menu ID %d", minfo->mid);
        
        semaphore.next_menu     = MENU_ERROR; 
        goto END_ROUTINE;
    }

    /* assign menu window to the menu */
    if ((rstatus = set_menu_win(menu_menu, menu_window)) != E_OK) {
        NODE_SETLASTERR(CURRENT_ADDR, "Unable to associate menu with it's window:", menu_geterror(rstatus));
        log_write_trace("menu_loadmenu()", "set_menu_win()");
        
        semaphore.next_menu     = MENU_ERROR;
        goto END_ROUTINE;
    }

    /* Create a submenu window pair for menu controlling */
    if ((rstatus = set_menu_sub(menu_menu, derwin(menu_window, (minfo->maxy) - 4, (minfo->maxx) - 2, 3, 2))) != E_OK) {
        NODE_SETLASTERR(CURRENT_ADDR, "Unable to create subwindow for menu:", menu_geterror(rstatus));
        log_write_trace("menu_loadmenu()", "set_menu_sub()");

        semaphore.next_menu     = MENU_ERROR;
        goto END_ROUTINE;
    }

    /* set menu format, max rows and cols */
    if ((rstatus = set_menu_format(menu_menu, MENU_ROWS, MENU_COLS)) != E_OK) {
        NODE_SETLASTERR(CURRENT_ADDR, "Unable to set menu size format:", menu_geterror(rstatus));
        log_write_trace("menu_loadmenu()", "set_menu_format()");
        
        semaphore.next_menu     = MENU_ERROR;
        goto END_ROUTINE;
    }

    /* Set menu cursor for highlighted item */
    if ((rstatus = set_menu_mark(menu_menu, " > ")) != E_OK) {
        NODE_SETLASTERR(CURRENT_ADDR, "Unable to set menu cursor mark:", menu_geterror(rstatus));
        log_write_trace("menu_loadmenu()", "set_menu_mark()");

        semaphore.next_menu     = MENU_ERROR;
        goto END_ROUTINE;
    }

    /* paint the whole window background */
    gui_paint_win_bg(menu_window, getmaxx(menu_window), getmaxy(menu_window));

    /* set menu box and header color to red */
    PAIRON_COLOR_RED(menu_window);
    
    /* Draw border line around menu with default line characters */
    box(menu_window, 0, 0);

    menu_print_in_middle(menu_window, 1, 0, minfo->maxx, minfo->menu_title);
    
    /* draw line separators in menu box */
    mvwaddch(menu_window, 2, 0, ACS_LTEE);
    mvwhline(menu_window, 2, 1, ACS_HLINE, (minfo->maxx) - 2);
    mvwaddch(menu_window, 2, (minfo->maxx) - 1, ACS_RTEE);
    wrefresh(menu_window);

    /* post menu and refresh */
    if ((rstatus = post_menu(menu_menu)) != E_OK) {
        NODE_SETLASTERR(CURRENT_ADDR, "Unable to post ncurses menu:", menu_geterror(rstatus));
        log_write_trace("menu_loadmenu()", "post_menu()");

        goto END_ROUTINE;
    }
    PAIROFF_COLOR_RED(menu_window);
    wrefresh(menu_window);
    
    /* turn on keypad to allow keyboard menu navigation */
    keypad(menu_window, true);

    /* menu is now setup, any handling code goes below */


    int key_pressed = -1;               /* key pressed in console window */
    
    //menuid_t next_menu_id = minfo->mid; /* next menu id. holds current menu id while in menu. any other value than current menuid exits the current menu */

    /* This is the menu key handler */
    while ((key_pressed = wgetch(menu_window)) != ERR && minfo->mid == semaphore.next_menu) {
        /* current selectec/highlighted item in menu */
        int id_item_selected = item_index(current_item(menu_menu));
        if (id_item_selected == ERR) {
            NODE_SETLASTERR(CURRENT_ADDR, NODE_NOINFO, ENULLPTR);
            log_write_trace("menu_loadmenu()", "id_item_selected");

            semaphore.next_menu = MENU_ERROR;
        }
    
        gui_clear_output_rows(nodecfg.pmainwin);

        /* END key exits Central Node */
        switch (key_pressed) {
            /* down arrow */
            case KEY_DOWN:
                /* move down an item */
                menu_driver(menu_menu, REQ_DOWN_ITEM);
                break;
        
            /* up arrow */
            case KEY_UP:
                /* move up an item-izer */
                menu_driver(menu_menu, REQ_UP_ITEM);
                break;

            /* left arrow */
            case KEY_LEFT:
                /* return to previous menu */
                semaphore.next_menu = minfo->prev_mid;
                break;

            /* Big Enter Key, numpad enter key and right arrow are all the same */
            case KEY_BIGENTER:   case KEY_ENTER:     case KEY_RIGHT:
                ;
                int ret = menu_execoption(minfo, &semaphore, id_item_selected);
                log_write(LOG_DEBUG, "Menu operation returned exit code '%d'", ret);
                break;

            /* backtick under ESC */
            case KEY_SHOME:
                /* console */
                log_write(LOG_DEBUG, "Spawning console");
                break;

            case KEY_END:
                /* Exit */
                semaphore.next_menu = MENU_EXIT;
                break;

            default:
                log_write(LOG_DEBUG, "MENU_HANDLER: Unknown key pressed - '%d'", key_pressed);
                break;
        }

        wrefresh(menu_window);
        if (semaphore.next_menu != minfo->mid) {
            break;
        }
    }

    if (key_pressed == ERR) {
        NODE_SETLASTERR(CURRENT_ADDR, "An error has occured reading keyboard input:", errno);
        log_write_trace("menu_loadmenu()", "wgetch() => key_pressed");
    }


    /* End routine, all handling code ends here */
    END_ROUTINE:
    log_write(LOG_DEBUG, "Entering END_ROUTINE for menu window %d", minfo->mid);
    /* restore redefined colors
     * No sense in checking return values, since errors are handled inside the functions,
     * and cleanup below is mandatory
    */
    ncurses_restore_color(&color_white_backup, COLOR_WHITE);
    ncurses_restore_color(&color_black_backup, COLOR_BLACK);

    /* clear screen */
    gui_wclearscr(menu_window);
    gui_paint_win_bg(menu_window, getmaxx(menu_window), getmaxy(menu_window));

    /* set keypad back to default */
    keypad(menu_window, false);

    if (menu_menu != NULL) {
        /* unpost menu */
        unpost_menu(menu_menu);
        
        /* free menu memory */
        free_menu(menu_menu);
    }

    /* free menu entries */
    for (size_t i = 0; i < minfo->nentries + 1; i++) {
        if (item_allitems[i] != NULL) {
            free_item(item_allitems[i]);
        }
    }

    if (item_allitems != NULL) {
        free(item_allitems);
    }

    /* clear all screen output */
    gui_wclearscr(menu_window);

    /* delete menu window */
    delwin(menu_window);
    endwin();

    /* restore cursor and echo */
    echo();
    curs_set(1);

    return semaphore;
}