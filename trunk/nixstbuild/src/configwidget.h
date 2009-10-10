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
#include <QWidget>

class QCheckBox;
class QComboBox;
class QLineEdit;

namespace NLua {
class CLuaTable;
}

class CBaseConfValue;

class CConfigWidget: public QWidget
{
    typedef std::map<std::string, CBaseConfValue *> TValueWidgetMap;
    TValueWidgetMap valueWidgetMap;
    
public:
    CConfigWidget(QWidget *parent = 0, Qt::WindowFlags flags = 0);

    void loadConfig(const std::string &file);
    void saveConfig(const std::string &file);
    void newConfig(const std::string &file);
};

class CBaseConfValue: public QWidget
{
    NLua::CLuaTable luaValueTable;

    virtual void coreLoadValue(void) = 0;
    virtual void corePushValue(void) = 0;
    virtual void coreClearWidget(void) = 0;

protected:
    NLua::CLuaTable &getLuaValueTable(void) { return luaValueTable; }
    
public:
    CBaseConfValue(const NLua::CLuaTable &luat, QWidget *parent = 0,
                   Qt::WindowFlags flags = 0) : QWidget(parent, flags),
                                                luaValueTable(luat) { }

    void loadValue(void) { coreLoadValue(); }
    void pushValue(void) { corePushValue(); }
    void clearWidget(void) { coreClearWidget(); }
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
