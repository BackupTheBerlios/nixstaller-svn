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

class QMenu;
class QWidget;
class QDialog;
class QTreeWidget;
class QListWidget;
class QStackedWidget;

#include <string>

using namespace std;

#include <QDialog>
#include <QCheckBox>

class Ui_ScreenInputDialog;

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
    QTreeWidget*screenlist;
    QCheckBox *init;
    QCheckBox *install;

    Ui_ScreenInputDialog *currentSUi;
    QDialog *currentSDlg;

    QMenu *listContext;

    string gscript;

    void setupInit();
    void setupInstall();


private slots:
    void sOK();
    void sCancel();
    void sShow(int row);
    void sBUp();
    void sBDown();
    void sBAdd();
    void sBRemove();

    void sListContext(const QPoint &pos);
    void sListAddCheckbox();
    void sListAddDirSelector();
    void sListAddInput();
    void sListAddRadioButton();
    void sListAddConfigMenu();

    void ssidOK();
    void ssidCancel();

    void ssidDefaultC(bool c);
    void ssidCustomC(bool c);
};
