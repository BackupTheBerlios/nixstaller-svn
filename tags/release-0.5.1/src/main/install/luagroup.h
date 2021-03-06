/*
    Copyright (C) 2006 - 2009 Rick Helmus (rhelmus_AT_gmail.com)

    This file is part of Nixstaller.
    
    This program is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License as published by the Free Software
    Foundation; either version 2 of the License, or (at your option) any later
    version. 
    
    This program is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
    PARTICULAR PURPOSE. See the GNU General Public License for more details. 
    
    You should have received a copy of the GNU General Public License along with
    this program; if not, write to the Free Software Foundation, Inc., 51 Franklin
    St, Fifth Floor, Boston, MA 02110-1301 USA
*/

#ifndef LUAGROUP_H
#define LUAGROUP_H

#include <vector>
#include "main/lua/luaclass.h"

class CBaseLuaWidget;
class CBaseLuaInputField;
class CBaseLuaCheckbox;
class CBaseLuaRadioButton;
class CBaseLuaDirSelector;
class CBaseLuaCFGMenu;
class CBaseLuaMenu;
class CBaseLuaImage;
class CBaseLuaProgressBar;
class CBaseLuaTextField;
class CBaseLuaLabel;

class CBaseLuaGroup
{
protected:
    typedef std::vector<CBaseLuaWidget *> TWidgetList;
    
private:
    TWidgetList m_WidgetList;
    bool m_bEndsScreen;
    
    static std::map<std::string, bool> m_RegisteredWidgets;

    virtual CBaseLuaInputField *CreateInputField(const char *label, const char *desc, const char *val,
            int max, const char *type) = 0;
    virtual CBaseLuaCheckbox *CreateCheckbox(const char *desc, const std::vector<std::string> &l,
                                             const std::vector<TSTLVecSize> &e) = 0;
    virtual CBaseLuaRadioButton *CreateRadioButton(const char *desc, const std::vector<std::string> &l,
                                                   TSTLVecSize e) = 0;
    virtual CBaseLuaDirSelector *CreateDirSelector(const char *desc, const char *val) = 0;
    virtual CBaseLuaCFGMenu *CreateCFGMenu(const char *desc) = 0;
    virtual CBaseLuaMenu *CreateMenu(const char *desc, const std::vector<std::string> &l,
                                     TSTLVecSize e) = 0;
    virtual CBaseLuaImage *CreateImage(const char *file) = 0;
    virtual CBaseLuaProgressBar *CreateProgressBar(const char *desc) = 0;
    virtual CBaseLuaTextField *CreateTextField(const char *desc, bool wrap, const char *size) = 0;
    virtual CBaseLuaLabel *CreateLabel(const char *title) = 0;
    
    void AddWidget(CBaseLuaWidget *w, const char *type);
    
protected:
    // This function is not called by the destructor, because in some frontends a lua widget is part of the gui and will
    // be automaticly deleted
    void DeleteWidgets(void);
    
    const TWidgetList &GetWidgetList(void) { return m_WidgetList; }
    
public:
    CBaseLuaGroup(void) : m_bEndsScreen(false) {}
    virtual ~CBaseLuaGroup(void) {}
    
    void UpdateLanguage(void);
    bool IsEnabled(void) const;
    bool CheckWidgets(void);
    void SetScreenEnds(bool e) { m_bEndsScreen = e; }
    bool EndsScreen(void) const { return m_bEndsScreen; }
    
    static void LuaRegister(void);
    
    static int LuaAddInput(lua_State *L);
    static int LuaAddCheckbox(lua_State *L);
    static int LuaAddRadioButton(lua_State *L);
    static int LuaAddDirSelector(lua_State *L);
    static int LuaAddCFGMenu(lua_State *L);
    static int LuaAddMenu(lua_State *L);
    static int LuaAddImage(lua_State *L);
    static int LuaAddProgressBar(lua_State *L);
    static int LuaAddTextField(lua_State *L);
    static int LuaAddLabel(lua_State *L);
};

#endif
