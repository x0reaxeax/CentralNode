#include "../base/cnode.h"          /* cnode */
#include "../gui/termcolor.h"       /* teminal colors */
#include "../sys/config.h"          /* config paths */

#include <stdio.h>
#include <string.h>

void print_help(void) {
    /*  COLOR FORMATTING EXPLANATION:
     *
     * TC_CLR_GREEN         - shell commands
     * TC_BOLDCLR_CYAN      - Section Heading
     * TC_CLR_ORANGE        - A warning
     * TC_BOLDCLR_RED       - A very important warning
     * TC_BOLDCLR_NEON_PINK - A note
     * TC_CLR_YELLOW        - System path
    */ 


    /* Usage */
    printf("Usage: %s [OPTION] ...\n\n", nodecfg.execname);
    
    /* Informational header */
    printf(         " * Central Node requires root privileges, since it utilizes the tinc daemon for all network operations.\n"
                    " * All configuration files are stored at " TC_CLR_YELLOW "\"%s\"" TC_CLR_RESET ", and the default log file location is " TC_CLR_YELLOW "\"%s\".\n" TC_CLR_RESET
                    " * Communication with nodeWD also requires root access.\n"
    TC_CLR_ORANGE   " * It is highly encouraged to run Central Node in a sandboxed environment, to prevent unintentional damage to the system.\n\n" TC_CLR_RESET
    , CNODE_CFG_FOLDER, CNODE_LOG_PATH);
    

    /* Startup parameters */
    puts(TC_BOLDCLR_CYAN 
                "\n    Arguments:" TC_CLR_RESET);
    
    printf(
                "       --help          - Displays this help message. Cannot be used with other startup parameters.\n"
                "       --cleanup       - Perform cleanup (useful after crashes)\n"
                "       --panic         - Completely erases Central Node's presence from the machine" TC_BOLDCLR_RED " (THIS CANNOT BE UNDONE)" TC_CLR_RESET "\n"
                "       --resetcfg      - Resets configuration file to default values\n"
                "       --debug         - Enables debug logging. This option precedes any setting specified in the config file\n"
                "       --testcolor     - Launch a test to determine if a terminal emulator is compatible with Central Node\n"
                "       --genuuid       - Generate new cryptographically secure UUID. Central Node must be run at least ONCE to use this\n\n");
    
    printf(TC_BOLDCLR_CYAN
                        "    Exit codes:\n"       TC_CLR_RESET
            TC_CLR_RED  "       ENULLPTR            " TC_CLR_RESET "%5d - %#6x  - R/W was attempted on null pointer\n"
            TC_CLR_RED  "       EINTEGRITY          " TC_CLR_RESET "%5d - %#6x  - Integrity check failure\n"
            TC_CLR_RED  "       EAUTHFAIL           " TC_CLR_RESET "%5d - %#6x  - Authentication failure\n"
            TC_CLR_RED  "       EUUIDGEN            " TC_CLR_RESET "%5d - %#6x  - libuuid generic failure\n"
            TC_CLR_RED  "       EINVCFG             " TC_CLR_RESET "%5d - %#6x  - Invalid config entry\n"
            TC_CLR_RED  "       EINVLEN             " TC_CLR_RESET "%5d - %#6x  - Invalid parameter length\n"
            TC_CLR_RED  "       EWINSZ              " TC_CLR_RESET "%5d - %#6x  - Invalid terminal window size\n"
            TC_CLR_RED  "       EFPREAD             " TC_CLR_RESET "%5d - %#6x  - Generic file read failure\n"
            TC_CLR_RED  "       EFPWRITE            " TC_CLR_RESET "%5d - %#6x  - Generic file write failure\n"
            TC_CLR_RED  "       EINITCLR            " TC_CLR_RESET "%5d - %#6x  - Failed to initialize ncurses colors\n"
            TC_CLR_RED  "       EINITPAIR           " TC_CLR_RESET "%5d - %#6x  - Failed to initialize ncurses color pairs\n"
            TC_CLR_RED  "       EINITWIN            " TC_CLR_RESET "%5d - %#6x  - Failed to initialize ncurses window\n"
            TC_CLR_RED  "       ENOCOLOR            " TC_CLR_RESET "%5d - %#6x  - Terminal does not support ncurses colors\n"
            TC_CLR_RED  "       ESTARTCLR           " TC_CLR_RESET "%5d - %#6x  - Unable to start ncurses colors\n"
            TC_CLR_RED  "       EOUTBND             " TC_CLR_RESET "%5d - %#6x  - Out of bounds\n"
            TC_CLR_RED  "       ECURLINIT           " TC_CLR_RESET "%5d - %#6x  - Unable to initialize cURL instance\n"
            TC_CLR_RED  "       ECRYPTO             " TC_CLR_RESET "%5d - %#6x  - Generic cryptography error\n"
            TC_CLR_RED  "       EUDEPEND            " TC_CLR_RESET "%5d - %#6x  - Unmet dependencies\n"
            TC_CLR_RED  "       EAUTOLOG            " TC_CLR_RESET "%5d - %#6x  - Auto Event Log failure\n"
            TC_CLR_LIME "       CNODE_EXEC_EXIT     " TC_CLR_RESET "%5d - %#6x  - Exitted before startup routine - not an error\n\n"
            
                        "       Central Node also uses the standard library error numbers, more info can be found in "TC_CLR_GREEN "\"man 3 errno\"" TC_CLR_RESET "\n\n"
TC_BOLDCLR_NEON_PINK    "       Note:\n" TC_CLR_RESET
                        "           The special shell variable " TC_CLR_GREEN "\"$?\"" TC_CLR_RESET " used to read program exit codes is limited to 256 values.\n"
                        "           Since Central Node uses additional/extended error codes, " TC_CLR_GREEN "\"$?\"" TC_CLR_RESET " can yield inaccurate values.\n"
                        "           For this reason, Central Node by default displays any error code number at it's exit routine.\n", 
            ENULLPTR,   ENULLPTR,
            EINTEGRITY, EINTEGRITY,
            EAUTHFAIL,  EAUTHFAIL,
            EUUIDGEN,   EUUIDGEN,
            EINVCFG,    EINVCFG,
            EINVLEN,    EINVLEN,
            EWINSZ,     EWINSZ,
            EFPREAD,    EFPREAD,
            EFPWRITE,   EFPWRITE,
            EINITCLR,   EINITCLR,
            EINITPAIR,  EINITPAIR,
            EINITWIN,   EINITWIN,
            ENOCOLOR,   ENOCOLOR,
            ESTARTCLR,  ESTARTCLR,
            EOUTBND,    EOUTBND,
            ECURLINIT,  ECURLINIT,
            ECRYPTO,    ECRYPTO,
            EUDEPEND,   EUDEPEND,
            EAUTOLOG,   EAUTOLOG,
            CNODE_EXECUTION_EXIT, CNODE_EXECUTION_EXIT
        );


    /* epilogue */
    printf("\nCentral Node %d.%d build %d\n", CNODE_VERSION_MAJOR, CNODE_VERSION_MINOR, CNODE_BUILD_VERSION);
    printf("https://github.com/x0reaxeax/CentralNode\n");
}

void print_uuid_help(char uuid_str[UUID_STR_LEN]) {
    printf("[ * ----------------- NEW UUID ----------------- * ]\n"
           "[+] UUID: " TC_CLR_ORANGE "'%s'" TC_CLR_RESET "\n"
           "[+] Store your UUID in a safe location.\n"
TC_CLR_RED "[+] ! DO NOT LOSE IT !" TC_CLR_RESET "\n"
            , uuid_str);
}