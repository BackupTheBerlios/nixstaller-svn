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

#ifndef DESKENTRY_H
#define DESKENTRY_H

#include <QDialog>
#include <QMetaType>

#include "treeedit.h"

class QLineEdit;

class CDesktopEntryWidget: public CTreeEdit
{
    Q_OBJECT
    
private slots:
    void addEntry(void);
    
public:

    struct entryitem
    {
        std::string name, exec, categories, icon;
        entryitem(void) {}
        entryitem(const std::string &n, const std::string &e, const std::string &c,
                  const std::string &i) : name(n), exec(e), categories(c), icon(i) { }
    };

    typedef std::vector<entryitem> entryvec;
    
    CDesktopEntryWidget(QWidget *parent = 0, Qt::WindowFlags flags = 0);

    void getEntries(entryvec &entries);
};

class CNewDeskEntryDialog: public QDialog
{
    Q_OBJECT
    
    QLineEdit *fileNameEdit, *execEdit, *mainCatEdit, *addCatEdit, *iconEdit;

private slots:
    void OK(void);
    
public:
    CNewDeskEntryDialog(QWidget *parent = 0, Qt::WindowFlags flags = 0);

    std::string getName(void) const;
    std::string getExec(void) const;
    std::string getCategories(void) const;
    std::string getIcon(void) const;
};

Q_DECLARE_METATYPE(CDesktopEntryWidget::entryitem)

#endif
