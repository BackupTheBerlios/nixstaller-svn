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

class QListWidgetItem;
class QListWidget;
class QStackedWidget;
class QToolButton;

class QFormatScheme;
class QLanguageFactory;

class CEditor;

class CExpertScreen: public QMainWindow
{
    Q_OBJECT

    QStackedWidget *m_pWidgetStack;
    QListWidget *m_pListWidget;
    CEditor *editor;

    void createFileMenu(void);
    void createBuildRunMenu(void);
    void createSettingsMenu(void);
    void createHelpMenu(void);
    void createMenuBar(void);
    
    void addListItem(QString icon, QString name);
    
    QWidget *createGeneralConf(void);
    QWidget *createPackageConf(void);
    QWidget *createRunConf(void);

    QToolButton *createToolButton(QString icon, QString label);

private slots:
    void changePage(QListWidgetItem *current, QListWidgetItem *previous);
    void editSettings(void);
    
public:
    CExpertScreen(QWidget *parent = 0, Qt::WindowFlags flags = 0);
};

#endif
