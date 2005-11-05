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
        printf("ExecuteCommand: %d\n", LibSU.ExecuteCommand(passwd, true));
    }
    else
        printf("\nSorry\n");
    return 0;
}
