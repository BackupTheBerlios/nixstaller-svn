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

#include <QApplication>
#include <QDesktopWidget>
#include <QPushButton>
#include <QVBoxLayout>

#include "welcome.h"
#include "expert.h"

CWelcomeScreen::CWelcomeScreen(QWidget *parent,
                               Qt::WindowFlags flags) : QMainWindow(parent, flags)
{
    setWindowTitle("Nixstbuild v0.1");

    QDesktopWidget *d = QApplication::desktop();
    int w = d->width();     // returns desktop width
    int h = d->height();    // returns desktop height

    move((w - width())/2, (h - height())/2);

    QWidget *cw = new QWidget;
    setCentralWidget(cw);
    
    QVBoxLayout *vbox = new QVBoxLayout(cw);

    standardButton = new QPushButton("Standard Mode");
    connect(standardButton, SIGNAL(clicked()), this, SLOT(standardSlot()));
    vbox->addWidget(standardButton);
    
    expertButton = new QPushButton("Expert Mode");
    connect(expertButton, SIGNAL(clicked()), this, SLOT(expertSlot()));
    vbox->addWidget(expertButton);
}

void CWelcomeScreen::standardSlot()
{
//     CNixstbuildStandard *nbstandard = new CNixstbuildStandard();
//     nbstandard->show();

//     hide();
}

void CWelcomeScreen::expertSlot()
{
    (new CExpertScreen)->show();
    hide();
}
