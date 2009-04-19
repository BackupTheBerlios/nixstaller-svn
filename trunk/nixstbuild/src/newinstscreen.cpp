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
#include <QListWidget>
#include <QMenu>
#include <QPushButton>
#include <QSignalMapper>
#include <QSpinBox>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

#include "main/main.h"
#include "main/lua/lua.h"
#include "main/lua/luatable.h"
#include "newinstscreen.h"

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

    form->addRow("Variable name (no spaces)", new QLineEdit);
    form->addRow("Screen title", new QLineEdit);
    form->addRow("Generate canactivate() function", new QCheckBox);
    form->addRow("Generate activate() function", new QCheckBox);
    form->addRow("Generate update() function", new QCheckBox);
    
    return box;
}

QWidget *CNewScreenDialog::createWidgetGroup()
{
    QGroupBox *box = new QGroupBox(tr("Widgets"), this);
    QHBoxLayout *hbox = new QHBoxLayout(box);

    hbox->addWidget(widgetList = new QListWidget);

    // UNDONE: Icons
    QVBoxLayout *bbox = new QVBoxLayout;
    bbox->addWidget(addWidgetB = new QPushButton("Add"));
    bbox->addWidget(remWidgetB = new QPushButton("Remove"));
    bbox->addWidget(upWidgetB = new QPushButton("Move up"));
    bbox->addWidget(downWidgetB = new QPushButton("Move down"));

    loadWidgetTab();
    QMenu *menu = new QMenu;
    QSignalMapper *sigMapper = new QSignalMapper;
    
    for (widgetmap::iterator it=widgetMap.begin(); it!=widgetMap.end(); it++)
        addWidgetBMenuItem(it->first.c_str(), menu, sigMapper);

    connect(sigMapper, SIGNAL(mapped(const QString &)),
            this, SLOT(addWidgetItem(const QString &)));
    
    addWidgetB->setMenu(menu);
    
    hbox->addLayout(bbox);

    enableEditButtons(false);
    
    return box;
}

void CNewScreenDialog::enableEditButtons(bool e)
{
    remWidgetB->setEnabled(e);
    upWidgetB->setEnabled(e);
    downWidgetB->setEnabled(e);
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
    std::vector<CBaseWidgetField *> fieldVec;

    lua_rawgeti(NLua::LuaState, LUA_REGISTRYINDEX, widgetMap[name.toLatin1().data()]);
    NLua::CLuaTable tab(-1);
    if (tab)
    {
        std::string k;
        
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
        
        lua_pop(NLua::LuaState, 1); // tab["fields"]
    }

    lua_pop(NLua::LuaState, 1); // tab

    QDialogButtonBox *bbox = new QDialogButtonBox(QDialogButtonBox::Ok |
            QDialogButtonBox::Cancel);
    connect(bbox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    connect(bbox, SIGNAL(rejected()), &dialog, SLOT(reject()));
    form->addWidget(bbox);
    
    if (dialog.exec() == QDialog::Accepted)
    {
        new QListWidgetItem(name, widgetList);

        QString code;
        while (!fieldVec.empty())
        {
            CBaseWidgetField *fw = fieldVec.back();
            fieldVec.pop_back();
            delete fw;
        }
        
        if (widgetList->count() == 1)
        {
            enableEditButtons(true);
            widgetList->setCurrentRow(0);
        }
    }
}

void CNewScreenDialog::remWidgetItem()
{
    if (widgetList->count() == 1)
        enableEditButtons(false);
    
    // currentItem() returns NULL when list is empty, so delete should always be save
    delete widgetList->currentItem();
}

void CNewScreenDialog::upWidgetItem()
{
    int row = widgetList->currentRow();
    if (!widgetList->count() || !row)
        return;
    widgetList->insertItem(row-1, widgetList->takeItem(row));
    widgetList->setCurrentRow(row-1);
}

void CNewScreenDialog::downWidgetItem()
{
    int row = widgetList->currentRow();
    if (!widgetList->count() || (row == widgetList->count()-1))
        return;
    widgetList->insertItem(row+1, widgetList->takeItem(row));
    widgetList->setCurrentRow(row+1);
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

CListWidgetField::CListWidgetField(NLua::CLuaTable &field, QWidget *parent,
                                   Qt::WindowFlags f) : QWidget(parent, f)
{
    QHBoxLayout *hbox = new QHBoxLayout(this);
    
    optTree = new QTreeWidget;
    optTree->setRootIsDecorated(false);
    optTree->setHeaderLabels(QStringList() << "Option" << "Default enabled");
    hbox->addWidget(optTree);

    // UNDONE: Icons
    QVBoxLayout *bbox = new QVBoxLayout;
    bbox->addWidget(addOptB = new QPushButton("Add"));
    bbox->addWidget(remOptB = new QPushButton("Remove"));
    bbox->addWidget(upOptB = new QPushButton("Move up"));
    bbox->addWidget(downOptB = new QPushButton("Move down"));

    connect(addOptB, SIGNAL(clicked()), this, SLOT(addOptItem()));
    connect(remOptB, SIGNAL(clicked()), this, SLOT(remOptItem()));
    connect(upOptB, SIGNAL(clicked()), this, SLOT(upOptItem()));
    connect(downOptB, SIGNAL(clicked()), this, SLOT(downOptItem()));
    
    hbox->addLayout(bbox);
}

void CListWidgetField::enableEditButtons(bool e)
{
    remOptB->setEnabled(e);
    upOptB->setEnabled(e);
    downOptB->setEnabled(e);
}

void CListWidgetField::addOptItem()
{
    QTreeWidgetItem *item = new QTreeWidgetItem;
    item->setText(0, "Name");
    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
    optTree->addTopLevelItem(item);
    optTree->setItemWidget(item, 1, new QCheckBox);
    optTree->editItem(item);
    
    enableEditButtons(true);
}

void CListWidgetField::remOptItem()
{
    if (optTree->topLevelItemCount() == 1)
        enableEditButtons(false);
    
    QTreeWidgetItem *cur = optTree->currentItem();
    optTree->removeItemWidget(cur, 1);
    delete cur;
}

void CListWidgetField::upOptItem()
{
    QTreeWidgetItem *cur = optTree->currentItem();
    int index = optTree->indexOfTopLevelItem(cur);
    if (!optTree->topLevelItemCount() || !index)
        return;
    optTree->insertTopLevelItem(index-1, optTree->takeTopLevelItem(index));
    optTree->setCurrentItem(cur);
}

void CListWidgetField::downOptItem()
{
    QTreeWidgetItem *cur = optTree->currentItem();
    int index = optTree->indexOfTopLevelItem(cur);
    if (!optTree->topLevelItemCount() || (index == optTree->topLevelItemCount()-1))
        return;
    optTree->insertTopLevelItem(index+1, optTree->takeTopLevelItem(index));
    optTree->setCurrentItem(cur);
}
