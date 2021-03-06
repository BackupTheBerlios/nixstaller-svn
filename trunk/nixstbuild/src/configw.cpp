#include <QApplication>
#include <QRadioButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
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

QConfigWidget::QConfigWidget(QWidget *parent, const char *lua_description, const char *property_table, const char *output): QWidget(parent)
{
    grid = new QGridLayout();
    setLayout(grid);

    outputfile = output;

    try
    {
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
                string vname;
                CVariableProvider *current;
                descr["var"] >> vname;
                providers.push_back(current = new CFileProvider(vname, ""));
                grid->addWidget(current, i, 1);                
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
            } else if (val == "choice")
            {
                string vname;
                CVariableProvider *current;
                vector<string> choices;
                descr["var"] >> vname;

                descr["choices"].GetTable();
                CLuaTable choicet(-1);

                for (int j = 1; j <= choicet.Size(); j++)
                {
                    string newval;
                    choicet[j] >> newval;
                    choices.push_back(newval);
                }
                
                choicet.Close();
                lua_pop(LuaState, 1);
                
                providers.push_back(current = new CChoiceProvider(vname, choices));
                grid->addWidget(current, i, 1);
            } else if (val == "multi-choice")
            {
                string vname;
                CVariableProvider *current;
                vector<string> choices;
                descr["var"] >> vname;

                descr["choices"].GetTable();
                CLuaTable choicet(-1);

                for (int j = 1; j <= choicet.Size(); j++)
                {
                    string newval;
                    choicet[j] >> newval;
                    choices.push_back(newval);
                }
                
                choicet.Close();
                lua_pop(LuaState, 1);
                
                providers.push_back(current = new CMultiChoiceProvider(vname, choices));
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
    luaout.open(outputfile.c_str(), fstream::out);

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

CChoiceProvider::CChoiceProvider(string name, vector<string> choices)
{
    vector<string>::iterator iter;

    choicevec = choices;
    varname = name;

    buttonGroup = new QButtonGroup(this);
    setLayout(new QVBoxLayout());

    for (iter = choices.begin(); iter != choices.end(); iter++)
    {
        QRadioButton *newbutton = new QRadioButton((*iter).c_str());
        buttonGroup->addButton(newbutton);
        layout()->addWidget(newbutton);
    }
}

string CChoiceProvider::toString()
{
    if (buttonGroup->checkedButton() != 0)
    {
        return varname + " = \"" + buttonGroup->checkedButton()->text().toStdString() + "\"";
    } else return string("");
}

CMultiChoiceProvider::CMultiChoiceProvider(string name, vector<string> choices)
{
    vector<string>::iterator iter;

    varname = name;

    setLayout(new QVBoxLayout());

    for (iter = choices.begin(); iter != choices.end(); iter++)
    {
        QCheckBox *newbutton = new QCheckBox((*iter).c_str());
        choicevec.push_back(newbutton);
        layout()->addWidget(newbutton);
    }
}

string CMultiChoiceProvider::toString()
{
    bool notfirst = false;

    vector<QCheckBox*>::iterator iter;
    string ret = varname + " = {";

    for (iter = choicevec.begin(); iter != choicevec.end(); iter++)
    {
        if ((*iter)->isChecked())
        {
            if (!notfirst)
            {
                ret.append("\"" + (*iter)->text().toStdString() + "\"");
                notfirst = true;
            } else {
                ret.append(",\"" + (*iter)->text().toStdString() + "\"");
            }
        }
    }

    ret.append("}");
    return ret;
}

CFileProvider::CFileProvider(string name, string dest)
{
    varname = name;

    lineEdit = new QLineEdit();
    openButton = new QPushButton("Choose...");

    setLayout(new QHBoxLayout());
    layout()->addWidget(lineEdit);
    layout()->addWidget(openButton);
}

string CFileProvider::toString()
{
    return "";
}



