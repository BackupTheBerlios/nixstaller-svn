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
#include <QRadioButton>
#include <QSignalMapper>
#include <QSpinBox>
#include <QTreeWidgetItem>
#include <QVariant>
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

    QMenu *menu = new QMenu(addWidgetB);
    QSignalMapper *sigMapper = new QSignalMapper(box);

    TStringVec types;
    getWidgetTypes(types);
    for (TStringVec::iterator it=types.begin(); it!=types.end(); it++)
        addWidgetBMenuItem(it->c_str(), menu, sigMapper);

    connect(sigMapper, SIGNAL(mapped(const QString &)),
            this, SLOT(addWidgetItem(const QString &)));
    
    addWidgetB->setMenu(menu);
    
    return box;
}

void CNewScreenDialog::addWidgetBMenuItem(const QString &name, QMenu *menu, QSignalMapper *mapper)
{
    QAction *a = menu->addAction(name);
    connect(a, SIGNAL(triggered()), mapper, SLOT(map()));
    mapper->setMapping(a, name);
}

void CNewScreenDialog::OK()
{
    if (verifyLuaEdit(varEdit))
        accept();
}

void CNewScreenDialog::cancel()
{
    reject();
}

void CNewScreenDialog::addWidgetItem(const QString &name)
{
    widgetitem wit;
    if (newInstWidget(name.toStdString(), wit))
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(QStringList() << wit.variable.c_str() << name);
        widgetList->insertAtCurrent(item);
        QVariant v;
        v.setValue(wit);
        item->setData(0, Qt::UserRole, v);
    }
}

std::string CNewScreenDialog::variableName() const
{
    return varEdit->text().toStdString();
}

std::string CNewScreenDialog::screenTitle() const
{
    return titleEdit->text().toStdString();
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

void CNewScreenDialog::getWidgets(std::vector<widgetitem> &out) const
{
    const int size = widgetList->itemCount();
    for (int i=0; i<size; i++)
        out.push_back(widgetList->itemAt(i)->data(0, Qt::UserRole).value<widgetitem>());
}
