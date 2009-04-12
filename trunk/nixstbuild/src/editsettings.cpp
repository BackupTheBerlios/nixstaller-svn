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

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QSettings>
#include <QVBoxLayout>

#include "editsettings.h"

CEditSettings::CEditSettings(QWidget *parent,
                             Qt::WindowFlags f) : QDialog(parent, f)
{
    setModal(true);

    QVBoxLayout *vbox = new QVBoxLayout(this);

    vbox->addWidget(createDisplaySettings());
    vbox->addWidget(createEditSettings());

    QDialogButtonBox *bbox = new QDialogButtonBox(QDialogButtonBox::Ok |
            QDialogButtonBox::Cancel);
    
    connect(bbox, SIGNAL(accepted()), this, SLOT(slotOK()));
    connect(bbox, SIGNAL(rejected()), this, SLOT(slotCancel()));
    vbox->addWidget(bbox);
}

void CEditSettings::slotOK()
{
    QSettings settings;
    settings.beginGroup("editor");

    settings.beginGroup("panels");
    settings.setValue("line_numbers", lineNrCheck->isChecked());
    settings.setValue("line_changes", lineChangeCheck->isChecked());
    settings.setValue("fold_indicators", foldIndCheck->isChecked());
    settings.setValue("status", statusCheck->isChecked());
    settings.endGroup();
    
    settings.setValue("auto_indent", indentCheck->isChecked());
    settings.setValue("wrap", wrapCheck->isChecked());
    settings.setValue("wrap_movement", wrapMovCheck->isChecked());

    settings.endGroup();
    accept();
}

void CEditSettings::slotCancel()
{
    reject();
}

QWidget *CEditSettings::createDisplaySettings()
{
    QSettings settings;
    settings.beginGroup("editor");
    settings.beginGroup("panels");
    
    QGroupBox *box = new QGroupBox(tr("Display settings"), this);
    QVBoxLayout *vbox = new QVBoxLayout(box);

    lineNrCheck = new QCheckBox(tr("Show line numbers"));
    lineNrCheck->setChecked(settings.value("line_numbers", false).toBool());
    vbox->addWidget(lineNrCheck);

    lineChangeCheck = new QCheckBox(tr("Show line change panel"));
    lineChangeCheck->setChecked(settings.value("line_changes", true).toBool());
    vbox->addWidget(lineChangeCheck);

    foldIndCheck = new QCheckBox(tr("Show fold indicators"));
    foldIndCheck->setChecked(settings.value("fold_indicators", true).toBool());
    vbox->addWidget(foldIndCheck);

    statusCheck = new QCheckBox(tr("Show status panel"));
    statusCheck->setChecked(settings.value("status", true).toBool());
    vbox->addWidget(statusCheck);

    settings.endGroup();
    settings.endGroup();
    
    return box;
}

QWidget *CEditSettings::createEditSettings()
{
    QSettings settings;
    settings.beginGroup("editor");
    
    QGroupBox *box = new QGroupBox(tr("Edit settings"), this);
    QVBoxLayout *vbox = new QVBoxLayout(box);

    indentCheck = new QCheckBox(tr("Auto indent"));
    indentCheck->setChecked(settings.value("auto_indent", true).toBool());
    vbox->addWidget(indentCheck);
    
    wrapCheck = new QCheckBox(tr("Wrap lines"));
    wrapCheck->setChecked(settings.value("wrap", true).toBool());
    vbox->addWidget(wrapCheck);
    
    wrapMovCheck = new QCheckBox(tr("Cursor jumps after wrapped line"));
    wrapMovCheck->setChecked(settings.value("wrap_movement", false).toBool());
    vbox->addWidget(wrapMovCheck);

    settings.endGroup();
    return box;
}
