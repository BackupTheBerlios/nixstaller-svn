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

#include <string>

using namespace std;

#include <Qt>
#include <QtGui>

#include <QFrame>
#include <QDialog>
#include <QCheckBox>
#include <QMultiMap>
#include <QTreeWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>

#include "rungen.h"
#include "ui_screeninput.h"

const char *base_install = "function Install()\n\tinstall.extractfiles()\nend\n\n";

inline QTreeWidgetItem *newItem(QTreeWidget *parent, char *caption)
{
    QTreeWidgetItem *nitem = new QTreeWidgetItem();
    nitem->setText(0, caption);
    return nitem;
}

inline QTreeWidgetItem *newItemQS(QTreeWidget *parent, QString caption)
{
    QTreeWidgetItem *nitem = new QTreeWidgetItem();
    nitem->setText(0, caption);
    return nitem;
}

inline bool isDefault(QString screen)
{
    if ((screen=="WelcomeScreen") || (screen=="LicenseScreen") || (screen=="SelectDirScreen") || (screen=="InstallScreen") || (screen=="FinishScreen"))
        return true;
    else return false;
}

NBRunGen::NBRunGen(QWidget *parent): QDialog(parent)
{
    QVBoxLayout *mainlay = new QVBoxLayout(this);
    QHBoxLayout *toplay = new QHBoxLayout;
    winlist = new QListWidget;
    winlist->addItem("Init");
    winlist->addItem("Install");
    winlist->setMaximumSize(128, 1280);

    qstack = new QStackedWidget;
    init_widget = new QWidget;
    install_widget = new QWidget;

    toplay->addWidget(winlist);
    toplay->addWidget(qstack);
    toplay->setAlignment(Qt::AlignLeft);

    QHBoxLayout *btns = new QHBoxLayout();
    btns->setAlignment(Qt::AlignRight);
    QPushButton *bok = new QPushButton("OK");
    QPushButton *bcancel = new QPushButton("Cancel");
    btns->addWidget(bok);
    btns->addWidget(bcancel);

    connect(bok, SIGNAL(clicked()), this, SLOT(sOK()));
    connect(bcancel, SIGNAL(clicked()), this, SLOT(sCancel()));
    connect(winlist, SIGNAL(currentRowChanged(int)), this, SLOT(sShow(int)));

    mainlay->addLayout(toplay);
    mainlay->addLayout(btns);

    setupInit();
    setupInstall();
}

void NBRunGen::setupInit()
{
    QVBoxLayout *lay = new QVBoxLayout(init_widget);
    QFrame *topframe = new QFrame;
    topframe->setFrameStyle(QFrame::Panel | QFrame::Plain);
    topframe->setLineWidth(1);

    lay->addWidget(topframe);
    QVBoxLayout *clay = new QVBoxLayout(topframe);
    init = new QCheckBox("Generate Init()", topframe);
    init->setToolTip("Generate simple Init() function that initializes selected screens.");
    clay->addWidget(init);
    clay->addSpacing(4);
    QFrame *restframe = new QFrame;
    restframe->setFrameStyle(QFrame::Panel | QFrame::Plain);
    restframe->setLineWidth(1);

    // Configure init
    QVBoxLayout *screenlay = new QVBoxLayout;
    QHBoxLayout *listlay = new QHBoxLayout(restframe);

    QVBoxLayout *btn_lay = new QVBoxLayout;
    QPushButton *bup = new QPushButton(QIcon(":/cr32-action-up.png"), "");
    QPushButton *bdown = new QPushButton(QIcon(":/cr32-action-down.png"), "");

    screenlist = new QTreeWidget;
    screenlist->addTopLevelItem(newItem(screenlist, "WelcomeScreen"));
    screenlist->addTopLevelItem(newItem(screenlist, "LicenseScreen"));
    screenlist->addTopLevelItem(newItem(screenlist, "SelectDirScreen"));
    screenlist->addTopLevelItem(newItem(screenlist, "InstallScreen"));
    screenlist->addTopLevelItem(newItem(screenlist, "FinishScreen"));

    QStringList headers;
    headers << "Screens" << "Screen text";
    screenlist->setHeaderLabels(headers);

    QPushButton *badd = new QPushButton(QIcon(":/cr32-action-add.png"), "Add");
    QPushButton *bremove = new QPushButton(QIcon(":/cr32-action-remove.png"), "Remove");
    QHBoxLayout *barlay = new QHBoxLayout;
    barlay->addWidget(badd); barlay->addWidget(bremove);

    connect(badd, SIGNAL(clicked()), this, SLOT(sBAdd()));
    connect(bremove, SIGNAL(clicked()), this, SLOT(sBRemove()));

    screenlay->addWidget(screenlist);
    screenlay->addLayout(barlay);

    btn_lay->addWidget(bup); 
    btn_lay->addWidget(bdown);
    btn_lay->setAlignment(Qt::AlignVCenter);
    listlay->addLayout(screenlay);
    listlay->addLayout(btn_lay);
    //conflayout->addLayout(listlay);

    connect(bup, SIGNAL(clicked()), this, SLOT(sBUp()));
    connect(bdown, SIGNAL(clicked()), this, SLOT(sBDown()));

    lay->addWidget(restframe);
    lay->setAlignment(Qt::AlignTop);

    qstack->addWidget(init_widget);

    listContext = new QMenu("Add Widget");
    screenlist->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(listContext->addAction("Add Checkbox"), SIGNAL(triggered()), this, SLOT(sListAddCheckbox()));
    connect(listContext->addAction("Add Directory Selector"), SIGNAL(triggered()), this, SLOT(sListAddDirSelector()));
    connect(listContext->addAction("Add Input Field"), SIGNAL(triggered()), this, SLOT(sListAddInput()));
    connect(listContext->addAction("Add Radiobutton"), SIGNAL(triggered()), this, SLOT(sListAddRadioButton()));
    connect(listContext->addAction("Add Config menu"), SIGNAL(triggered()), this, SLOT(sListAddConfigMenu()));

    connect(screenlist, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(sListContext(QPoint)));
}

