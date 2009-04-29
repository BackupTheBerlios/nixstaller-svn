/***************************************************************************
 *   Copyright (C) 2009 by Rick Helmus / InternetNightmare   *
 *   rhelmus_AT_gmail.com / internetnightmare_AT_gmail.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QTreeWidgetItem>

#include "main/lua/luatable.h"
#include "newinstwidget.h"
#include "utils.h"

namespace {

typedef std::map<std::string, int> widgetmap;

widgetmap &getWidgetMap(void)
{
    static widgetmap widgetMap;
    static bool loaded = false;

    if (!loaded)
    {
        loaded = true;
        NLua::CLuaTable wtab("widget_properties");

        if (wtab)
        {
            std::string key;
            while (wtab.Next(key))
            {
                wtab[key].GetTable();
                luaL_checktype(NLua::LuaState, -1, LUA_TTABLE);
                widgetMap[key] = NLua::MakeReference(-1);
                lua_pop(NLua::LuaState, 1);
            }
        }
    }

    return widgetMap;
}

}

CNewWidgetDialog::CNewWidgetDialog(NLua::CLuaTable &table, QWidget *parent,
                                   Qt::WindowFlags f) : QDialog(parent, f)
{
    setModal(true);

    QFormLayout *form = new QFormLayout(this);

    form->addRow("Variable name", nameField = createLuaEdit());
    
    table["fields"].GetTable();
    luaL_checktype(NLua::LuaState, -1, LUA_TTABLE);
    NLua::CLuaTable fields(-1);
    const int size = fields.Size();

    for (int n=1; n<=size; n++)
    {
        fields[n].GetTable();
        luaL_checktype(NLua::LuaState, -1, LUA_TTABLE);
        NLua::CLuaTable field(-1);

        std::string n, type;
        field["name"] >> n;
        field["type"] >> type;

        CBaseWidgetField *fw = NULL;

        if (type == "string")
            fw = new CStringWidgetField(field);
        else if (type == "int")
            fw = new CIntWidgetField(field);
        else if (type == "choice")
            fw = new CChoiceWidgetField(field);
        else if (type == "list")
            fw = new CListWidgetField(field);
        else if (type == "bool")
            fw = new CCheckWidgetField(field);

        assert(fw);

        form->addRow(n.c_str(), fw->getFieldWidget());
        fieldVec.push_back(fw);

        lua_pop(NLua::LuaState, 1); // fields[n]
    }

    lua_pop(NLua::LuaState, 1); // table["fields"]

    QDialogButtonBox *bbox = new QDialogButtonBox(QDialogButtonBox::Ok |
            QDialogButtonBox::Cancel);
    connect(bbox, SIGNAL(accepted()), this, SLOT(OK()));
    connect(bbox, SIGNAL(rejected()), this, SLOT(reject()));
    form->addWidget(bbox);
}

CNewWidgetDialog::~CNewWidgetDialog()
{
    while (!fieldVec.empty())
    {
        delete fieldVec.back();
        fieldVec.pop_back();
    }
}

void CNewWidgetDialog::OK()
{
    if (verifyLuaEdit(nameField))
        accept();
}

std::string CNewWidgetDialog::getVarName(void) const
{
    return nameField->text().toStdString();
}

void CNewWidgetDialog::getArgs(TStringVec &vec) const
{
    for (std::vector<CBaseWidgetField *>::const_iterator it=fieldVec.begin();
         it!=fieldVec.end(); it++)
        (*it)->addArgs(vec);
}

CStringWidgetField::CStringWidgetField(NLua::CLuaTable &field) : emptyNil(false)
{
    lineEdit = new QLineEdit;

    if (field["default"])
    {
        const char *def;
        field["default"] >> def;
        lineEdit->setText(def);
    }

    if (field["emptynil"])
        field["emptynil"] >> emptyNil;
}

QWidget *CStringWidgetField::getFieldWidget()
{
    return lineEdit;
}

void CStringWidgetField::addArgs(TStringVec &vec)
{
    std::string s = lineEdit->text().toStdString();
    if (emptyNil && s.empty())
        vec.push_back("nil");
    else
        vec.push_back("\"" + s + "\"");
}

CIntWidgetField::CIntWidgetField(NLua::CLuaTable &field)
{
    spinBox = new QSpinBox;
    spinBox->setMaximum(INT_MAX);

    if (field["default"])
    {
        int def;
        field["default"] >> def;
        spinBox->setValue(def);
    }
}

QWidget *CIntWidgetField::getFieldWidget()
{
    return spinBox;
}

void CIntWidgetField::addArgs(TStringVec &vec)
{
    std::string arg;
    arg += spinBox->value();
    vec.push_back(arg);
}

CChoiceWidgetField::CChoiceWidgetField(NLua::CLuaTable &field)
{
    comboBox = new QComboBox;

    field["options"].GetTable();
    luaL_checktype(NLua::LuaState, -1, LUA_TTABLE);
    NLua::CLuaTable options(-1);
    const int size = options.Size();

    for (int n=1; n<=size; n++)
    {
        const char *o;
        options[n] >> o;
        comboBox->addItem(o);
    }
    
    lua_pop(NLua::LuaState, 1); // options

    if (field["default"])
    {
        int def;
        field["default"] >> def;
        comboBox->setCurrentIndex(def-1);
    }
}

QWidget *CChoiceWidgetField::getFieldWidget()
{
    return comboBox;
}

void CChoiceWidgetField::addArgs(TStringVec &vec)
{
    vec.push_back("\"" + comboBox->currentText().toStdString() + "\"");
}

CListWidgetField::CListWidgetField(NLua::CLuaTable &field)
{
    optList = new CTreeEdit;
    
    optList->setHeader(QStringList() << "Option" << "Default enabled");

    // UNDONE: Icons
    optList->insertButton(0, addOptB = new QPushButton("Add"));

    std::string type;
    field["listtype"] >> type;
    multiChoice = (type == "multi");

    boxGroup = new QButtonGroup(this);
    boxGroup->setExclusive(!multiChoice);
    
    connect(addOptB, SIGNAL(clicked()), this, SLOT(addOptItem()));
}

void CListWidgetField::addOptItem()
{
    QTreeWidgetItem *item = new QTreeWidgetItem;
    item->setText(0, "Name");
    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);

    optList->insertAtCurrent(item);

    QAbstractButton *but;
    if (multiChoice)
        but = new QCheckBox;
    else
        but = new QRadioButton;
    
    boxGroup->addButton(but);
    optList->getTree()->setItemWidget(item, 1, but);
    optList->getTree()->editItem(item);
}

QWidget *CListWidgetField::getFieldWidget()
{
    return optList;
}

void CListWidgetField::addArgs(TStringVec &vec)
{
    const int size = optList->itemCount();
    std::string opts, defs;
    
    for (int i=0; i<size; i++)
    {
        QTreeWidgetItem *it = optList->itemAt(i);

        if (!opts.empty())
            opts += ", ";

        opts += "\"" + it->text(0).toStdString() + "\"";

        QAbstractButton *but = dynamic_cast<QAbstractButton*>(optList->getTree()->itemWidget(it, 1));
        if (but && but->isChecked())
        {
            if (multiChoice && !defs.empty())
                defs += ", ";
            defs += "\"" + it->text(0).toStdString() + "\"";
        }
    }

    vec.push_back("{ " + opts + " }");

    if (multiChoice)
        vec.push_back("{ " + defs + " }");
    else
        vec.push_back(defs);
}

CCheckWidgetField::CCheckWidgetField(NLua::CLuaTable &field)
{
    checkBox = new QCheckBox;

    if (field["default"])
    {
        bool def;
        field["default"] >> def;
        checkBox->setChecked(def);
    }
}

QWidget *CCheckWidgetField::getFieldWidget()
{
    return checkBox;
}

void CCheckWidgetField::addArgs(TStringVec &vec)
{
    vec.push_back(checkBox->isChecked() ? "true" : "false");
}

bool newInstWidget(const std::string &name, widgetitem &out)
{
    bool ret = true;
    
    lua_rawgeti(NLua::LuaState, LUA_REGISTRYINDEX, getWidgetMap()[name]);
    NLua::CLuaTable tab(-1);

    if (tab["fields"])
    {
        CNewWidgetDialog dialog(tab);
        
        if (dialog.exec() == QDialog::Accepted)
        {
            out.variable = dialog.getVarName();
            dialog.getArgs(out.args);
            tab["func"] >> out.func;
        }
        else
            ret = false;
    }
    else // No fields (ie. screenend)
    {
        out.variable.clear();
        out.args.clear();
        tab["func"] >> out.func;
    }

    lua_pop(NLua::LuaState, 1); // tab

    return ret;
}

void getWidgetTypes(TStringVec &out)
{
    const widgetmap &wm = getWidgetMap();
    for(widgetmap::const_iterator it=wm.begin(); it!=wm.end(); it++)
        out.push_back(it->first);
}
