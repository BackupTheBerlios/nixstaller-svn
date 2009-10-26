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

#include <map>

#include <QDialog>
#include <QWidget>

#include "main/lua/luatable.h"

class QCheckBox;
class QComboBox;
class QFormLayout;
class QLineEdit;
class QPushButton;

class CTreeEdit;

class CBaseConfValue;

class CNewUnOptdialog: public QDialog
{
    Q_OBJECT
    
    QFormLayout *form;
    QLineEdit *nameField, *descField, *shortField, *varField;
    QComboBox *typeField;

    void enableVarField(bool e);
    
private slots:
    void typeChanged(const QString &t);
    void OK(void);
    
public:
    CNewUnOptdialog(QWidget *parent=0, Qt::WindowFlags flags=0);
    
    void setName(const QString &n);
    QString getName(void) const;

    void setDesc(const QString &d);
    QString getDesc(void) const;

    void setShort(const QString &s);
    QString getShort(void) const;
    
    void setType(const QString &t);
    QString getType(void) const;
    
    void setVar(const QString &v);
    QString getVar(void) const;
};


class CConfigWidget: public QWidget
{
    Q_OBJECT
    
    typedef std::vector<CBaseConfValue *> TValueWidgetList;
    TValueWidgetList valueWidgetList;
    
private slots:
    void valueChangedCB(NLua::CLuaTable &t);
    
public:
    CConfigWidget(const char *proptab, QWidget *parent = 0, Qt::WindowFlags flags = 0);

    void loadConfig(const std::string &dir);
    void saveConfig(const std::string &dir);
    void newConfig(const std::string &dir);

signals:
    void confValueChanged(NLua::CLuaTable &t);
};

class CBaseConfValue: public QWidget
{
    Q_OBJECT
    
    NLua::CLuaTable luaValueTable;

    virtual void coreLoadValue(void) = 0;
    virtual void corePushValue(void) = 0;
    virtual void coreClearWidget(void) = 0;
    virtual void coreSetProjectDir(const std::string &) { }

protected:
    NLua::CLuaTable &getLuaValueTable(void) { return luaValueTable; }

protected slots:
    void valueChangedCB(void);
    
public:
    CBaseConfValue(const NLua::CLuaTable &luat, QWidget *parent = 0,
                   Qt::WindowFlags flags = 0) : QWidget(parent, flags),
                                                luaValueTable(luat) { }

    void loadValue(void) { coreLoadValue(); }
    void pushValue(void) { corePushValue(); }
    void clearWidget(void) { coreClearWidget(); }
    void setProjectDir(const std::string &dir) { coreSetProjectDir(dir); }

signals:
    void confValueChanged(NLua::CLuaTable &t);
};

class CStringConfValue: public CBaseConfValue
{
    QLineEdit *lineEdit;
    
    virtual void coreLoadValue(void);
    virtual void corePushValue(void);
    virtual void coreClearWidget(void);

public:
    CStringConfValue(const NLua::CLuaTable &luat, QWidget *parent = 0,
                     Qt::WindowFlags flags = 0);
};

class CChoiceConfValue: public CBaseConfValue
{
    QComboBox *comboBox;
    
    virtual void coreLoadValue(void);
    virtual void corePushValue(void);
    virtual void coreClearWidget(void);

public:
    CChoiceConfValue(const NLua::CLuaTable &luat, QWidget *parent = 0,
                     Qt::WindowFlags flags = 0);
};

class CMultiChoiceConfValue: public CBaseConfValue
{
    typedef std::vector<QCheckBox *> TChoiceList;
    TChoiceList choiceList;

    virtual void coreLoadValue(void);
    virtual void corePushValue(void);
    virtual void coreClearWidget(void);

public:
    CMultiChoiceConfValue(const NLua::CLuaTable &luat, QWidget *parent = 0,
                          Qt::WindowFlags flags = 0);
};

class CBoolConfValue: public CBaseConfValue
{
    QCheckBox *checkBox;
    
    virtual void coreLoadValue(void);
    virtual void corePushValue(void);
    virtual void coreClearWidget(void);

public:
    CBoolConfValue(const NLua::CLuaTable &luat, QWidget *parent = 0,
                   Qt::WindowFlags flags = 0);
};

class CFileConfValue: public CBaseConfValue
{
    Q_OBJECT

    std::string projectDir;
    QLineEdit *fileView;
    
    virtual void coreLoadValue(void);
    virtual void corePushValue(void);
    virtual void coreClearWidget(void);
    virtual void coreSetProjectDir(const std::string &dir);

private slots:
    void modifyCB(void);
    
public:
    CFileConfValue(const NLua::CLuaTable &luat, QWidget *parent = 0,
                   Qt::WindowFlags flags = 0);
};

class CUnOptsConfValue: public CBaseConfValue
{
    Q_OBJECT
    
    CTreeEdit *treeEdit;
    QPushButton *addButton;
    
    virtual void coreLoadValue(void);
    virtual void corePushValue(void);
    virtual void coreClearWidget(void);

private slots:
    void addCB(void);
    
public:
    CUnOptsConfValue(const NLua::CLuaTable &luat, QWidget *parent = 0,
                     Qt::WindowFlags flags = 0);
};
