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

#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include "hyperlink.h"

// -------------------------------------
// FLTK Hyperlink Class
// -------------------------------------

CFLTKHyperLink::CFLTKHyperLink(int X, int Y, int W, int H, const char *l) : Fl_Box(X, Y, W, H, l)
{
    labelcolor(FL_BLUE);
}

int CFLTKHyperLink::handle(int event)
{
    switch (event)
    {
        case FL_RELEASE:
            do_callback();
            return 1;
        case FL_ENTER:
            fl_cursor(FL_CURSOR_HAND);
            Fl::check(); // Update cursor
            return 1;
        case FL_LEAVE:
            fl_cursor(FL_CURSOR_DEFAULT);
            Fl::check(); // Update cursor
            return 1;
    }
    
    return Fl_Box::handle(event);
}
