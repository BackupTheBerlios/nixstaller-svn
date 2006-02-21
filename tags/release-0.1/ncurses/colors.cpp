#include <cdk/cdk.h>

int main()
{
    int color[]	= {
                            COLOR_WHITE,	COLOR_RED,	COLOR_GREEN,
                            COLOR_YELLOW,	COLOR_BLUE,	COLOR_MAGENTA,
                            COLOR_CYAN,	COLOR_BLACK
                    };
    int pair = 1;
    int fg, bg;
    int limit;

    limit = 8;//(COLORS < 8) ? COLORS : 8;

    /* Create the color pairs. */
    for (fg=0; fg < limit; fg++)
    {
        for (bg=0; bg < limit; bg++)
        {
            printf("pair: %d c[fg]: %d c[bg]: %d\n", pair++, color[fg], color[bg]);
        }
    }

    return 0;
} 
