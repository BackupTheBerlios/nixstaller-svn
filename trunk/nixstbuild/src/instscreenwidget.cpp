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

CInstScreenWidget::CInstScreenWidget(QWidget *parent,
                                     Qt::WindowFlags flags) : CTreeEdit(parent, flags),
                                                              gotDefaultSet(false)
{
    // UNDONE: Icons
    insertButton(0, addScreenB = new QPushButton("Add screen"));
    insertButton(1, newScreenB = new QPushButton("New screen"));

    QMenu *menu = new QMenu();
    QSignalMapper *sigMapper = new QSignalMapper;
    
    addScreenBMenuItem("Welcome screen", "WelcomeScreen", menu, sigMapper);
    addScreenBMenuItem("License screen", "LicenseScreen", menu, sigMapper);
    addScreenBMenuItem("Select destination screen", "SelectDirScreen", menu, sigMapper);
    addScreenBMenuItem("Package toggle screen", "PackageToggleScreen", menu, sigMapper);
    addScreenBMenuItem("Package destination screen", "PackageDirScreen", menu, sigMapper);
    addScreenBMenuItem("Install screen", "InstallScreen", menu, sigMapper);
    addScreenBMenuItem("Summary screen", "SummaryScreen", menu, sigMapper);
    addScreenBMenuItem("Finish screen", "FinishScreen", menu, sigMapper);
    
    connect(sigMapper, SIGNAL(mapped(const QString &)),
            this, SLOT(addScreen(const QString &)));
    addScreenB->setMenu(menu);

    connect(newScreenB, SIGNAL(clicked()), this, SLOT(newScreen()));
}

void CInstScreenWidget::addScreenBMenuItem(const QString &name,
                                           const QString &varname,
                                           QMenu *menu, QSignalMapper *mapper)
{
    QAction *a = menu->addAction(name);
    connect(a, SIGNAL(triggered()), mapper, SLOT(map()));
    mapper->setMapping(a, varname);
}

void CInstScreenWidget::deleteItems(const QString &name)
{
    QTreeWidgetItem *it;
    while((it = searchItem(name)))
        delete it;
}

void CInstScreenWidget::addScreen(const QString &name)
{
    if (!itemCount())
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
        QTreeWidgetItem *item = new QTreeWidgetItem(QStringList() << dialog.variableName().c_str());
        QVariant v;
        screenitem si(dialog.variableName(), dialog.screenTitle(), dialog.genCanActivate(),
                      dialog.genActivate(), dialog.genUpdate());
        dialog.getWidgets(si.widgets);
        v.setValue(si);
        item->setData(0, Qt::UserRole, v);
        addItem(item);
        selectItem(item);
    }
}

void CInstScreenWidget::setDefaults(bool pkg)
{

    if (!gotDefaultSet)
    {
        addItem(QStringList() << "WelcomeScreen");
        addItem(QStringList() << "LicenseScreen");
        addItem(QStringList() << "InstallScreen");
        addItem(QStringList() << "FinishScreen");
        gotDefaultSet = true;
    }

    if (pkg)
    {
        deleteItems("Select destination screen");

        QTreeWidgetItem *pts = searchItem("PackageToggleScreen");
        QTreeWidgetItem *pds = searchItem("PackageDirScreen");

        if (!pts)
        {
            int start = searchItemIndex("LicenseScreen");
            if (start == -1)
                start = searchItemIndex("WelcomeScreen");

            // searchItemIndex returns -1 if not found, so widget will be
            // inserted at start in that case
            start++;

            insertItem(start, QStringList() << "PackageToggleScreen");
            if (!pds)
                insertItem(start+1, QStringList() << "PackageDirScreen");
        }
        else if (!pds)
        {
            int start = searchItemIndex("PackageToggleScreen");
            // start _should_ be valid

            insertItem(start+1, QStringList() << "PackageDirScreen");
        }

        if (!searchItem("SummaryScreen"))
        {
            // Try to stuff summary screen between Install and Finish screens...
            int start = searchItemIndex("InstallScreen") + 1;
            if (start == 0) // Not found?
                start = searchItemIndex("FinishScreen");
            if (start == -1) // Still not found?
                start = itemCount(); // Add to end
            insertItem(start, QStringList() << "SummaryScreen");
        }
    }
    else
    {
        deleteItems("PackageToggleScreen");
        deleteItems("PackageDirScreen");
        deleteItems("SummaryScreen");

        if (!searchItem("SelectDirScreen"))
        {
            int start = searchItemIndex("LicenseScreen");
            if (start == -1)
                start = searchItemIndex("WelcomeScreen");

            // searchItemIndex returns -1 if not found, so widget will be
            // inserted at start in that case
            insertItem(start+1, QStringList() << "SelectDirScreen");
        }
    }
}

void CInstScreenWidget::getScreens(TStringVec &screenlist, screenvec &customs)
{
    const int size = itemCount();
    for (int i=0; i<size; i++)
    {
        QTreeWidgetItem *item = itemAt(i);
        screenlist.push_back(item->text(0).toStdString());
        QVariant v = item->data(0, Qt::UserRole);
        if (!v.isNull())
            customs.push_back(v.value<screenitem>());
    }
}
