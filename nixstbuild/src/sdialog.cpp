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

#include <Qt>
#include <QtGui>

#include <QLabel>
#include <QDialog>
#include <QGroupBox>
#include <QLineEdit>
#include <QSettings>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QFileDialog>

#include "sdialog.h"

NBSettingsDialog::NBSettingsDialog(QWidget *parent): QDialog(parent)
{
    QSettings settings("INightmare", "Nixstbuilder");
    setModal(true);
    QVBoxLayout *vbox = new QVBoxLayout(this);

    QFrame *frame = new QFrame;
    frame->setFrameStyle(QFrame::Plain | QFrame::Panel);
    frame->setLineWidth(1);

    QHBoxLayout *olay = new QHBoxLayout(frame);
    olay->addWidget(new QLabel("geninstall.sh location:"));
    genPath = new QLineEdit;
    genPath->setText(settings.value("geninstall").toString());
    olay->addWidget(genPath);
    genPathOpen = new QPushButton(QIcon(":/fileopen.xpm"), "", this);
    olay->addWidget(genPathOpen);
    connect(genPathOpen, SIGNAL(clicked()), this, SLOT(showOpen()));

    QHBoxLayout *hbox = new QHBoxLayout;
    btnOK = new QPushButton("OK");
    connect(btnOK, SIGNAL(clicked()), this, SLOT(sOK()));
    btnCancel = new QPushButton("Cancel");
    connect(btnCancel, SIGNAL(clicked()), this, SLOT(sCancel()));

    hbox->addWidget(btnOK);
    hbox->addWidget(btnCancel);
    hbox->setAlignment(Qt::AlignRight);

    //vbox->addLayout(olay);
    vbox->addWidget(frame);
    vbox->addSpacing(8);
    vbox->addLayout(hbox);

    setWindowTitle("Settings");
}

void NBSettingsDialog::showOpen()
{
    genPath->setText(QFileDialog::getOpenFileName(this, "geninstall.sh"));
}

void NBSettingsDialog::sOK()
{
    QSettings settings("INightmare", "Nixstbuilder");
    settings.setValue("geninstall", genPath->displayText());
    accept();
}

void NBSettingsDialog::sCancel()
{
    reject();
}
