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

#ifndef INSTSCREENWIDGET_H
#define INSTSCREENWIDGET_H

#include <treeedit.h>

class QMenu;
class QPushButton;
class QSignalMapper;

class CInstScreenWidget: public CTreeEdit
{
    Q_OBJECT

    QPushButton *addScreenB, *newScreenB;
    bool gotDefaultSet;

    void addScreenBMenuItem(const QString &name, QMenu *menu, QSignalMapper *mapper);
    void deleteItems(const QString &name);

private slots:
    void addScreen(const QString &name);
    void newScreen(void);

public:
    CInstScreenWidget(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    void setDefaults(bool pkg);
};

#endif