void NBRunGen::setupInstall()
{
    QVBoxLayout *lay = new QVBoxLayout(install_widget);
    QFrame *topframe = new QFrame;
    topframe->setFrameStyle(QFrame::Panel | QFrame::Plain);
    topframe->setLineWidth(1);

    lay->addWidget(topframe);
    QVBoxLayout *clay = new QVBoxLayout(topframe);
    install = new QCheckBox("Generate Install()", topframe);
    install->setToolTip("Generate simple Install() function that extracts files");
    clay->addWidget(install);
    clay->addSpacing(4);
    QFrame *restframe = new QFrame;
    restframe->setFrameStyle(QFrame::Panel | QFrame::Plain);
    restframe->setLineWidth(1);

    lay->addWidget(restframe);
    lay->setAlignment(Qt::AlignTop);

    qstack->addWidget(install_widget);
}

void NBRunGen::sOK()
{
    if (init->isChecked())
    {
        if (screenlist->topLevelItemCount()==0)
        {
            QMessageBox::warning(this, "Nixstbuild", tr("Please specify at least one screen"));
            gscript = " ";
            return;
        }

        gscript = "function Init()\n\t";

        for (int i = 0; i < screenlist->topLevelItemCount()-1; i++)
        {
            if (!isDefault(screenlist->topLevelItem(i)->text(0)))
            {
                gscript += screenlist->topLevelItem(i)->text(0).toStdString() + " = install.newcfgscreen(\""+ screenlist->topLevelItem(i)->text(1).toStdString()  +"\")\n\t";
            }
        }
        gscript += "\n";
        gscript += "\tinstall.screenlist = { ";

        for (int i = 0; i < screenlist->topLevelItemCount()-1; i++)
        {
            gscript += screenlist->topLevelItem(i)->text(0).toStdString() + ", ";
        }

        gscript += screenlist->topLevelItem(screenlist->topLevelItemCount()-1)->text(0).toStdString() + " }\n";
        gscript += "end\n\n";
    }

    if (install->isChecked())
    {
        gscript += base_install;
    }

    accept();
}

void NBRunGen::sCancel()
{
    reject();
}

void NBRunGen::sShow(int row)
{
    qstack->setCurrentIndex(row);
}

string NBRunGen::script()
{

    if (gscript.empty())
    {
        return string(" ");
    }

    return gscript;
}

void NBRunGen::sBUp()
{
    QTreeWidgetItem *citem = screenlist->currentItem();
    QTreeWidgetItem *nitem = citem->clone();
    int nindex = screenlist->indexOfTopLevelItem(citem) - 1;

    if (nindex<0) nindex = 0;

    delete citem;

    screenlist->insertTopLevelItem(nindex, nitem);
    screenlist->setCurrentItem(nitem);
}

