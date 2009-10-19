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
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

#include "main/lua/luafunc.h"
#include "configwidget.h"
#include "extrafilesdialog.h"
#include "treeedit.h"

CConfigWidget::CConfigWidget(const char *proptab, QWidget *parent,
                             Qt::WindowFlags flags) : QWidget(parent, flags)
{
    QFormLayout *form = new QFormLayout(this);
    
    NLua::CLuaTable tab(proptab);
    if (tab)
    {
        std::string var;
        const int size = tab.Size();
        for (int i=1; i<=size; i++)
        {
            NLua::CLuaTable vtab;
            tab[i] >> vtab;

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
            else if (type == "file")
                confValue = new CFileConfValue(vtab);
            else if (type == "unopts")
                confValue = new CUnOptsConfValue(vtab);

            if (confValue)
            {
                connect(confValue, SIGNAL(confValueChanged(NLua::CLuaTable &)),
                        this, SLOT(valueChangedCB(NLua::CLuaTable &)));
                std::string name;
                vtab["name"] >> name;

                // Big widgets
                if (type == "unopts")
                {
                    // Put label above
                    form->addWidget(new QLabel(name.c_str()));
                    form->addWidget(confValue);
                }
                else
                    form->addRow(name.c_str(), confValue);
                
                valueWidgetList.push_back(confValue);
            }
        }
    }
}

void CConfigWidget::valueChangedCB(NLua::CLuaTable &t)
{
    emit confValueChanged(t);
}

void CConfigWidget::loadConfig(const std::string &dir)
{
    NLua::CLuaFunc func("loadGenConf");
    assert(func);
    if (func)
    {
        func << dir;
        func(0);
    }

    for (TValueWidgetList::iterator it=valueWidgetList.begin();
         it!=valueWidgetList.end(); it++)
    {
        (*it)->setProjectDir(dir);
        (*it)->loadValue();
    }
}

void CConfigWidget::saveConfig(const std::string &dir)
{
    for (TValueWidgetList::iterator it=valueWidgetList.begin();
         it!=valueWidgetList.end(); it++)
    {
        (*it)->setProjectDir(dir);
        (*it)->pushValue();
    }

    NLua::CLuaFunc func("saveGenConf");
    assert(func);
    if (func)
    {
        func << dir;
        func(0);
    }
}

void CConfigWidget::newConfig(const std::string &dir)
{
    NLua::CLuaFunc func("newGenConf");
    assert(func);
    if (func)
    {
        func << dir;
        func(0);
    }

    // Clear widgets
    for (TValueWidgetList::iterator it=valueWidgetList.begin();
         it!=valueWidgetList.end(); it++)
    {
        (*it)->setProjectDir(dir);
        (*it)->clearWidget();
    }

    // Load new (default) file
    loadConfig(dir);
}


void CBaseConfValue::valueChangedCB()
{
    corePushValue();
    emit confValueChanged(luaValueTable);
}


CStringConfValue::CStringConfValue(const NLua::CLuaTable &luat, QWidget *parent,
                                   Qt::WindowFlags flags) : CBaseConfValue(luat, parent, flags)
{
    QVBoxLayout *vbox = new QVBoxLayout(this);
    vbox->addWidget(lineEdit = new QLineEdit);
    connect(lineEdit, SIGNAL(textEdited(const QString &)), this,
            SLOT(valueChangedCB()));
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

    connect(comboBox, SIGNAL(currentIndexChanged(const QString &)), this,
            SLOT(valueChangedCB()));
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
    TStringVec l;
    getLuaValueTable()["choices"] >> l;

    QGridLayout *grid = new QGridLayout(this);
    int row = 0, col = 0;
    for (TStringVec::const_iterator it=l.begin(); it!=l.end(); it++)
    {
        QCheckBox *box = new QCheckBox(it->c_str());
        connect(box, SIGNAL(stateChanged(int)), this, SLOT(valueChangedCB()));
        grid->addWidget(box, row, col);

        if (col == 0)
            col++;
        else
        {
            row++;
            col = 0;
        }
        
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
    connect(checkBox, SIGNAL(stateChanged(int)), this, SLOT(valueChangedCB()));
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


CFileConfValue::CFileConfValue(const NLua::CLuaTable &luat, QWidget *parent,
                               Qt::WindowFlags flags) : CBaseConfValue(luat, parent, flags)
{
    QHBoxLayout *hbox = new QHBoxLayout(this);

    hbox->addWidget(fileView = new QLineEdit);
    fileView->setReadOnly(true);
    
    QPushButton *button = new QPushButton("Modify");
    connect(button, SIGNAL(clicked()), this, SLOT(modifyCB()));
    hbox->addWidget(button);
}

void CFileConfValue::coreLoadValue()
{
    if (getLuaValueTable()["value"])
    {
        std::string v;
        getLuaValueTable()["value"] >> v;
        fileView->setText(v.c_str());
    }
}

void CFileConfValue::corePushValue()
{
    if (!fileView->text().isEmpty())
        getLuaValueTable()["value"] << fileView->text().toStdString();
}

void CFileConfValue::coreClearWidget()
{
    fileView->setText("");
}

void CFileConfValue::coreSetProjectDir(const std::string &dir)
{
    projectDir = dir;
}

void CFileConfValue::modifyCB()
{
    CExtraFilesDialog dialog(projectDir.c_str());
    if (dialog.exec() == QDialog::Accepted)
    {
        fileView->setText(dialog.file());
    }
}


CUnOptsConfValue::CUnOptsConfValue(const NLua::CLuaTable &luat, QWidget *parent,
                                   Qt::WindowFlags flags) : CBaseConfValue(luat, parent, flags)
{
    QVBoxLayout *vbox = new QVBoxLayout(this);
    
    vbox->addWidget(treeEdit = new CTreeEdit(false));
    treeEdit->setHeader(QStringList() << "Name" << "Short" << "Type" << "Variable name");
    treeEdit->insertButton(0, addButton = new QPushButton("Add"));
    connect(addButton, SIGNAL(clicked()), this, SLOT(addCB()));
}

void CUnOptsConfValue::coreLoadValue()
{
/*    bool v;
    getLuaValueTable()["value"] >> v;
    checkBox->setChecked(v);*/
}

void CUnOptsConfValue::corePushValue()
{
//     getLuaValueTable()["value"] << checkBox->isChecked();
}

void CUnOptsConfValue::coreClearWidget()
{
//     checkBox->setChecked(false);
}

void CUnOptsConfValue::addCB()
{
    QDialog dialog;
    dialog.setModal(true);
    QFormLayout *form = new QFormLayout(&dialog);

    QLineEdit *nameField = new QLineEdit;
    form->addRow("Name", nameField);

    QLineEdit *shortField = new QLineEdit;
    shortField->setMaxLength(1);
    form->addRow("Short", shortField);

    QComboBox *typeField = new QComboBox;
    typeField->addItems(QStringList() << "None" << "String" << "List");
    form->addRow("Variable type", typeField);

    QLineEdit *varField = new QLineEdit;
    form->addRow("Variable name", varField);

    // UNDONE: Input verification
    
    QDialogButtonBox *bbox = new QDialogButtonBox(QDialogButtonBox::Ok |
            QDialogButtonBox::Cancel);
    
    connect(bbox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    connect(bbox, SIGNAL(rejected()), &dialog, SLOT(reject()));
    form->addWidget(bbox);

    if (dialog.exec() == QDialog::Accepted)
    {
        treeEdit->addItem(QStringList() << nameField->text() << shortField->text() <<
                typeField->currentText() << varField->text());
        valueChangedCB();
    }
}
