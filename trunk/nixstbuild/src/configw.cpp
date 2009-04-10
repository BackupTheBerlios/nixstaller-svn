#include <QApplication>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLineEdit>
#include <QWidget>
#include <QLabel>

#include <iostream>
#include <string>

using namespace std;

#include "main/exception.h"
#include "main/lua/lua.h"
#include "main/lua/luatable.h"
#include "configw.h"
#include "luaparser.h"

using namespace NLua;

QConfigWidget::QConfigWidget(QWidget *parent, char *lua_description, char *property_table, char *output): QWidget(parent)
{

    CLuaParser luaparser;

    grid = new QGridLayout();
    setLayout(grid);

    outputfile = output;

    try
    {
        luaparser.Init(0, 0);
        luaparser.PRunLua(lua_description);

        CLuaTable props(property_table);

        string newkey;

        for (int i=1; i <= props.Size(); i++)
        {
            props[i].GetTable();
            luaL_checktype(LuaState, -1, LUA_TTABLE);
            
            CLuaTable descr(-1);

            string val;
            descr["name"] >> val;
            cout << val << endl;
            grid->addWidget(new QLabel(val.c_str()), i, 0);
            descr["ptype"] >> val;
            
            if (val == "file")
            {
                
            } else if (val == "string")
            {
                string vname;
                CVariableProvider *current;
                descr["var"] >> vname;
                providers.push_back(current = new CStringProvider(vname));
                grid->addWidget(current, i, 1);
            } else if (val == "boolean")
            {
                string vname;
                CVariableProvider *current;
                descr["var"] >> vname;
                providers.push_back(current = new CBooleanProvider(vname));
                grid->addWidget(current, i, 1);
            }

            descr.Close();
            lua_pop(LuaState, 1);
       }
    }

    catch (Exceptions::CException &e)
    {
        cerr << "luaparser: " << e.what() << endl;
    }
}

QConfigWidget::~QConfigWidget()
{
}

void QConfigWidget::buildConfig()
{
    vector<CVariableProvider*>::iterator iter;
    fstream luaout;
    luaout.open(outputfile, fstream::out);

    for (iter = providers.begin(); iter != providers.end(); iter++)
    {
        luaout << (*iter)->toString() << endl;
    }

    luaout.close();
}

string CVariableProvider::getName()
{
    return varname;
}

CStringProvider::CStringProvider(string name)
{
    varname = name;
    lineEdit = new QLineEdit(this);
    setLayout(new QHBoxLayout());
    layout()->addWidget(lineEdit);
}

string CStringProvider::toString()
{
    return varname + " = \"" + lineEdit->text().toStdString() + "\"";
}

CBooleanProvider::CBooleanProvider(string name)
{
    checkBox = new QCheckBox();
    setLayout(new QHBoxLayout);
    layout()->addWidget(checkBox);
    varname = name;
}

string CBooleanProvider::toString()
{
    return varname + " = " + (checkBox->isChecked() ? "true" : "false");
}





