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

#include <algorithm>

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QVBoxLayout>

#include "main/lua/luafunc.h"
#include "main/lua/luatable.h"
#include "configwidget.h"

CConfigWidget::CConfigWidget(QWidget *parent,
                             Qt::WindowFlags flags) : QWidget(parent, flags)
{
    QFormLayout *form = new QFormLayout(this);
    
    NLua::CLuaTable tab("configGenProperties"); // UNDONE: Specify
    if (tab)
    {
        std::string var;
        while (tab.Next(var))
        {
            NLua::CLuaTable vtab;
            tab[var] >> vtab;

            std::string type;
            vtab["type"] >> type;

            CBaseConfValue *confValue = NULL;
            if (type == "string")
                confValue = new CStringConfValue(vtab);
            else if (type == "choice")
                confValue = new CChoiceConfValue(vtab);
            else if (type == "multichoice")
                confValue = new CMultiChoiceConfValue(vtab);
            else if (type == "boolean")
                confValue = new CBoolConfValue(vtab);

            if (confValue)
            {
                std::string name;
                vtab["name"] >> name;
                form->addRow(name.c_str(), confValue);
                valueWidgetMap[var] = confValue;
            }
        }
    }
}

void CConfigWidget::loadConfig(const std::string &file)
{
    NLua::CLuaFunc func("loadGenConf");
    assert(func);
    if (func)
    {
        func << file;
        func(0);
    }

    for (TValueWidgetMap::iterator it=valueWidgetMap.begin();
         it!=valueWidgetMap.end(); it++)
        it->second->loadValue();
}

void CConfigWidget::saveConfig(const std::string &file)
{
    for (TValueWidgetMap::iterator it=valueWidgetMap.begin();
         it!=valueWidgetMap.end(); it++)
        it->second->pushValue();

    NLua::CLuaFunc func("saveGenConf");
    assert(func);
    if (func)
    {
        func << file;
        func(0);
    }
}

void CConfigWidget::newConfig(const std::string &file)
{
    NLua::CLuaFunc func("newGenConf");
    assert(func);
    if (func)
    {
        func << file;
        func(0);
    }

    // Clear widgets
    for (TValueWidgetMap::iterator it=valueWidgetMap.begin();
         it!=valueWidgetMap.end(); it++)
        it->second->clearWidget();

    // Load new (default) file
    loadConfig(file);
}


CStringConfValue::CStringConfValue(const NLua::CLuaTable &luat, QWidget *parent,
                                   Qt::WindowFlags flags) : CBaseConfValue(luat, parent, flags)
{
    QVBoxLayout *vbox = new QVBoxLayout(this);
    vbox->addWidget(lineEdit = new QLineEdit);
}

void CStringConfValue::coreLoadValue()
{
    std::string val;
    getLuaValueTable()["value"] >> val;
    lineEdit->setText(val.c_str());
}

void CStringConfValue::corePushValue()
{
    getLuaValueTable()["value"] << lineEdit->text().toStdString();
}

void CStringConfValue::coreClearWidget()
{
    lineEdit->setText("");
}


CChoiceConfValue::CChoiceConfValue(const NLua::CLuaTable &luat, QWidget *parent,
                                   Qt::WindowFlags flags) : CBaseConfValue(luat, parent, flags)
{
    QVBoxLayout *vbox = new QVBoxLayout(this);
    vbox->addWidget(comboBox = new QComboBox);

    TStringVec l;
    getLuaValueTable()["choices"] >> l;
    for (TStringVec::const_iterator it=l.begin(); it!=l.end(); it++)
        comboBox->addItem(it->c_str());
}

void CChoiceConfValue::coreLoadValue()
{
    std::string val;
    getLuaValueTable()["value"] >> val;
    
    int index = comboBox->findText(val.c_str());
    if (index != -1)
        comboBox->setCurrentIndex(index);
}

void CChoiceConfValue::corePushValue(void)
{
    getLuaValueTable()["value"] << comboBox->currentText().toStdString();
}

void CChoiceConfValue::coreClearWidget()
{
    comboBox->setCurrentIndex(0);
}


CMultiChoiceConfValue::CMultiChoiceConfValue(const NLua::CLuaTable &luat,
                                             QWidget *parent,
                                             Qt::WindowFlags flags) : CBaseConfValue(luat, parent, flags)
{
    QVBoxLayout *vbox = new QVBoxLayout(this);

    TStringVec l;
    getLuaValueTable()["choices"] >> l;
    for (TStringVec::const_iterator it=l.begin(); it!=l.end(); it++)
    {
        QCheckBox *box = new QCheckBox(it->c_str());
        vbox->addWidget(box);
        choiceList.push_back(box);
    }
}

void CMultiChoiceConfValue::coreLoadValue()
{
    TStringVec val;
    getLuaValueTable()["value"] >> val;

    for (TChoiceList::iterator it=choiceList.begin(); it!=choiceList.end(); it++)
        (*it)->setChecked(std::find(val.begin(), val.end(),
          (*it)->text().toStdString()) != val.end());
}

void CMultiChoiceConfValue::corePushValue(void)
{
    TStringVec val;
    for (TChoiceList::iterator it=choiceList.begin(); it!=choiceList.end(); it++)
    {
        if ((*it)->isChecked())
            val.push_back((*it)->text().toStdString());
    }
    
    getLuaValueTable()["value"] << val;
}

void CMultiChoiceConfValue::coreClearWidget()
{
    while (!choiceList.empty())
    {
        delete choiceList.back();
        choiceList.pop_back();
    }
}


CBoolConfValue::CBoolConfValue(const NLua::CLuaTable &luat, QWidget *parent,
                               Qt::WindowFlags flags) : CBaseConfValue(luat, parent, flags)
{
    QVBoxLayout *vbox = new QVBoxLayout(this);
    vbox->addWidget(checkBox = new QCheckBox);
}

void CBoolConfValue::coreLoadValue()
{
    bool v;
    getLuaValueTable()["value"] >> v;
    checkBox->setChecked(v);
}

void CBoolConfValue::corePushValue()
{
    getLuaValueTable()["value"] << checkBox->isChecked();
}

void CBoolConfValue::coreClearWidget()
{
    checkBox->setChecked(false);
}
