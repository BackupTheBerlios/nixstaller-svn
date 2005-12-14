#include "libsu.h"

using namespace LIBSU;

void printout(const char *s, void *p) { printf(s); }

int main()
{
    char user[128], passwd[128];
    printf("user: ");
    scanf("%s", user);
    printf("password: ");
    if (scanf("%s", passwd))
    {
        CLibSU LibSU("cdrecord --help", user);
        LibSU.SetPath("/bin:/usr/bin:/usr/pkg/bin:/usr/local/bin");
        LibSU.SetTerminalOutput(false);
        LibSU.SetOutputFunc(printout);
        printf("ExecuteCommand: %d\n", LibSU.ExecuteCommand(passwd, true));
    }
    else
        printf("\nSorry\n");
    return 0;
}
