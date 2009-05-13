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

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMenu>
#include <QMenuBar>
#include <QScrollArea>
#include <QStackedWidget>
#include <QStatusBar>
#include <QSplitter>
#include <QTextEdit>
#include <QToolBar>
#include <QTreeView>
#include <QVBoxLayout>

#include "configw.h"
#include "dirbrowser.h"
#include "editor.h"
#include "editsettings.h"
#include "expert.h"
#include "rungen.h"

QConfigWidget *qw;

CExpertScreen::CExpertScreen(QWidget *parent, Qt::WindowFlags flags) : QMainWindow(parent, flags)
{
    setWindowTitle("Nixstbuild v0.1 - Expert mode");
    statusBar()->showMessage(tr("Ready"));
    resize(700, 500);

    editSettings = new CEditSettings(this); // Do this BEFORE createMenuBar()!

    createMenuBar();

    // UNDONE: Layout handling is a bit messing (not auto)
    const int gridw = 150, gridh = 70, listw = gridw+6;
    listWidget = new QListWidget;
//     listWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);
    listWidget->setViewMode(QListView::IconMode);
//     listWidget->setIconSize(QSize(iconw, iconh));
    listWidget->setMovement(QListView::Static);
    listWidget->setFixedWidth(listw);
//     listWidget->setWordWrap(true);
    listWidget->setWrapping(true);
    listWidget->setGridSize(QSize(gridw, gridh));
//     listWidget->setSpacing(6);

    QPalette p = listWidget->palette();
    QColor c = p.color(QPalette::Highlight);
    c.setAlpha(96);
    p.setColor(QPalette::Highlight, c);
    listWidget->setPalette(p);

    // UNDONE: Paths
    addListItem("nixstbuild/gfx/tux_config.png", "General configuration");
    addListItem("nixstbuild/gfx/ark_addfile.png", "Package configuration");
    addListItem("nixstbuild/gfx/ark_addfile.png", "Runtime configuration");
    addListItem("nixstbuild/gfx/ark_addfile.png", "File manager");
    connect(listWidget, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)),
            this, SLOT(changePage(QListWidgetItem *, QListWidgetItem*)));

    widgetStack = new QStackedWidget;
    widgetStack->addWidget(createGeneralConf());
    widgetStack->addWidget(createPackageConf());
    widgetStack->addWidget(createRunConf());
    widgetStack->addWidget(createFileManager());
    
    QWidget *cw = new QWidget;
    setCentralWidget(cw);
    
    QHBoxLayout *layout = new QHBoxLayout(cw);

    layout->addWidget(listWidget);
    layout->addWidget(widgetStack);
}

void CExpertScreen::changePage(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (!current)
        current = previous;

    widgetStack->setCurrentIndex(listWidget->row(current));

    qw->buildConfig();
}

void CExpertScreen::launchRunGen()
{
    CRunGenerator rg;
    if (rg.exec() == QDialog::Accepted)
    {
        editor->setText(rg.getRun());
    }
}

void CExpertScreen::showEditSettings()
{
    if (editSettings->exec() == QDialog::Accepted)
        editor->loadSettings();
}

void CExpertScreen::changeDestBrowser(QListWidgetItem *current,
                                      QListWidgetItem *previous)
{
    if (!current)
        current = previous;

    fileDestBrowser->setRootDir(current->data(Qt::UserRole).toString());
}

void CExpertScreen::createFileMenu()
{
    QMenu *menu = menuBar()->addMenu(tr("&File"));

    QAction *action = new QAction(tr("&New project"), this);
    action->setShortcut(tr("Ctrl+N"));
    action->setStatusTip(tr("Create a new project"));
    menu->addAction(action);

    action = new QAction(tr("&Open project"), this);
    action->setShortcut(tr("Ctrl+O"));
    action->setStatusTip(tr("Open existing project directory"));
    menu->addAction(action);

    menu->addSeparator();
    
    action = new QAction(tr("&Save project"), this);
    action->setShortcut(tr("Ctrl+S"));
    action->setStatusTip(tr("Save project changes"));
    menu->addAction(action);

    action = new QAction(tr("Save project as..."), this);
    action->setStatusTip(tr("Save project to a different directory"));
    menu->addAction(action);

    menu->addSeparator();

    action = new QAction(tr("&Exit"), this);
    action->setShortcut(tr("Ctrl+Q"));
    action->setStatusTip(tr("Exit program"));
    connect(action, SIGNAL(triggered()), this, SLOT(close()));
    menu->addAction(action);
}

