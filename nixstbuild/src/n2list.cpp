/***************************************************************************
 *   Copyright (C) 2007 by InternetNightmare   *
 *   internetnightmare@gmail.com   *
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

#include <QPushButton>
#include <QTreeWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "n2list.h"

inline QTreeWidgetItem *newItem(QTreeWidget *parent, char *caption, char *caption2)
{
    QTreeWidgetItem *nitem = new QTreeWidgetItem();
    nitem->setText(0, caption);
    nitem->setText(1, caption2);
    nitem->setFlags(nitem->flags() | Qt::ItemIsEditable);
    return nitem;
}

N2List::N2List()
 : QWidget()
{
    QVBoxLayout *vlayout = new QVBoxLayout(this);
    QHBoxLayout *hlayout = new QHBoxLayout;
    paramlist = new QTreeWidget;
    badd = new QPushButton("Add");
    bremove = new QPushButton("Remove");
    hlayout->addWidget(badd);
    hlayout->addWidget(bremove);
    vlayout->addWidget(paramlist);
    vlayout->addLayout(hlayout);

    connect(badd, SIGNAL(clicked()), this, SLOT(iadd()));
    connect(bremove, SIGNAL(clicked()), this, SLOT(iremove()));
    connect(paramlist, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(iedit(QTreeWidgetItem*,int)));
}

N2List::~N2List()
{
}

void N2List::setHeaderLabels(QStringList labellist)
{
    paramlist->setHeaderLabels(labellist);
}

QStringList *N2List::getValues()
{
    QStringList *ret = new QStringList;

    for (int i = 0; i < paramlist->topLevelItemCount(); i++)
    {
        *ret << paramlist->topLevelItem(i)->text(1);
        *ret << paramlist->topLevelItem(i)->text(0);
    }
    return ret;
}

void N2List::iadd()
{
    paramlist->addTopLevelItem(newItem(paramlist, "p1", "p2"));
}

void N2List::iremove()
{
    if (paramlist->topLevelItemCount() > 0)
        paramlist->takeTopLevelItem(paramlist->indexOfTopLevelItem(paramlist->currentItem()));
}

void N2List::iedit(QTreeWidgetItem *item, int column)
{
    paramlist->editItem(item, column);
}

