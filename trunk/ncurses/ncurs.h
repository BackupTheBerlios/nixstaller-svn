#ifndef NCURS_H
#define NCURS_H

#include <vector>
#include <list>
#include <sstream>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <cdk/cdk.h>
#include "main.h"

#define NO_FILE -1
#define ESCAPE  -2

#define DEFAULT_WIDTH       50
#define DEFAULT_HEIGHT      12

int ReadDir(const std::string &dir, char ***list);
int SwitchButtonK(EObjectType cdktype GCC_UNUSED, void *object, void *clientData, chtype key GCC_UNUSED);
int ScrollParamMenuK(EObjectType cdktype, void *object, void *clientData, chtype key);
int ExitK(EObjectType cdktype, void *object, void *clientData, chtype key);
int ViewFile(char *file, char **buttons, int buttoncount, char *title);
void WarningBox(const char *msg, ...);
bool YesNoBox(const char *msg, ...);
void ExtrThinkFunc(void *p);
void PrintExtrOutput(const char *msg, void *p);

inline int dummyK(EObjectType cdktype GCC_UNUSED, void *object, void *clientData GCC_UNUSED,
                  chtype key GCC_UNUSED) { return true; };

#include "widgets.h"

extern WINDOW *MainWin;
extern CDKSCREEN *CDKScreen;
extern CButtonBar ButtonBar;

inline void PrintInstOutput(const char *msg, void *p) { ((CCDKSWindow *)p)->AddText(msg, false); };
inline int GetMaxHeight(void) { return getbegy(ButtonBar.GetLabel()->GetLabel()->win)-2; };

#endif
