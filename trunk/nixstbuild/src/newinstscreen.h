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

#ifndef NEWINSTSCREEN_H
#define NEWINSTSCREEN_H

#include <map>
#include <string>

#include <QDialog>

class QComboBox;
class QLineEdit;
class QListWidget;
class QMenu;
class QPushButton;
class QSignalMapper;
class QSpinBox;
class QTreeWidget;

namespace NLua {
class CLuaTable;
}

class CNewScreenDialog: public QDialog
{
    Q_OBJECT

    QListWidget *widgetList;
    QPushButton *addWidgetB, *remWidgetB, *upWidgetB, *downWidgetB;

    typedef std::map<std::string, int> widgetmap;
    widgetmap widgetMap;
    
    QWidget *createMainGroup(void);
    QWidget *createWidgetGroup(void);
    void enableEditButtons(bool e);
    void loadWidgetTab(void);
    void addWidgetBMenuItem(const QString &name, QMenu *menu, QSignalMapper *mapper);
    
private slots:
    void OK(void);
    void cancel(void);
    
    void addWidgetItem(const QString &name);
    void remWidgetItem(void);
    void upWidgetItem(void);
    void downWidgetItem(void);
    
public:
    CNewScreenDialog(QWidget *parent = 0, Qt::WindowFlags f = 0);
    virtual ~CNewScreenDialog(void);
};

class CBaseWidgetField
{
public:
    virtual ~CBaseWidgetField(void) {}
    
    virtual QWidget *getFieldWidget(void) = 0;
    virtual std::string getArg(void) = 0;
};

class CStringWidgetField: public CBaseWidgetField
{
    QLineEdit *lineEdit;
    
public:
    CStringWidgetField(NLua::CLuaTable &field);
    virtual QWidget *getFieldWidget(void);
    virtual std::string getArg(void) {}
};

class CIntWidgetField: public CBaseWidgetField
{
    QSpinBox *spinBox;
    
public:
    CIntWidgetField(NLua::CLuaTable &field);
    virtual QWidget *getFieldWidget(void);
    virtual std::string getArg(void) {}
};

class CChoiceWidgetField: public CBaseWidgetField
{
    QComboBox *comboBox;
    
public:
    CChoiceWidgetField(NLua::CLuaTable &field);
    virtual QWidget *getFieldWidget(void);
    virtual std::string getArg(void) {}
};

class CListWidgetField: public QWidget, public CBaseWidgetField
{
    Q_OBJECT

    QTreeWidget *optTree;
    QPushButton *addOptB, *remOptB, *upOptB, *downOptB;

    void enableEditButtons(bool e);

private slots:
    void addOptItem(void);
    void remOptItem(void);
    void upOptItem(void);
    void downOptItem(void);

public:
    CListWidgetField(NLua::CLuaTable &field, QWidget *parent = 0, Qt::WindowFlags f = 0);

    virtual QWidget *getFieldWidget(void) { return this; }
    virtual std::string getArg(void) {}
};

#endif
