#ifndef _CNODE_CURSES_MENU_H_
#define _CNODE_CURSES_MENU_H_

/* Menu unique IDs */
typedef unsigned int menuid_t;

#define KEY_BIGENTER                    0xA         /* ncurses only defines numpad enter key */

#define MENU_ENTRY_MAXLEN               32          /* Max length of menu item/entry string */
#define MENU_ENTRY_WIDE_MAXLEN          68          /* Max length of wide menu item/entry string */

#define MENU_ID_MAIN                    0           /* main menu */
#define MENU_ID_NETWORKS                1           /* network list menu */
#define MENU_ID_NODEWDCTL               2           /* nodeWD control panel menu */
#define MENU_ID_CONFIGCTL               3           /* config control panel menu */
#define MENU_ID_TINCDCTL                4

#define MENU_NMENUS                     5           /* Number of defined menus */

#define MENU_MAIN_ENTRY_UPDATE          4
#define MENU_MAIN_ENTRY_RESETLOG        5
#define MENU_MAIN_ENTRY_CONTACT         6

#define MENU_TINCDCTL_ENTRY_DSTATUS     0
#define MENU_TINCDCTL_ENTRY_NEWPAIR     1
#define MENU_TINCDCTL_ENTRY_SHOWUUID    2

#define MENU_EXIT                       0x100
#define MENU_ERROR                      0x101
#define MENU_ID_INVALID                 0x102

#define MENU_OUTPUT_ROW_0               (LINES - 12)
#define MENU_OUTPUT_ROW_1               (LINES - 11)
#define MENU_OUTPUT_ROW_2               (LINES - 10)
#define MENU_OUTPUT_ROW_3               (LINES - 9)
#define MENU_OUTPUT_ROW_4               (LINES - 8)
#define MENU_OUTPUT_ROW_5               (LINES - 7)
#define MENU_OUTPUT_ROW_6               (LINES - 6)

/**
 * @brief Contains menu information necessary for loading any menu
 * 
 */
struct menu_info {
    menuid_t mid;                                   /* unique menu id */
    menuid_t prev_mid;                              /* previous menu id */
    unsigned int nentries;                          /* number of menu entries */
    char **menu_entries;                            /* Holds menu entries */
    char *menu_title;                               /* menu title/name */
    unsigned int maxy;                              /* max size of x-dim */
    unsigned int maxx;                              /* max size of y-dim */
};

/**
 * @brief Initializes all CNODE menus and loads the specified menu
 * 
 * @param start_menu    ID of menu to start 
 * @return EXIT_SUCCESS 
 * @return EXIT_FAILURE  
 */
int menu_init_all_menus(menuid_t start_menu);


/* misc menu constant */
#define CTRLD               4

/* array size macro */
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))


#endif