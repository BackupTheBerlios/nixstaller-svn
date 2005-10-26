#include "main.h"

int main()
{
    CLibSU LibSU("ls -a", "mama");
    char passwd[128];
    printf("password: ");
    if (scanf("%s", passwd))
    {
        LibSU.ExecuteCommand("passwd", 1);
        for (short s=0;s<128;s++) passwd[s] = 0; // Clear passwd
    }
    else
        printf("\nSorry\n");
    return 0;
}
