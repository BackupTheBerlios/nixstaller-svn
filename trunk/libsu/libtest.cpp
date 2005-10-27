#include "libsu.h"

int main()
{
    char user[128], passwd[128];
    printf("user: ");
    scanf("%s", user);
    printf("password: ");
    if (scanf("%s", passwd))
    {
        CLibSU LibSU("cdrecord --help", user);
        LibSU.ExecuteCommand(passwd, 1);
        for (short s=0;s<128;s++) passwd[s] = 0; // Clear passwd
    }
    else
        printf("\nSorry\n");
    return 0;
}
