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

class QWidget;
class QDialog;
class QListWidget;
class QStackedWidget;

#include <string>

using namespace std;

#include <QDialog>
#include <QCheckBox>

class NBRunGen: public QDialog
{
    Q_OBJECT
public:
    NBRunGen(QWidget *parent = 0);

    string script();

private:
    QWidget *init_widget;
    QWidget *install_widget;
    QStackedWidget *qstack;
    QListWidget *winlist;
    QCheckBox *init;
    QCheckBox *install;
    QListWidget *screenList;

    string gscript;

    void setupInit();
    void setupInstall();


private slots:
    void sOK();
    void sCancel();
    void sShow(int row);
    void sBUp();
    void sBDown();
};
