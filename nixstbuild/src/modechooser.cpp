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

#include <QtGui>
#include <QLabel>
#include <QVBoxLayout>
#include <QMainWindow>
#include <QPushButton>

#include "nixstbuild.h"
#include "nbstandard.h"

#include "modechooser.h"

CModeChooser::CModeChooser(QWidget * parent = 0, Qt::WindowFlags flags = 0)
{
    setWindowTitle("Nixstbuild v0.1");

    QDesktopWidget *d = QApplication::desktop();
    int w = d->width();     // returns desktop width
    int h = d->height();    // returns desktop height

    move((w - width())/2, (h - height())/2);

    QWidget *centralWidget = new QWidget();
    setCentralWidget(centralWidget);

    standardModeB = new QPushButton("Standard Mode");
    expertModeB = new QPushButton("Expert Mode");
    standardModeB->setEnabled(false);

    connect(standardModeB, SIGNAL(clicked()), this, SLOT(standardMode()));
    connect(expertModeB, SIGNAL(clicked()), this, SLOT(expertMode()));

    QVBoxLayout *layout = new QVBoxLayout(this);
    centralWidget->setLayout(layout);

    layout->addWidget(standardModeB);
    layout->addWidget(expertModeB);
    layout->addWidget(new QLabel("It is recommended to use Standard Mode"));
}

void CModeChooser::standardMode()
{
    CNixstbuildStandard *nbstandard = new CNixstbuildStandard();
    nbstandard->show();

    hide();
}

void CModeChooser::expertMode()
{
    nixstbuild * mw = new nixstbuild();
    mw->show();

    hide();
}

