#ifndef NCURS_H
#define NCURS_H

#include <list>

#include <cdk/cdk.h>
#include "main.h"

#define NO_FILE -1
#define ESCAPE  -2

#define DEFAULT_WIDTH       50
#define DEFAULT_HEIGHT      12

class CBaseScreen
{
public:
    CBaseScreen(void) { };
    
    virtual bool Activate(void) { return true; };
    virtual bool Next(void) { return true; };
    virtual bool Prev(void) { return true; };
};

class CLangScreen: public CBaseScreen
{
public:
};

void throwerror(const char *error, ...);
int ReadDir(const char *dir, char ***list);
int SwitchButtonK(EObjectType cdktype GCC_UNUSED, void *object, void *clientData GCC_UNUSED, chtype key GCC_UNUSED);
int CreateDirK(EObjectType cdktype GCC_UNUSED, void *object, void *clientData GCC_UNUSED, chtype key GCC_UNUSED);
int ScrollParamMenuK(EObjectType cdktype GCC_UNUSED, void *object, void *clientData GCC_UNUSED, chtype key GCC_UNUSED);
int ViewFile(char *file, char **buttons, int buttoncount, char *title);
void SetBottomLabel(char **msg, int count);

extern WINDOW *MainWin;
extern CDKSCREEN *CDKScreen;
extern CDKLABEL *BottomLabel;

inline int dummyK(EObjectType cdktype GCC_UNUSED, void *object, void *clientData GCC_UNUSED,
                  chtype key GCC_UNUSED) { return true; };
int GetSpace(char *s1, char *s2);

#endif
