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

#include <QHBoxLayout>
#include <QMenu>
#include <QPushButton>
#include <QSignalMapper>
#include <QTreeWidgetItem>

#include "instscreenwidget.h"
#include "newinstscreen.h"

CInstScreenWidget::CInstScreenWidget(QWidget *parent,
                                     Qt::WindowFlags flags) : CTreeEdit(parent, flags),
                                                              gotDefaultSet(false)
{
    // UNDONE: Icons
    insertButton(0, addScreenB = new QPushButton("Add screen"));
    insertButton(1, newScreenB = new QPushButton("New screen"));

    QMenu *menu = new QMenu();
    QSignalMapper *sigMapper = new QSignalMapper;
    
    addScreenBMenuItem("Welcome screen", menu, sigMapper);
    addScreenBMenuItem("License screen", menu, sigMapper);
    addScreenBMenuItem("Select destination screen", menu, sigMapper);
    addScreenBMenuItem("Package destination screen", menu, sigMapper);
    addScreenBMenuItem("Package toggle screen", menu, sigMapper);
    addScreenBMenuItem("Install screen", menu, sigMapper);
    addScreenBMenuItem("Summary screen", menu, sigMapper);
    addScreenBMenuItem("Finish screen", menu, sigMapper);
    
    connect(sigMapper, SIGNAL(mapped(const QString &)),
            this, SLOT(addScreen(const QString &)));
    addScreenB->setMenu(menu);

    connect(newScreenB, SIGNAL(clicked()), this, SLOT(newScreen()));
}

void CInstScreenWidget::addScreenBMenuItem(const QString &name, QMenu *menu, QSignalMapper *mapper)
{
    QAction *a = menu->addAction(name);
    connect(a, SIGNAL(triggered()), mapper, SLOT(map()));
    mapper->setMapping(a, name);
}

void CInstScreenWidget::deleteItems(const QString &name)
{
    QTreeWidgetItem *it;
    while((it = searchItem(name)))
        delete it;
}

void CInstScreenWidget::addScreen(const QString &name)
{
    if (!count())
        addItem(QStringList() << name);
    else
    {
        insertItem(currentItemIndex()+1, QStringList() << name);
        selectItem(currentItemIndex()+1);
    }
}

void CInstScreenWidget::newScreen()
{
    CNewScreenDialog dialog;
    if (dialog.exec() == QDialog::Accepted)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(QStringList() << dialog.variableName());
        addItem(item);
        selectItem(item);
    }
}

void CInstScreenWidget::setDefaults(bool pkg)
{

    if (!gotDefaultSet)
    {
        addItem(QStringList() << "Welcome screen");
        addItem(QStringList() << "License screen");
        addItem(QStringList() << "Install screen");
        addItem(QStringList() << "Finish screen");
        gotDefaultSet = true;
    }

    if (pkg)
    {
        deleteItems("Select destination screen");

        QTreeWidgetItem *pds = searchItem("Package destination screen");
        QTreeWidgetItem *pts = searchItem("Package toggle screen");

        if (!pds)
        {
            int start = searchItemIndex("License screen");
            if (start == -1)
                start = searchItemIndex("Welcome screen");

            // searchItemIndex returns -1 if not found, so widget will be
            // inserted at start in that case
            start++;

            insertItem(start, QStringList() << "Package destination screen");
            if (!pts)
                insertItem(start+1, QStringList() << "Package toggle screen");
        }
        else if (!pts)
        {
            int start = searchItemIndex("Package destination screen");
            // start _should_ be valid

            insertItem(start+1, QStringList() << "Package toggle screen");
        }

        if (!searchItem("Summary screen"))
        {
            // Try to stuff summary screen between Install and Finish screens...
            int start = searchItemIndex("Install screen") + 1;
            if (start == 0) // Not found?
                start = searchItemIndex("Finish screen");
            if (start == -1) // Still not found?
                start = count(); // Add to end
            insertItem(start, QStringList() << "Summary screen");
        }
    }
    else
    {
        deleteItems("Package destination screen");
        deleteItems("Package toggle screen");
        deleteItems("Summary screen");

        if (!searchItem("Select destination screen"))
        {
            int start = searchItemIndex("License screen");
            if (start == -1)
                start = searchItemIndex("Welcome screen");

            // searchItemIndex returns -1 if not found, so widget will be
            // inserted at start in that case
            insertItem(start+1, QStringList() << "Select destination screen");
        }
    }
}