void NBRunGen::sBDown()
{
    QTreeWidgetItem *citem = screenlist->currentItem();
    QTreeWidgetItem *nitem = citem->clone();
    int nindex = screenlist->indexOfTopLevelItem(citem) + 1;

    if (nindex > screenlist->topLevelItemCount()-1) nindex--;

    delete citem;

    screenlist->insertTopLevelItem(nindex, nitem);
    screenlist->setCurrentItem(nitem);
}

void NBRunGen::sBAdd()
{
    currentSUi = new Ui_ScreenInputDialog();
    currentSDlg = new QDialog();
    currentSUi->setupUi(currentSDlg);

    connect(currentSUi->rDefault, SIGNAL(clicked(bool)), this, SLOT(ssidDefaultC(bool)));
    connect(currentSUi->rCustom, SIGNAL(clicked(bool)), this, SLOT(ssidCustomC(bool)));

    connect(currentSUi->btnOK, SIGNAL(clicked()), this, SLOT(ssidOK()));
    connect(currentSUi->btnCancel, SIGNAL(clicked()), this, SLOT(ssidCancel()));

    currentSUi->dScreenBox->addItem("WelcomeScreen");
    currentSUi->dScreenBox->addItem("LicenseScreen");
    currentSUi->dScreenBox->addItem("SelectDirScreen");
    currentSUi->dScreenBox->addItem("InstallScreen");
    currentSUi->dScreenBox->addItem("FinishScreen");

    currentSDlg->exec();

    delete currentSDlg;
    delete currentSUi;
}

void NBRunGen::sBRemove()
{
    delete screenlist->currentItem();
}

void NBRunGen::sListContext(const QPoint &pos)
{
/*
    if (!isDefault(screenlist->currentItem()->text(0)))
        listContext->popup(screenlist->mapToGlobal(pos));
*/
}

void NBRunGen::sListAddCheckbox()
{
    QTreeWidgetItem *item = screenlist->currentItem();
    QStringList s;
    s << "CheckBox" << "title";
    QTreeWidgetItem *nitem = new QTreeWidgetItem(item, s);
    nitem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
    item->addChild(nitem);
}

void NBRunGen::sListAddDirSelector()
{
    QTreeWidgetItem *item = screenlist->currentItem();
    QStringList s;
    s << "DirSelector" << "title";
    QTreeWidgetItem *nitem = new QTreeWidgetItem(item, s);
    nitem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
    item->addChild(nitem);
}

void NBRunGen::sListAddInput()
{
    QTreeWidgetItem *item = screenlist->currentItem();
    QStringList s;
    s << "InputField" << "title";
    QTreeWidgetItem *nitem = new QTreeWidgetItem(item, s);
    nitem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
    item->addChild(nitem);
}

void NBRunGen::sListAddRadioButton()
{
    QTreeWidgetItem *item = screenlist->currentItem();
    QStringList s;
    s << "RadioButton" << "title";
    QTreeWidgetItem *nitem = new QTreeWidgetItem(item, s);
    nitem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
    item->addChild(nitem);
}

void NBRunGen::sListAddConfigMenu()
{
    QTreeWidgetItem *item = screenlist->currentItem();
    QStringList s;
    s << "ConfigMenu" << "under construction";
    QTreeWidgetItem *nitem = new QTreeWidgetItem(item, s);
    nitem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
    item->addChild(nitem);
}

void NBRunGen::ssidOK()
{
    if (currentSUi->rDefault->isChecked())
    {
        screenlist->addTopLevelItem(newItemQS(screenlist, currentSUi->dScreenBox->currentText()));
    } else {
        QStringList text;
        text << currentSUi->cScreenBox->text() << currentSUi->screenText->text();
        screenlist->addTopLevelItem(new QTreeWidgetItem(screenlist, text));
    }

    currentSDlg->accept();
}

void NBRunGen::ssidCancel()
{
    currentSDlg->reject();
}

void NBRunGen::ssidDefaultC(bool c)
{
    currentSUi->dScreenBox->setEnabled(true);
    currentSUi->cScreenBox->setEnabled(false);
    currentSUi->screenText->setEnabled(false);
}

void NBRunGen::ssidCustomC(bool c)
{
    currentSUi->dScreenBox->setEnabled(false);
    currentSUi->cScreenBox->setEnabled(true);
    currentSUi->screenText->setEnabled(true);
}

