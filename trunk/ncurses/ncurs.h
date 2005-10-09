#ifndef NCURS_H
#define NCURS_H

#include <list>

#include <cdk/cdk.h>
#include "main.h"

#define NO_FILE -1
#define ESCAPE  -2

#define DEFAULT_WIDTH       50
#define DEFAULT_HEIGHT      12

int ReadDir(const char *dir, char ***list);
int SwitchButtonK(EObjectType cdktype GCC_UNUSED, void *object, void *clientData GCC_UNUSED, chtype key GCC_UNUSED);
int CreateDirK(EObjectType cdktype GCC_UNUSED, void *object, void *clientData GCC_UNUSED, chtype key GCC_UNUSED);
int ViewFile(char *file, char **buttons, int buttoncount, char *title);
void SetBottomLabel(char **msg, int count);
char *CreateText(const char *s, ...);
void FreeStrings(void);

extern CDKSCREEN *CDKScreen;
extern CDKLABEL *BottomLabel;
extern std::list<char *> StringList; // List of all strings created by CreateText, for easy removal :)

inline int dummyK(EObjectType cdktype GCC_UNUSED, void *object, void *clientData GCC_UNUSED,
                  chtype key GCC_UNUSED) { return true; };
int GetSpace(char *s1, char *s2);

#endif
