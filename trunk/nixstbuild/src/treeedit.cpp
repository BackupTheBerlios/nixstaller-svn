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
#include <QPushButton>
#include <QTreeWidget>
#include <QVBoxLayout>

#include "treeedit.h"

CTreeEdit::CTreeEdit(QWidget *parent,
                     Qt::WindowFlags flags) : QWidget(parent, flags)
{
    QHBoxLayout *hbox = new QHBoxLayout(this);

    hbox->addWidget(tree = new QTreeWidget);
    tree->setRootIsDecorated(false);
    tree->setHeaderHidden(true);

    // UNDONE: Icons
    buttonLayout = new QVBoxLayout;
    buttonLayout->addWidget(remB = new QPushButton("Remove"));
    buttonLayout->addWidget(upB = new QPushButton("Move up"));
    buttonLayout->addWidget(downB = new QPushButton("Move down"));

    connect(remB, SIGNAL(clicked()), this, SLOT(del()));
    connect(upB, SIGNAL(clicked()), this, SLOT(up()));
    connect(downB, SIGNAL(clicked()), this, SLOT(down()));
    
    hbox->addLayout(buttonLayout);
}

void CTreeEdit::enableEditButtons(bool e)
{
    remB->setEnabled(e);
    upB->setEnabled(e);
    downB->setEnabled(e);
}

void CTreeEdit::del()
{
    if (tree->topLevelItemCount() == 1)
        enableEditButtons(false);

    QTreeWidgetItem *cur = tree->currentItem();
    if (cur)
    {
        tree->removeItemWidget(cur, 1);
        delete cur;
    }
}

void CTreeEdit::up()
{
    QTreeWidgetItem *cur = tree->currentItem();
    int index = tree->indexOfTopLevelItem(cur);
    if (!tree->topLevelItemCount() || !index)
        return;
    tree->insertTopLevelItem(index-1, tree->takeTopLevelItem(index));
    tree->setCurrentItem(cur);
}

void CTreeEdit::down()
{
    QTreeWidgetItem *cur = tree->currentItem();
    int index = tree->indexOfTopLevelItem(cur);
    if (!tree->topLevelItemCount() || (index == tree->topLevelItemCount()-1))
        return;
    tree->insertTopLevelItem(index+1, tree->takeTopLevelItem(index));
    tree->setCurrentItem(cur);
}

void CTreeEdit::setHeader(const QStringList &labels)
{
    tree->setHeaderHidden(false);
    tree->setHeaderLabels(labels);
}

void CTreeEdit::addItem(QTreeWidgetItem *item)
{
    tree->addTopLevelItem(item);
    if (tree->topLevelItemCount() == 1)
    {
        enableEditButtons(true);
        tree->setCurrentItem(item);
    }
}

void CTreeEdit::addItem(const QStringList &items)
{
    addItem(new QTreeWidgetItem(items));
}

void CTreeEdit::insertItem(int index, QTreeWidgetItem *item)
{
    tree->insertTopLevelItem(index, item);
    if (tree->topLevelItemCount() == 1)
    {
        enableEditButtons(true);
        tree->setCurrentItem(item);
    }
}

void CTreeEdit::insertItem(int index, const QStringList &items)
{
    insertItem(index, new QTreeWidgetItem(items));
}

QTreeWidgetItem *CTreeEdit::currentItem(void) const
{
    return tree->currentItem();
}

int CTreeEdit::currentItemIndex(void) const
{
    return tree->indexOfTopLevelItem(tree->currentItem());
}

void CTreeEdit::selectItem(QTreeWidgetItem *item)
{
    tree->setCurrentItem(item);
}

void CTreeEdit::selectItem(int index)
{
    tree->setCurrentItem(tree->topLevelItem(index));
}

QTreeWidgetItem *CTreeEdit::searchItem(const QString &name, int column)
{
    const int size = tree->topLevelItemCount();
    for (int i=0; i<size; i++)
    {
        if (tree->topLevelItem(i)->text(column) == name)
            return tree->topLevelItem(i);
    }
    
    return NULL;
}

int CTreeEdit::searchItemIndex(const QString &name, int column)
{
    const int size = tree->topLevelItemCount();
    for (int i=0; i<size; i++)
    {
        if (tree->topLevelItem(i)->text(column) == name)
            return i;
    }
    
    return -1;
}

QTreeWidgetItem *CTreeEdit::itemAt(int index)
{
    return tree->topLevelItem(index);
}

int CTreeEdit::itemCount() const
{
    return tree->topLevelItemCount();
}

void CTreeEdit::insertButton(int index, QPushButton *button)
{
    buttonLayout->insertWidget(index, button);
}
