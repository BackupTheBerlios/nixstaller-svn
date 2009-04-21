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

#ifndef TREEEDIT_H
#define TREEEDIT_H

#include <QWidget>

class QPushButton;
class QTreeWidget;
class QTreeWidgetItem;
class QVBoxLayout;

class CTreeEdit: public QWidget
{
    Q_OBJECT

    QVBoxLayout *buttonLayout;
    QPushButton *remB, *upB, *downB;
    QTreeWidget *tree;

    void enableEditButtons(bool e);

private slots:
    void del(void);
    void up(void);
    void down(void);

protected:
    QTreeWidgetItem *searchItem(const QString &name, int column=0);
    int searchItemIndex(const QString &name, int column=0);
    int count(void) const;
    QTreeWidget *getTree(void) { return tree; }

public:
    CTreeEdit(QWidget *parent = 0, Qt::WindowFlags flags = 0);

    void setHeader(const QStringList &labels);
    void addItem(QTreeWidgetItem *item);
    void addItem(const QStringList &items);
    
    void insertItem(int index, QTreeWidgetItem *item);
    void insertItem(int index, const QStringList &items);

    QTreeWidgetItem *currentItem(void) const;
    int currentItemIndex(void) const;

    void selectItem(QTreeWidgetItem *item);
    void selectItem(int index);
    
    void insertButton(int index, QPushButton *button);
};

#endif
