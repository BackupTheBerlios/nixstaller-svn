#ifndef NCURS_H
#define NCURS_H

#include <vector>
#include <list>
#include <sstream>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>

#include <cdk/cdk.h>
#include "main.h"

#define NO_FILE -1
#define ESCAPE  -2

#define DEFAULT_WIDTH       50
#define DEFAULT_HEIGHT      12

void EndProg(void);
int ReadDir(std::string &dir, char ***list);
int SwitchButtonK(EObjectType cdktype GCC_UNUSED, void *object, void *clientData, chtype key GCC_UNUSED);
int CreateDirK(EObjectType cdktype GCC_UNUSED, void *object, void *clientData, chtype key);
int ScrollParamMenuK(EObjectType cdktype, void *object, void *clientData, chtype key);
int ExitK(EObjectType cdktype GCC_UNUSED, void *object, void *clientData GCC_UNUSED, chtype key);
int ViewFile(char *file, char **buttons, int buttoncount, char *title);
void SetBottomLabel(char **msg, int count);
void WarningBox(const char *msg, ...);
bool YesNoBox(const char *msg, ...);

inline int dummyK(EObjectType cdktype GCC_UNUSED, void *object, void *clientData GCC_UNUSED,
                  chtype key GCC_UNUSED) { return true; };

class CCharListHelper
{
    std::list<char *> m_Items;
    char **m_pCList;
    bool m_bModified;
    
    typedef std::list<char *>::iterator LI;
    
public:
    CCharListHelper(void) : m_pCList(NULL), m_bModified(false) { };
    ~CCharListHelper(void) { Clear(); };
     
    operator char**(void) { return GetArray(); };
    char *operator [](int n) { return GetArray()[n]; }
    
    void AddItem(char *str) { m_Items.push_back(strdup(str)); m_bModified = true; };
    void AddItem(const std::string &str) { m_Items.push_back(strdup(str.c_str())); m_bModified = true; };
    void Clear(void) { for(LI it=m_Items.begin();it!=m_Items.end();it++) free(*it); m_Items.clear(); delete [] m_pCList;
                       m_pCList = NULL; };
    int Count(void) { return m_Items.size(); };
    char **GetArray(void)
    {
        if (!m_pCList || m_bModified)
        {
            if (m_pCList) delete [] m_pCList;
            m_pCList=new char*[m_Items.size()+1];
            int i=0;
            for(LI it=m_Items.begin(); it!=m_Items.end(); it++) m_pCList[i++] = *it;
            m_pCList[i] = NULL;
            m_bModified = false;
        }
        return m_pCList;
    };
};

#include "widgets.h"

extern WINDOW *MainWin;
extern CDKSCREEN *CDKScreen;
extern CCDKLabel *BottomLabel;

inline void PrintInstOutput(const char *msg, void *p) { ((CCDKSWindow *)p)->AddText(msg, false); };

#endif
