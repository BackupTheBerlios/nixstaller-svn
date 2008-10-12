/*
    Copyright (C) 2006 - 2008 Rick Helmus (rhelmus_AT_gmail.com)

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

#ifndef GTK_LUAWIDGET_H
#define GTK_LUAWIDGET_H

#include "main/install/luawidget.h"

class CInstallScreen;

class CLuaWidget: virtual public CBaseLuaWidget
{
    GtkWidget *m_pBox, *m_pTitle;
    int m_iMaxWidth;
    CInstallScreen *m_pInstallScreen;
    
    virtual void CoreSetTitle(void);
    virtual void CoreUpdateMaxWidth(int w) {}
    
protected:
    void SafeLuaDataChanged(void);
    void UpdateScreenLayout(void);
    
public:
    CLuaWidget(void);
    
    GtkWidget *GetBox(void) { return m_pBox; }
    void SetMaxWidth(int w);
    void SetScreen(CInstallScreen *screen) { m_pInstallScreen = screen; }
};

#endif
