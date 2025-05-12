
#define EC_STX        "\x02"                    // Start Of teXt
#define EC_ETX        "\x03"                    // End Of teXt

#define EC_LF         "\x0a"                    // Line Feed
#define EC_CR         "\x0d"                    // Carriage Return

#define EC_COPR       "(c)" // "\xb8"           // Copyright sign
#define EC_RETM       "(r)" // "\xa9"           // Registered sign

#define EC_CRLF       "\x0d\x0a"                // Carriage Return & Line Feed

#define EC_CLS        "\e[2J"                   // CLear Screen
#define EC_CHM        "\e[H"                    // Cursor HoMe

// #define EC_CURPOS(V,H)  "\e[" V ";" H "H"

#define EC_CLRTABS    "\e[3g"                   // CLeaR TABS
#define EC_SETTAB     "\eH"                     // SET TAB

#define EC_NOF        "\e[m"                    // NO Format
#define EC_BLD        "\e[1m"                   // BoLD
#define EC_LOI        "\e[2m"                   // LOw Intensity
#define EC_ULN        "\e[4m"                   // UnderLiNe
#define EC_BLK        "\e[5m"                   // BLinK
#define EC_REV        "\e[7m"                   // REverse Video
#define EC_INV        "\e[8m"                   // INVisible text

#define EC_DHI_T      "\e#3"                    // Double HeIght Top
#define EC_DHI_B      "\e#4"                    // Double HeIght Bottom
#define EC_SWI        "\e#5"                    // Single WIdth
#define EC_DWI        "\e#6"                    // Double WIdth

#define EC_CON        "\e[?25h"                 // Cursor ON
#define EC_COF        "\e[?25l"                 // Cursor OFf

