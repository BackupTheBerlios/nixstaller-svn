#include "libsu.h"

using namespace LIBSU;

void printout(const char *s, void *p) { printf(s); }

int main()
{
    char command[128], user[128], passwd[128];
    printf("command: ");
    scanf("%s", command);
    printf("user: ");
    scanf("%s", user); // Everyone can see your password...not good
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
