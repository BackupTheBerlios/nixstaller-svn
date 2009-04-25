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

#include <QMetaType>

#include "main/main.h"
#include "newinstscreen.h"
#include "treeedit.h"

class QMenu;
class QPushButton;
class QSignalMapper;

class CInstScreenWidget: public CTreeEdit
{
    Q_OBJECT

    QPushButton *addScreenB, *newScreenB;
    bool gotDefaultSet;

    void addScreenBMenuItem(const QString &name, const QString &varname,
                            QMenu *menu, QSignalMapper *mapper);
    void deleteItems(const QString &name);

private slots:
    void addScreen(const QString &name);
    void newScreen(void);

public:

    struct screenitem
    {
        std::string variable, title;
        bool canActivate, activate, update;
        CNewScreenDialog::widgetvec widgets;
        screenitem(void) {}
        screenitem(const std::string &v, const std::string &t, bool ca,
                   bool a, bool u) : variable(v), title(t), canActivate(ca),
                                     activate(a), update(u) {}
    };

    typedef std::vector<screenitem> screenvec;
    
    CInstScreenWidget(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    
    void setDefaults(bool pkg);
    void getScreens(TStringVec &screenlist, screenvec &customs);
};

Q_DECLARE_METATYPE(CInstScreenWidget::screenitem)

#endif
