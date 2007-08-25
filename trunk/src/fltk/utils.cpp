/*
    Copyright (C) 2006, 2007 Rick Helmus (rhelmus_AT_gmail.com)

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

#include "fltk.h"
#include <FL/fl_draw.H>

int TextWidth(const char *t)
{
    return fl_width(t) + 10; // HACK: FLTK's text width function doesn't return full width, so we add a little extra
}

int TitleHeight(int w, const char *desc)
{
    int lines = 1;
    
    w -= 10; // HACK: FLTK seems to offset a little more
    
    const int defheight = fl_height();

    if (desc && *desc)
    {
        const size_t length = strlen(desc);
        double widthfromline = 0.0;
        for (size_t chars=0; chars<length; chars++)
        {
            if ((desc[chars] == '\n') || (widthfromline >= static_cast<double>(w)))
            {
                lines++;
                widthfromline = 0.0;
            }
            else
                widthfromline += fl_width(desc[chars]);
        }
        return lines * defheight;
    }
    
    return 0;
}

