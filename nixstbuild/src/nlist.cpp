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

#include <QHBoxLayout>
#include <QComboBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QStringList>
#include <QInputDialog>


#include "nlist.h"


NList::NList()
 : QWidget()
{
    badd = new QPushButton("Add");
    bremove = new QPushButton("Remove");
    list = new QComboBox;
    layout = new QHBoxLayout(this);
    layout->addWidget(list);
    layout->addWidget(badd);
    layout->addWidget(bremove);

    connect(badd, SIGNAL(clicked()), this, SLOT(add()));
    connect(bremove, SIGNAL(clicked()), this, SLOT(remove()));
}

NList::~NList()
{
}

void NList::add()
{
    bool ok;
    QString text = QInputDialog::getText(this, "Input", "Item name:", QLineEdit::Normal, "", &ok);

    if (ok && !text.isEmpty())
    {
        list->addItem(text);
    }
}

void NList::remove()
{
    list->removeItem(list->currentIndex());
}

QStringList NList::getList()
{
    QStringList rlist;

    for (int i = 0; i < list->count(); i++)
    {
        rlist << list->itemText(i);
    }
    return rlist;
}


