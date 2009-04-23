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

#include <assert.h>

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>
#include <QSignalMapper>
#include <QSpinBox>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

#include "main/main.h"
#include "main/lua/lua.h"
#include "main/lua/luatable.h"
#include "newinstscreen.h"
#include "utils.h"

CNewScreenDialog::CNewScreenDialog(QWidget *parent,
                                   Qt::WindowFlags flags) : QDialog(parent, flags)
{
    setModal(true);

    QVBoxLayout *vbox = new QVBoxLayout(this);

    vbox->addWidget(createMainGroup());
    vbox->addWidget(createWidgetGroup());
    
    QDialogButtonBox *bbox = new QDialogButtonBox(QDialogButtonBox::Ok |
            QDialogButtonBox::Cancel);
    connect(bbox, SIGNAL(accepted()), this, SLOT(OK()));
    connect(bbox, SIGNAL(rejected()), this, SLOT(cancel()));
    vbox->addWidget(bbox);
}

CNewScreenDialog::~CNewScreenDialog()
{
    for (widgetmap::iterator it=widgetMap.begin(); it!=widgetMap.end(); it++)
        NLua::Unreference(it->second);
}

QWidget *CNewScreenDialog::createMainGroup()
{
    QGroupBox *box = new QGroupBox(tr("Main settings"), this);
    QFormLayout *form = new QFormLayout(box);

    form->addRow("Variable name", varEdit = createLuaEdit());
    form->addRow("Screen title", titleEdit = new QLineEdit);
    form->addRow("Generate canactivate() function", canActBox = new QCheckBox);
    form->addRow("Generate activate() function", actBox = new QCheckBox);
    form->addRow("Generate update() function", updateBox = new QCheckBox);
    
    return box;
}

QWidget *CNewScreenDialog::createWidgetGroup()
{
    QGroupBox *box = new QGroupBox(tr("Widgets"), this);
    QHBoxLayout *hbox = new QHBoxLayout(box);

    hbox->addWidget(widgetList = new CTreeEdit);
    widgetList->setHeader(QStringList() << "Name" << "Type");

    // UNDONE: Icons
    widgetList->insertButton(0, addWidgetB = new QPushButton("Add"));

    loadWidgetTab();
    QMenu *menu = new QMenu;
    QSignalMapper *sigMapper = new QSignalMapper;
    
    for (widgetmap::iterator it=widgetMap.begin(); it!=widgetMap.end(); it++)
        addWidgetBMenuItem(it->first.c_str(), menu, sigMapper);

    connect(sigMapper, SIGNAL(mapped(const QString &)),
            this, SLOT(addWidgetItem(const QString &)));
    
    addWidgetB->setMenu(menu);
    
    return box;
}

void CNewScreenDialog::loadWidgetTab()
{
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

void CNewScreenDialog::addWidgetBMenuItem(const QString &name, QMenu *menu, QSignalMapper *mapper)
{
    QAction *a = menu->addAction(name);
    connect(a, SIGNAL(triggered()), mapper, SLOT(map()));
    mapper->setMapping(a, name);
}

void CNewScreenDialog::OK()
{
    // UNDONE: Check fields
    accept();
}

void CNewScreenDialog::cancel()
{
    reject();
}

void CNewScreenDialog::addWidgetItem(const QString &name)
{
    QDialog dialog;
    dialog.setModal(true);
    QFormLayout *form = new QFormLayout(&dialog);

    QLineEdit *nameField = createLuaEdit();
    form->addRow("Variable name", nameField);
    
    std::vector<CBaseWidgetField *> fieldVec;

    lua_rawgeti(NLua::LuaState, LUA_REGISTRYINDEX, widgetMap[name.toLatin1().data()]);
    NLua::CLuaTable tab(-1);
    
    tab["fields"].GetTable();
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

        assert(fw);

        form->addRow(n.c_str(), fw->getFieldWidget());
        fieldVec.push_back(fw);

        lua_pop(NLua::LuaState, 1); // fields[n]
    }

    lua_pop(NLua::LuaState, 2); // tab["fields"] and tab

    QDialogButtonBox *bbox = new QDialogButtonBox(QDialogButtonBox::Ok |
            QDialogButtonBox::Cancel);
    connect(bbox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    connect(bbox, SIGNAL(rejected()), &dialog, SLOT(reject()));
    form->addWidget(bbox);
    
    if (dialog.exec() == QDialog::Accepted)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(QStringList() << nameField->text() << name);
        widgetList->addItem(item);

        QStringList args;
        while (!fieldVec.empty())
        {
            CBaseWidgetField *fw = fieldVec.front();
            args << fw->getArg();
            fieldVec.erase(fieldVec.begin());
            delete fw;
        }

        const char *func;
        tab["func"] >> func;
        
        QVariant v;
        v.setValue(widgetitem(func, nameField->text(), args));
        item->setData(0, Qt::UserRole, v);
    }
}

QString CNewScreenDialog::variableName() const
{
    return varEdit->text();
}

QString CNewScreenDialog::screenTitle() const
{
    return titleEdit->text();
}

bool CNewScreenDialog::genCanActivate() const
{
    return canActBox->isChecked();
}

bool CNewScreenDialog::genActivate() const
{
    return actBox->isChecked();
}

bool CNewScreenDialog::genUpdate() const
{
    return updateBox->isChecked();
}

CStringWidgetField::CStringWidgetField(NLua::CLuaTable &field)
{
    lineEdit = new QLineEdit;
    
    if (field["default"])
    {
        const char *def;
        field["default"] >> def;
        lineEdit->setText(def);
    }
}

QWidget *CStringWidgetField::getFieldWidget()
{
    return lineEdit;
}

QString CStringWidgetField::getArg()
{
    return "\"" + lineEdit->text() + "\"";
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

QString CIntWidgetField::getArg()
{
    return QString::number(spinBox->value());
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

QString CChoiceWidgetField::getArg()
{
    return "\"" + comboBox->currentText() + "\"";
}

CListWidgetField::CListWidgetField(NLua::CLuaTable &field, QWidget *parent,
                                   Qt::WindowFlags f) : CTreeEdit(parent, f)
{
    setHeader(QStringList() << "Option" << "Default enabled");

    // UNDONE: Icons
    insertButton(0, addOptB = new QPushButton("Add"));

    connect(addOptB, SIGNAL(clicked()), this, SLOT(addOptItem()));
}

void CListWidgetField::addOptItem()
{
    QTreeWidgetItem *item = new QTreeWidgetItem;
    item->setText(0, "Name");
    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
    addItem(item);
    getTree()->setItemWidget(item, 1, new QCheckBox);
    getTree()->editItem(item);
}

QString CListWidgetField::getArg()
{
    const int size = count();
    QString ret;
    for (int i=0; i<size; i++)
    {
        QTreeWidgetItem *it = itemAt(i);

        if (!ret.isEmpty())
            ret += ", ";

        ret += "\"" + it->text(0) + "\"";
    }
    
    return "{ " + ret + " }";
}
