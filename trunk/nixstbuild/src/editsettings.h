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

#ifndef EDIT_SETTINGS_H
#define EDIT_SETTINGS_H

#include <QDialog>

#include "qeditor.h"

class QCheckBox;
class QEditConfig;
class QEditor;
class QSettings;

class CEditSettings: public QDialog
{
    Q_OBJECT

    QCheckBox *lineNrCheck, *lineChangeCheck, *foldIndCheck, *statusCheck;
    QCheckBox *indentCheck, *wrapCheck, *wrapMovCheck;
    QEditConfig *editConfig;

    void applySettings(void);
    void applyFlag(int &flags, QEditor::EditFlag flag, bool on);
    QMap<QString, QVariant> readSettingsMap(const QSettings &s);
    QWidget *createDisplaySettings(void);
    QWidget *createEditSettings(void);
    QWidget *createEditConfigs(void);
    
private slots:
    void slotOK(void);
    void slotCancel(void);

protected:
    virtual void showEvent(QShowEvent *event);
    
public:
    CEditSettings(QWidget *parent = 0, Qt::WindowFlags f = 0);

    void loadSettings(void);
    void saveSettings(void);
};

#endif
