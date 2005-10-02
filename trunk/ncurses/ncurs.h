#ifndef NCURS_H
#define NCURS_H

#include <cdk/cdk.h>
#include "main.h"

#define NO_FILE -1
#define ESCAPE  -2

#define DEFAULT_WIDTH       50
#define DEFAULT_HEIGHT      12

extern int ReadDir(const char *dir, char ***list);
extern int SwitchButtonK(EObjectType cdktype GCC_UNUSED, void *object, void *clientData GCC_UNUSED, chtype key GCC_UNUSED);
extern int CreateDirK(EObjectType cdktype GCC_UNUSED, void *object, void *clientData GCC_UNUSED, chtype key GCC_UNUSED);
extern int ViewFile(char *file, char **buttons, int buttoncount, char *title);
extern void SetBottomLabel(char **msg, int count);

extern CDKSCREEN *CDKScreen;
extern CDKLABEL *BottomLabel;

inline int dummyK(EObjectType cdktype GCC_UNUSED, void *object, void *clientData GCC_UNUSED,
                  chtype key GCC_UNUSED) { return true; };
#endif
