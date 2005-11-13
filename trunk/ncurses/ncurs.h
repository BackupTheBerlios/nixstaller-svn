#ifndef NCURS_H
#define NCURS_H

#include <list>
#include <sstream>

#include <cdk/cdk.h>
#include "main.h"
#include "widgets.h"

#define NO_FILE -1
#define ESCAPE  -2

#define DEFAULT_WIDTH       50
#define DEFAULT_HEIGHT      12

void throwerror(const char *error, ...);
int ReadDir(const char *dir, char ***list);
int SwitchButtonK(EObjectType cdktype GCC_UNUSED, void *object, void *clientData GCC_UNUSED, chtype key GCC_UNUSED);
int CreateDirK(EObjectType cdktype GCC_UNUSED, void *object, void *clientData GCC_UNUSED, chtype key GCC_UNUSED);
int ScrollParamMenuK(EObjectType cdktype GCC_UNUSED, void *object, void *clientData GCC_UNUSED, chtype key GCC_UNUSED);
int ViewFile(char *file, char **buttons, int buttoncount, char *title);
void SetBottomLabel(char **msg, int count);

extern WINDOW *MainWin;
extern CDKSCREEN *CDKScreen;
extern CCDKLabel *BottomLabel;

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
    ~CCharListHelper(void) { for(LI it=m_Items.begin(); it!=m_Items.end(); it++) free(*it); };
     
    operator char**(void)
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
    
    void AddItem(char *str) { m_Items.push_back(strdup(str)); m_bModified = true; };
    void AddItem(const std::string &str) { m_Items.push_back(strdup(str.c_str())); m_bModified = true; };
    int Count(void) { return m_Items.size(); };
};

#endif
