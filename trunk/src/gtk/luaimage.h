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

#ifndef GTK_LUAIMAGE_H
#define GTK_LUAIMAGE_H

#include "main/install/luaimage.h"
#include "luawidget.h"

class CLuaImage: public CBaseLuaImage, public CLuaWidget
{
    GdkPixbuf *m_pPixBuf;
    bool m_bHasImage;
    
    virtual void CoreActivateWidget(void) {}
    
    int MaxImageW(void) const { return 300; }
    int MaxImageH(void) const { return 200; }
    
public:
    CLuaImage(const char *file);
    
    bool HasValidImage(void) const { return m_bHasImage; }
};

#endif
