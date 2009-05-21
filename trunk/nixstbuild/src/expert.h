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

#ifndef EXPERT_H
#define EXPERT_H

#include <QMainWindow>

#include "main/main.h"

class QListWidgetItem;
class QListWidget;
class QStackedWidget;

class QFormatScheme;
class QLanguageFactory;

class CBaseExpertTab;
class CDirBrowser;
class CEditor;
class CEditSettings;

class QConfigWidget; // UNDONE

class CExpertScreen: public QMainWindow
{
    Q_OBJECT

    QString projectDir;
    QStackedWidget *widgetStack;
    QListWidget *listWidget;
    CEditSettings *editSettings;
    std::vector<CBaseExpertTab *> tabs;

    void createFileMenu(void);
    void createBuildRunMenu(void);
    void createSettingsMenu(void);
    void createHelpMenu(void);
    void createMenuBar(void);
    
    void addListItem(const QString &icon, const QString &name);
    void addTab(CBaseExpertTab *tab);

private slots:
    void newProject(void);
    void openProject(void);
    void saveProject(void);
    void changePage(QListWidgetItem *current, QListWidgetItem *previous);
    void showEditSettings(void);

protected:
    virtual void closeEvent(QCloseEvent *e);
    
public:
    CExpertScreen(QWidget *parent = 0, Qt::WindowFlags flags = 0);
};

class CBaseExpertTab: public QWidget
{
protected:
    CBaseExpertTab(QWidget *parent = 0, Qt::WindowFlags flags = 0) : QWidget(parent, flags) {}

public:
    virtual void loadProject(const QString &dir) = 0;
    virtual void saveProject(const QString &dir) = 0;
    virtual void newProject(const QString &dir) = 0;
};

class CGeneralConfTab: public CBaseExpertTab
{
    QConfigWidget *qw;
    
public:
    CGeneralConfTab(QWidget *parent = 0, Qt::WindowFlags flags = 0);

    virtual void loadProject(const QString &dir) {}
    virtual void saveProject(const QString &dir) {}
    virtual void newProject(const QString &dir) {}
};

class CPackageConfTab: public CBaseExpertTab
{
public:
    CPackageConfTab(QWidget *parent = 0, Qt::WindowFlags flags = 0);

    virtual void loadProject(const QString &dir) {}
    virtual void saveProject(const QString &dir) {}
    virtual void newProject(const QString &dir) {}
};

class CRunConfTab: public CBaseExpertTab
{
    Q_OBJECT
    
    CEditor *editor;

    void insertRunText(const QString &text);

private slots:
    void launchRunGen(void);
    void genRunWidgetCB(QAction *action);
    void genRunScreenCB(void);
    void genRunDeskEntryCB(void);
    void genRunFunctionCB(void);

public:
    CRunConfTab(QWidget *parent = 0, Qt::WindowFlags flags = 0);

    virtual void loadProject(const QString &dir);
    virtual void saveProject(const QString &dir);
    virtual void newProject(const QString &dir);
};

class CFileManagerTab: public CBaseExpertTab
{
    Q_OBJECT
    
    QListWidget *fileDestList;
    CDirBrowser *fileDestBrowser;

    void createDestFilesItem(const QString &title, const QString &dir);

private slots:
    void changeDestBrowser(QListWidgetItem *current, QListWidgetItem *previous);

public:
    CFileManagerTab(QWidget *parent = 0, Qt::WindowFlags flags = 0);

    virtual void loadProject(const QString &dir);
    virtual void saveProject(const QString &dir) {}
    virtual void newProject(const QString &dir);
};

#endif
