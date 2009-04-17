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
#include <QListWidget>
#include <QListWidgetItem>
#include <QMenu>
#include <QPushButton>
#include <QSignalMapper>

#include "instscreenwidget.h"

CInstScreenWidget::CInstScreenWidget(QWidget *parent,
                                     Qt::WindowFlags flags) : QWidget(parent, flags),
                                                              gotDefaultSet(false)
{
    QHBoxLayout *hbox = new QHBoxLayout(this);

    hbox->addWidget(screenList = new QListWidget);

    // UNDONE: Icons
    QVBoxLayout *bbox = new QVBoxLayout;
    bbox->addWidget(addScreenB = new QPushButton("Add screen"));
    bbox->addWidget(newScreenB = new QPushButton("New screen"));
    bbox->addWidget(remScreenB = new QPushButton("Remove screen"));
    bbox->addWidget(upScreenB = new QPushButton("Move up"));
    bbox->addWidget(downScreenB = new QPushButton("Move down"));

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

    connect(remScreenB, SIGNAL(clicked()), this, SLOT(delScreen()));
    connect(upScreenB, SIGNAL(clicked()), this, SLOT(upScreen()));
    connect(downScreenB, SIGNAL(clicked()), this, SLOT(downScreen()));
    
    hbox->addLayout(bbox);
}

void CInstScreenWidget::enableEditButtons(bool e)
{
    remScreenB->setEnabled(e);
    upScreenB->setEnabled(e);
    downScreenB->setEnabled(e);
}

void CInstScreenWidget::addScreenBMenuItem(const QString &name, QMenu *menu, QSignalMapper *mapper)
{
    QAction *a = menu->addAction(name);
    connect(a, SIGNAL(triggered()), mapper, SLOT(map()));
    mapper->setMapping(a, name);
}

QListWidgetItem *CInstScreenWidget::searchItem(const QString &name)
{
    const int size = screenList->count();
    for (int i=0; i<size; i++)
    {
        if (screenList->item(i)->text() == name)
            return screenList->item(i);
    }
    
    return NULL;
}

int CInstScreenWidget::searchItemRow(const QString &name)
{
    const int size = screenList->count();
    for (int i=0; i<size; i++)
    {
        if (screenList->item(i)->text() == name)
            return i;
    }
    
    return -1;
}

void CInstScreenWidget::deleteItems(const QString &name)
{
    QListWidgetItem *it;
    while((it = searchItem(name)))
        delete it;
}

void CInstScreenWidget::addScreen(const QString &name)
{
    new QListWidgetItem(name, screenList);
    if (screenList->count() == 1)
    {
        enableEditButtons(true);
        screenList->setCurrentRow(0);
    }
}

void CInstScreenWidget::delScreen()
{
    if (screenList->count() == 1)
        enableEditButtons(false);
    
    // currentItem() returns NULL when list is empty, so delete should always be save
    delete screenList->currentItem();
}

void CInstScreenWidget::upScreen()
{
    int row = screenList->currentRow();
    if (!screenList->count() || !row)
        return;
    screenList->insertItem(row-1, screenList->takeItem(row));
    screenList->setCurrentRow(row-1);
}

void CInstScreenWidget::downScreen()
{
    int row = screenList->currentRow();
    if (!screenList->count() || (row == screenList->count()-1))
        return;
    screenList->insertItem(row+1, screenList->takeItem(row));
    screenList->setCurrentRow(row+1);
}

void CInstScreenWidget::setDefaults(bool pkg)
{
    if (!gotDefaultSet)
    {
        new QListWidgetItem("Welcome screen", screenList);
        new QListWidgetItem("License screen", screenList);
        new QListWidgetItem("Install screen", screenList);
        new QListWidgetItem("Finish screen", screenList);
        gotDefaultSet = true;
    }

    if (pkg)
    {
        deleteItems("Select destination screen");

        QListWidgetItem *pds = searchItem("Package destination screen");
        QListWidgetItem *pts = searchItem("Package toggle screen");

        if (!pds)
        {
            int start = searchItemRow("License screen");
            if (start == -1)
                start = searchItemRow("Welcome screen");

            // searchItemRow returns -1 if not found, so widget will be
            // inserted at start in that case
            start++;

            screenList->insertItem(start, "Package destination screen");
            if (!pts)
                screenList->insertItem(start+1, "Package toggle screen");
        }
        else if (!pts)
        {
            int start = searchItemRow("Package destination screen");
            // start _should_ be valid

            screenList->insertItem(start+1, "Package toggle screen");
        }

        if (!searchItem("Summary screen"))
        {
            // Try to stuff summary screen between Install and Finish screens...
            int start = searchItemRow("Install screen") + 1;
            if (start == 0) // Not found?
                start = searchItemRow("Finish screen");
            if (start == -1) // Still not found?
                start = screenList->count(); // Add to end
            screenList->insertItem(start, "Summary screen");
        }
    }
    else
    {
        deleteItems("Package destination screen");
        deleteItems("Package toggle screen");
        deleteItems("Summary screen");

        if (!searchItem("Select destination screen"))
        {
            int start = searchItemRow("License screen");
            if (start == -1)
                start = searchItemRow("Welcome screen");

            // searchItemRow returns -1 if not found, so widget will be
            // inserted at start in that case
            screenList->insertItem(start+1, "Select destination screen");
        }
    }
    
    enableEditButtons(true);
}
