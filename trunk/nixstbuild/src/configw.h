#ifndef _CONFIGW_H
#define _CONFIGW_H


/*
loading a file is already done in luaparser btw
RunLua("nixstbuild.lua");
that wasn't so hard
NLua::LuaSet(m_OS, "osname", "os"); <-- Set a variable. Accepts most common var types
NLua::RegisterFunction(LuaInitDirIter, "dir", "io"); <-- Registers a regular Lua function (binding) last argument is optional and is used to define func in a lua package (so here you define io.dir)
RunLuaShared("main.lua"); <-- To run a shared file

NLua::CLuaTable tab("args"); <-- opens table "tab" then you can use the << and >> operators are direct indexes (also lots of types) at the end you must call tab.Close()
*and
eg tab[tabind] << argv[a];

main/frontend/luadepscreen.cpp <-- accessing subtables

you first need your big table with that LuaTable class
then you call tab[ind].GetTable(); To push the subtable on the stack
for good practise you can check the type: luaL_checktype(NLua::LuaState, -1, LUA_TTABLE); <-- -1 is the top index of the stack
it throws an error if it's not a table
NLua::CLuaTable tab2(-1); <-- to get your subtable

lua_pop(NLua::LuaState, 1); <-- to pop your table from the index when you are done with it
(this time 1 refers to 'first top')

you also probably need to include main/lua/lua.h and main/lua/luaclass.h

*/

#include <list>
#include <vector>

#include <QWidget>
#include <QLineEdit>
#include <QCheckBox>
#include <QGridLayout>
#include <QPushButton>
#include <QButtonGroup>

using namespace std;


class CVariableProvider: public QWidget
{
    Q_OBJECT
protected:
    string varname;
public:
    virtual string toString() = 0;
    string getName();
};

class CStringProvider: public CVariableProvider
{
protected:
    QLineEdit *lineEdit;
public:
    CStringProvider(string name);
    virtual string toString();
};

class CFileProvider: public CVariableProvider
{
protected:
    QLineEdit *lineEdit;
    QPushButton *openButton;
    string fdest;
public:
    CFileProvider(string name, string destination);
    virtual string toString();
};

class CChoiceProvider: public CVariableProvider
{
protected:
    vector<string> choicevec;
    QButtonGroup *buttonGroup;
public:
    CChoiceProvider(string name, vector<string> choices);
    virtual string toString();
};

class CBooleanProvider: public CVariableProvider
{
protected:
    QCheckBox *checkBox;
public:
    CBooleanProvider(string name);
    virtual string toString();
};

class CMultiChoiceProvider: public CVariableProvider
{
protected:
    vector<string> choicevec;
    QButtonGroup *buttonGroup;
public:
    CMultiChoiceProvider(string name, vector<string> choices);
    virtual string toString();
};

class QConfigWidget: public QWidget
{
    Q_OBJECT

    char *outputfile;
    vector<CVariableProvider*> providers;
    QGridLayout *grid;
public:
    QConfigWidget(QWidget *parent, char *lua_descrption, char *property_table, char *output);
    ~QConfigWidget();

    void buildConfig();
};


#endif
