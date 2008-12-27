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

#include "main/main.h"
#include "fltk.h"
#include "luagroup.h"
#include "luaimage.h"
#include <FL/Fl_Box.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Shared_Image.H>

// -------------------------------------
// Lua Image Class
// -------------------------------------

CLuaImage::CLuaImage(const char *file) : CBaseLuaWidget(""), m_iImageW(0), m_iImageH(0)
{
    Fl_Shared_Image *img = Fl_Shared_Image::get(file);
    
    if (img)
    {
        if ((img->w() > MaxImageW()) || (img->h() > MaxImageH()))
        {
            int neww, newh;
            GetScaledImageSize(img->w(), img->h(), MaxImageW(), MaxImageH(), neww, newh);
            Fl_Image *temp = img->copy(neww, newh);
            img->release();
            img = (Fl_Shared_Image *)temp;
        }

        Fl_Box *imgbox = new Fl_Box(0, 0, img->w(), img->h());
        imgbox->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        imgbox->image(img);
        GetGroup()->add(imgbox);
        
        m_iImageW = img->w();
        m_iImageH = img->h();
    }
    else
        SetEnable(false);
}

void CLuaImage::CoreSetVisible(bool v)
{
    if (v && !m_iImageW)
        SetEnable(false);
    else
        CLuaWidget::UpdateEnable(v);
}
