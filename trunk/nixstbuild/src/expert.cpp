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
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMenu>
#include <QMenuBar>
#include <QStackedWidget>
#include <QStatusBar>
#include <QScrollArea>
#include <QTextEdit>
#include <QToolButton>
#include <QVBoxLayout>

#include "configw.h"
#include "editor.h"
#include "editsettings.h"
#include "expert.h"

QConfigWidget *qw;

CExpertScreen::CExpertScreen(QWidget *parent, Qt::WindowFlags flags)
{
    setWindowTitle("Nixstbuild v0.1 - Expert mode");
    statusBar()->showMessage(tr("Ready"));
    resize(700, 500);

    createMenuBar();
    
    const int listw = 165, iconw = 96, iconh = 84;
    m_pListWidget = new QListWidget;
    m_pListWidget->setViewMode(QListView::IconMode);
    m_pListWidget->setIconSize(QSize(iconw, iconh));
    m_pListWidget->setMovement(QListView::Static);
    m_pListWidget->setMaximumWidth(listw);
    m_pListWidget->setSpacing(6);

    QPalette p = m_pListWidget->palette();
    QColor c = p.color(QPalette::Highlight);
    c.setAlpha(96);
    p.setColor(QPalette::Highlight, c);
    m_pListWidget->setPalette(p);

    // UNDONE: Paths
    addListItem("nixstbuild/gfx/tux_config.png", "General configuration");
    addListItem("nixstbuild/gfx/ark_addfile.png", "Package configuration");
    addListItem("nixstbuild/gfx/ark_addfile.png", "Runtime configuration");
    connect(m_pListWidget, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)),
            this, SLOT(changePage(QListWidgetItem *, QListWidgetItem*)));

    m_pWidgetStack = new QStackedWidget;
    m_pWidgetStack->addWidget(createGeneralConf());
    m_pWidgetStack->addWidget(createPackageConf());
    m_pWidgetStack->addWidget(createRunConf());
    
    QWidget *cw = new QWidget();
    setCentralWidget(cw);
    
    QHBoxLayout *layout = new QHBoxLayout(cw);

    layout->addWidget(m_pListWidget);
    layout->addWidget(m_pWidgetStack);
}

void CExpertScreen::changePage(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (!current)
        current = previous;

    m_pWidgetStack->setCurrentIndex(m_pListWidget->row(current));

    qw->buildConfig();
}

void CExpertScreen::editSettings()
{
    CEditSettings set;
    if (set.exec() == QDialog::Accepted)
        editor->loadSettings();
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
    connect(action, SIGNAL(triggered()), this, SLOT(editSettings()));
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

void CExpertScreen::addListItem(QString icon, QString name)
{
    QListWidgetItem *item = new QListWidgetItem(m_pListWidget);

    if (m_pListWidget->count() == 1)
        m_pListWidget->setCurrentItem(item);
        
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

    QWidget *buttonw = new QWidget;
    QHBoxLayout *blayout = new QHBoxLayout(buttonw);

    blayout->addWidget(createToolButton("nixstbuild/gfx/tux_config.png", tr("Generate code")));

    vlayout->addWidget(buttonw);

    editor = new CEditor(this);
    editor->load("../newdepscan.lua");
    
    vlayout->addWidget(editor);
    return ret;
}

QToolButton *CExpertScreen::createToolButton(QString icon, QString label)
{
    QToolButton *ret = new QToolButton;
    ret->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    ret->setText(label);
    ret->setIcon(QIcon(icon));
    ret->setIconSize(QSize(32, 32));
    return ret;
}
