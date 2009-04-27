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

#ifndef NEWINSTSCREEN_H
#define NEWINSTSCREEN_H

#include <map>
#include <string>

#include <QDialog>

#include "main/main.h"
#include "newinstwidget.h"

class QCheckBox;
class QLineEdit;
class QMenu;
class QPushButton;
class QSignalMapper;

class CNewScreenDialog: public QDialog
{
    Q_OBJECT

    CTreeEdit *widgetList;
    QPushButton *addWidgetB;
    QLineEdit *varEdit, *titleEdit;
    QCheckBox *canActBox, *actBox, *updateBox;

    QWidget *createMainGroup(void);
    QWidget *createWidgetGroup(void);
    void addWidgetBMenuItem(const QString &name, QMenu *menu, QSignalMapper *mapper);
    
private slots:
    void OK(void);
    void cancel(void);
    
    void addWidgetItem(const QString &name);
    
public:
    typedef std::vector<widgetitem> widgetvec;
    
    CNewScreenDialog(QWidget *parent = 0, Qt::WindowFlags f = 0);

    std::string variableName(void) const;
    std::string screenTitle(void) const;
    bool genCanActivate(void) const;
    bool genActivate(void) const;
    bool genUpdate(void) const;
    void getWidgets(widgetvec &out) const;
};

#endif
