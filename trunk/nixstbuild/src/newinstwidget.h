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

#ifndef NEWINSTWIDGET_H
#define NEWINSTWIDGET_H

#include <QDialog>
#include <QMetaType>

#include "main/main.h"
#include "treeedit.h"

class QButtonGroup;
class QCheckBox;
class QComboBox;
class QLineEdit;
class QSpinBox;

class CBaseWidgetField;

namespace NLua {
    class CLuaTable;
}

struct widgetitem
{
    std::string func, variable;
    TStringVec args;
    widgetitem(void) {}
    widgetitem(const std::string &f) : func(f) {}
    widgetitem(const std::string &f, const std::string &v,
               const TStringVec &a) : func(f), variable(v), args(a) {}
};

Q_DECLARE_METATYPE(widgetitem)

class CNewWidgetDialog: public QDialog
{
    Q_OBJECT
    
    std::vector<CBaseWidgetField *> fieldVec;
    QLineEdit *nameField;
    
private slots:
    void OK(void);
    
public:
    CNewWidgetDialog(NLua::CLuaTable &table, QWidget *parent = 0,
                     Qt::WindowFlags f = 0);
    virtual ~CNewWidgetDialog(void);

    std::string getVarName(void) const;
    void getArgs(TStringVec &vec) const;
};

class CBaseWidgetField
{
public:
    virtual ~CBaseWidgetField(void) {}
    
    virtual QWidget *getFieldWidget(void) = 0;
    virtual void addArgs(TStringVec &vec) = 0;
};

class CStringWidgetField: public CBaseWidgetField
{
    QLineEdit *lineEdit;
    bool emptyNil;
    
public:
    CStringWidgetField(NLua::CLuaTable &field);
    virtual QWidget *getFieldWidget(void);
    virtual void addArgs(TStringVec &vec);
};

class CIntWidgetField: public CBaseWidgetField
{
    QSpinBox *spinBox;
    
public:
    CIntWidgetField(NLua::CLuaTable &field);
    virtual QWidget *getFieldWidget(void);
    virtual void addArgs(TStringVec &vec);
};

class CChoiceWidgetField: public CBaseWidgetField
{
    QComboBox *comboBox;
    
public:
    CChoiceWidgetField(NLua::CLuaTable &field);
    virtual QWidget *getFieldWidget(void);
    virtual void addArgs(TStringVec &vec);
};

class CListWidgetField: public QObject, public CBaseWidgetField
{
    Q_OBJECT

    bool multiChoice;
    CTreeEdit *optList;
    QPushButton *addOptB;
    QButtonGroup *boxGroup;

private slots:
    void addOptItem(void);

public:
    CListWidgetField(NLua::CLuaTable &field);

    virtual QWidget *getFieldWidget(void);
    virtual void addArgs(TStringVec &vec);
};

class CCheckWidgetField: public CBaseWidgetField
{
    QCheckBox *checkBox;

public:
    CCheckWidgetField(NLua::CLuaTable &field);
    virtual QWidget *getFieldWidget(void);
    virtual void addArgs(TStringVec &vec);
};

bool newInstWidget(const std::string &name, widgetitem &out);
void getWidgetTypes(TStringVec &out);

#endif
