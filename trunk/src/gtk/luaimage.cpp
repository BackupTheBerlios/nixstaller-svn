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
#include "gtk.h"
#include "luaimage.h"

// -------------------------------------
// Lua Image Class
// -------------------------------------

CLuaImage::CLuaImage(const char *file) : CBaseLuaWidget(""), m_bHasImage(false)
{
    m_pPixBuf = gdk_pixbuf_new_from_file(file, NULL);
    
    if (m_pPixBuf)
    {
        int w = gdk_pixbuf_get_width(m_pPixBuf);
        int h = gdk_pixbuf_get_height(m_pPixBuf);
        int neww, newh;
        
        if ((w > MaxImageW()) || (h > MaxImageH()))
        {
            GetScaledImageSize(w, h, MaxImageW(), MaxImageH(), neww, newh);
            GdkPixbuf *tmp = gdk_pixbuf_scale_simple(m_pPixBuf, neww, newh, GDK_INTERP_BILINEAR);
            g_object_unref(m_pPixBuf);
            m_pPixBuf = tmp;
        }
        
        if (m_pPixBuf)
        {
            GtkWidget *img = gtk_image_new_from_pixbuf(m_pPixBuf);
            g_object_unref(m_pPixBuf);
            gtk_widget_show(img);
            gtk_container_add(GTK_CONTAINER(GetBox()), img);
            m_bHasImage = true;
        }
    }
}
