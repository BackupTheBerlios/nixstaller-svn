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
#include <QShowEvent>
#include <QVBoxLayout>

#include "qeditconfig.h"

#include "editsettings.h"

CEditSettings::CEditSettings(QWidget *parent,
                             Qt::WindowFlags f) : QDialog(parent, f)
{
    setModal(true);

    QVBoxLayout *vbox = new QVBoxLayout(this);

    vbox->addWidget(createDisplaySettings());
    vbox->addWidget(createEditSettings());
    vbox->addWidget(createEditConfigs());

    QDialogButtonBox *bbox = new QDialogButtonBox(QDialogButtonBox::Ok |
            QDialogButtonBox::Cancel);
    
    connect(bbox, SIGNAL(accepted()), this, SLOT(slotOK()));
    connect(bbox, SIGNAL(rejected()), this, SLOT(slotCancel()));
    vbox->addWidget(bbox);

    loadSettings();
    applySettings();
}

void CEditSettings::slotOK()
{
    saveSettings();
    applySettings();
    accept();
}

void CEditSettings::slotCancel()
{
    editConfig->cancel();
    reject();
}

void CEditSettings::applySettings()
{
    QSettings settings;
    settings.beginGroup("editor");

    int flags = QEditor::defaultFlags();

    applyFlag(flags, QEditor::AutoIndent, settings.value("auto_indent", true).toBool());
    applyFlag(flags, QEditor::LineWrap, settings.value("wrap", true).toBool());
    applyFlag(flags, QEditor::CursorJumpPastWrap, settings.value("wrap_movement", false).toBool());
    
    QEditor::setDefaultFlags(flags);

    editConfig->apply();
    
    settings.endGroup();
}

void CEditSettings::applyFlag(int &flags, QEditor::EditFlag flag, bool on)
{
    if (on)
        flags |= flag;
    else
        flags &= ~flag;
}

// Copied from QCodeEditor example
QMap<QString, QVariant> CEditSettings::readSettingsMap(const QSettings &s)
{
    QMap<QString, QVariant> m;
    QStringList c = s.childKeys();
    
    foreach ( QString k, c )
        m[k] = s.value(k);
    
    return m;
}

QWidget *CEditSettings::createDisplaySettings()
{
    QGroupBox *box = new QGroupBox(tr("Display settings"), this);
    QVBoxLayout *vbox = new QVBoxLayout(box);

    lineNrCheck = new QCheckBox(tr("Show line numbers"));
    vbox->addWidget(lineNrCheck);

    lineChangeCheck = new QCheckBox(tr("Show line change panel"));
    vbox->addWidget(lineChangeCheck);

    foldIndCheck = new QCheckBox(tr("Show fold indicators"));
    vbox->addWidget(foldIndCheck);

    statusCheck = new QCheckBox(tr("Show status panel"));
    vbox->addWidget(statusCheck);

    return box;
}

QWidget *CEditSettings::createEditSettings()
{
    QGroupBox *box = new QGroupBox(tr("Edit settings"), this);
    QVBoxLayout *vbox = new QVBoxLayout(box);

    indentCheck = new QCheckBox(tr("Auto indent"));
    vbox->addWidget(indentCheck);
    
    wrapCheck = new QCheckBox(tr("Wrap lines"));
    vbox->addWidget(wrapCheck);
    
    wrapMovCheck = new QCheckBox(tr("Cursor jumps after wrapped line"));
    vbox->addWidget(wrapMovCheck);

    return box;
}

QWidget *CEditSettings::createEditConfigs()
{
    editConfig = new QEditConfig(this);
    return editConfig;
}

void CEditSettings::showEvent(QShowEvent *event)
{
    if (!event->spontaneous())
        loadSettings();
}

void CEditSettings::loadSettings()
{
    QSettings settings;
    settings.beginGroup("editor");
    settings.beginGroup("panels");

    lineNrCheck->setChecked(settings.value("line_numbers", false).toBool());
    lineChangeCheck->setChecked(settings.value("line_changes", true).toBool());
    foldIndCheck->setChecked(settings.value("fold_indicators", true).toBool());
    statusCheck->setChecked(settings.value("status", true).toBool());

    settings.endGroup();

    indentCheck->setChecked(settings.value("auto_indent", true).toBool());
    wrapCheck->setChecked(settings.value("wrap", true).toBool());
    wrapMovCheck->setChecked(settings.value("wrap_movement", false).toBool());

    editConfig->loadKeys(readSettingsMap(settings));

    settings.endGroup();
}

void CEditSettings::saveSettings()
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

    editConfig->apply();
    const QMap<QString, QVariant> &map = editConfig->dumpKeys();
    QMap<QString, QVariant>::const_iterator it = map.constBegin();

    for (; it!=map.constEnd(); it++)
        settings.setValue(it.key(), *it);
    
    settings.endGroup();
}