void CExpertScreen::createBuildRunMenu()
{
    QMenu *menu = menuBar()->addMenu(tr("&Build && Run"));

    QAction *action = new QAction(tr("&Build"), this);
    action->setShortcut(tr("F7"));
    action->setStatusTip(tr("Build the installer"));
    menu->addAction(action);

    action = new QAction(tr("Build && &Run"), this);
    action->setShortcut(tr("F8"));
    action->setStatusTip(tr("Build and launch the installer"));
    menu->addAction(action);

    action = new QAction(tr("&Preview"), this);
    action->setShortcut(tr("F9"));
    action->setStatusTip(tr("Previews the installer"));
    menu->addAction(action);
}

void CExpertScreen::createSettingsMenu()
{
    QMenu *menu = menuBar()->addMenu(tr("&Settings"));

    QAction *action = new QAction(tr("&Editor settings"), this);
    action->setStatusTip(tr("Text editor settings"));
    connect(action, SIGNAL(triggered()), this, SLOT(showEditSettings()));
    menu->addAction(action);

    action = new QAction(tr("&Nixstbuild settings"), this);
    action->setStatusTip(tr("Settings for Nixstbuild"));
    menu->addAction(action);
}

void CExpertScreen::createHelpMenu()
{
    QMenu *menu = menuBar()->addMenu(tr("&Help"));

    QAction *action = new QAction(tr("&Nixstaller manual"), this);
    action->setShortcut(tr("F1"));
    action->setStatusTip(tr("Displays the Nixstaller manual"));
    menu->addAction(action);

    action = new QAction(tr("&About"), this);
    action->setStatusTip(tr("About Nixstbuild"));
    menu->addAction(action);

    action = new QAction(tr("About Qt"), this);
    connect(action, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    menu->addAction(action);
}

void CExpertScreen::createMenuBar()
{
    createFileMenu();
    createBuildRunMenu();
    createSettingsMenu();
    createHelpMenu();
}

void CExpertScreen::addListItem(const QString &icon, const QString &name)
{
    QListWidgetItem *item = new QListWidgetItem(listWidget);

    if (listWidget->count() == 1)
        listWidget->setCurrentItem(item);
        
    QFont font;
    font.setBold(true);
    item->setFont(font);
    item->setIcon(QIcon(icon));
    item->setText(name);
    item->setTextAlignment(Qt::AlignHCenter);
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
}

QWidget *CExpertScreen::createGeneralConf()
{
    QScrollArea *configScroll = new QScrollArea();
    configScroll->setWidget(qw = new QConfigWidget(this, "configprop.lua", "properties_config", "config.lua"));
    return configScroll;
}

QWidget *CExpertScreen::createPackageConf()
{
    QWidget *ret = new QWidget;
    QFormLayout *layout = new QFormLayout(ret);
    layout->addRow("Package name", new QLineEdit);
    return ret;
}

QWidget *CExpertScreen::createRunConf()
{
    QWidget *ret = new QWidget;
    QVBoxLayout *vlayout = new QVBoxLayout(ret);

    editor = new CEditor(ret);
    editor->getToolBar()->addSeparator();
    QAction *a = editor->getToolBar()->addAction("File wizard");
    connect(a, SIGNAL(triggered()), this, SLOT(launchRunGen()));
    
    editor->getToolBar()->addAction("Generate code");
    editor->load("../newdepscan.lua");
    
    vlayout->addWidget(editor);
    return ret;
}

QWidget *CExpertScreen::createFileManager()
{
    QWidget *ret = new QWidget;
    QVBoxLayout *vbox = new QVBoxLayout(ret);
    
    ret->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QSplitter *mainSplit = new QSplitter(Qt::Vertical);
    vbox->addWidget(mainSplit);

    mainSplit->addWidget(new CDirBrowser("/")); // UNDONE
    
    QSplitter *split = new QSplitter;
    mainSplit->addWidget(split);

    fileDestList = new QListWidget;
    createDestFilesItem("Generic Files", "/tmp/nixstb"); // UNDONE
    fileDestList->setCurrentRow(0);
    split->addWidget(fileDestList);
    connect(fileDestList, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)),
            this, SLOT(changeDestBrowser(QListWidgetItem *, QListWidgetItem*)));

    split->addWidget(fileDestBrowser = new CDirBrowser("/tmp/nixstb")); // UNDONE
    
    split->setSizes(QList<int>() << 100 << 300);
    
    return ret;
}

void CExpertScreen::createDestFilesItem(const QString &title,
                                        const QString &dir)
{
    QListWidgetItem *i = new QListWidgetItem(title, fileDestList);
    i->setData(Qt::UserRole, dir);
}

void CExpertScreen::closeEvent(QCloseEvent *e)
{
    // UNDONE: Check for unsaved stuff
    e->accept();
}
