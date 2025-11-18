#pragma once


#define EC_STR2(X)   #X
#define EC_STR(X)    EC_STR2(X)

#define EC_STX        "\x02"                    // Start Of teXt
#define EC_ETX        "\x03"                    // End Of teXt

#define EC_LF         "\x0a"                    // Line Feed
#define EC_CR         "\x0d"                    // Carriage Return

#define EC_COPR       "(c)" // "\xb8"           // Copyright sign
#define EC_RETM       "(r)" // "\xa9"           // Registered sign

#define EC_CRLF       "\x0d\x0a"                // Carriage Return & Line Feed
#define EC_TAB        "\t"                      // TAB

#define EC_CLS        "\e[2J"                   // CLear Screen
#define EC_CHM        "\e[H"                    // Cursor HoMe

#define EC_CURPOS(H,V)  "\e[" EC_STR(V) ";" EC_STR(H) "H"       // Set cursor to positon (H,V)

void buildCursorPosCode(int h, int v, char *buffer) {
  sprintf(buffer, "\e[%d;%dH", v, h);
}

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

#define EC_ALLOCSPRITES(N)                    "\e_GSPRITECOUNT" EC_STR(N) "$"
#define EC_DEFSPRITE(I,W,H,F,DATA)            "\e_GSPRITEDEF" EC_STR(I) ";" EC_STR(W) ";" EC_STR(H) ";" F ";" DATA "$"
#define EC_DEFSPRITECOL(I,W,H,F,R,G,B,DATA)   "\e_GSPRITEDEF" EC_STR(I) ";" EC_STR(W) ";" EC_STR(H) ";" F ";" EC_STR(R) ";" EC_STR(G) ";" EC_STR(B) ";" DATA "$"
#define EC_SETSPRITE(I,V,F,X,Y)               "\e_GSPRITESET" EC_STR(I) ";" V ";" EC_STR(F) ";" EC_STR(X) ";" EC_STR(Y) "$"

// 64x20 1 bpp image.
#define SPOROS_TECH_MONO_LOGO_DATA "038000000000e00207c000000001f0070fe000000003f8070ee000000003b8070440000000011007000000000000000730006000018006077000f00003c007077001f80007e0070ee001f80007e0039ee000f00003c0039ce0006000018003bce00000ff800003b8e00001ffc0000390e00001ffc0000380e000000000000380e000000000000380700000000000030070000000000007003000000000000600"

// Some extra predefined colors

#define AMBER_COLOR RGB888(255,192,0)


