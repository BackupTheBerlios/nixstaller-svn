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

#include <string>

using namespace std;

#include <Qt>
#include <QtGui>

#include "rungen.h"

#include <QFrame>
#include <QDialog>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>

const char *base_init = "function Init()\n\tScreenList = { WelcomeScreen, SelectDirScreen, InstallScreen }\nend\n\n";
const char *base_install = "function Install()\n\tinstall.extractfiles()\nend\n\n";

NBRunGen::NBRunGen(QWidget *parent): QDialog(parent)
{
    QVBoxLayout *mainlay = new QVBoxLayout(this);
    QHBoxLayout *toplay = new QHBoxLayout;
    winlist = new QListWidget;
    winlist->addItem("Init");
    winlist->addItem("Install");
    winlist->setMaximumSize(128, 1280);

    qstack = new QStackedWidget;
    init_widget = new QWidget;
    install_widget = new QWidget;

    toplay->addWidget(winlist);
    toplay->addWidget(qstack);
    toplay->setAlignment(Qt::AlignLeft);

    QHBoxLayout *btns = new QHBoxLayout();
    btns->setAlignment(Qt::AlignRight);
    QPushButton *bok = new QPushButton("OK");
    QPushButton *bcancel = new QPushButton("Cancel");
    btns->addWidget(bok);
    btns->addWidget(bcancel);

    connect(bok, SIGNAL(clicked()), this, SLOT(sOK()));
    connect(bcancel, SIGNAL(clicked()), this, SLOT(sCancel()));
    connect(winlist, SIGNAL(currentRowChanged(int)), this, SLOT(sShow(int)));

    mainlay->addLayout(toplay);
    mainlay->addLayout(btns);

    setupInit();
    setupInstall();

}

void NBRunGen::setupInit()
{
    QVBoxLayout *lay = new QVBoxLayout(init_widget);
    QFrame *topframe = new QFrame;
    topframe->setFrameStyle(QFrame::Panel | QFrame::Plain);
    topframe->setLineWidth(1);

    lay->addWidget(topframe);
    QVBoxLayout *clay = new QVBoxLayout(topframe);
    init = new QCheckBox("Generate Init()");
    clay->addWidget(init);
    clay->addSpacing(4);
    QFrame *restframe = new QFrame;
    restframe->setFrameStyle(QFrame::Panel | QFrame::Plain);
    restframe->setLineWidth(1);
/*
    // Configure init
    QVBoxLayout *conflayout = new QVBoxLayout;
    QHBoxLayout *listlay = new QHBoxLayout;
    screenList = new QListWidget;
    screenList->addItem("WelcomeScreen");
    screenList->addItem("SelectDirScreen");
    screenList->addItem("InstallScreen");
    listlay->addWidget(screenList);
    QVBoxLayout *btn_lay = new QVBoxLayout;
    QPushButton *bup = new QPushButton("Up");
    QPushButton *bdown = new QPushButton("Down");

    connect(bup, SIGNAL(clicked()), this, SLOT(sBUp()));
    connect(bdown, SIGNAL(clicked()), this, SLOT(sBDown()));
*/
    lay->addWidget(restframe);
    lay->setAlignment(Qt::AlignTop);

    qstack->addWidget(init_widget);
}

void NBRunGen::setupInstall()
{
    QVBoxLayout *lay = new QVBoxLayout(install_widget);
    QFrame *topframe = new QFrame;
    topframe->setFrameStyle(QFrame::Panel | QFrame::Plain);
    topframe->setLineWidth(1);

    lay->addWidget(topframe);
    QVBoxLayout *clay = new QVBoxLayout(topframe);
    install = new QCheckBox("Generate Install()");
    clay->addWidget(install);
    clay->addSpacing(4);
    QFrame *restframe = new QFrame;
    restframe->setFrameStyle(QFrame::Panel | QFrame::Plain);
    restframe->setLineWidth(1);

    lay->addWidget(restframe);
    lay->setAlignment(Qt::AlignTop);

    qstack->addWidget(install_widget);
}

void NBRunGen::sOK()
{
    if (init->isChecked())
    {
        gscript = base_init;
    }
    if (install->isChecked())
    {
        gscript += base_install;
    }

    accept();
}

void NBRunGen::sCancel()
{
    reject();
}

void NBRunGen::sShow(int row)
{
    //qstack->setCurrentIndex(winlist->currentRow());
    qstack->setCurrentIndex(row);
}

string NBRunGen::script()
{
    return gscript;
}

void NBRunGen::sBUp()
{
}

void NBRunGen::sBDown()
{
}

