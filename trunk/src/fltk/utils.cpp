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

#include "fltk.h"
#include <FL/Fl_Button.H>
#include <FL/Fl_Pack.H>
#include <FL/fl_draw.H>

void SetButtonWidth(Fl_Button *button)
{
    const int spacing = 50;
    int w = 0, h = 0;
    fl_font(button->labelfont(), button->labelsize());
    fl_measure(button->label(), w, h);
    button->size(w+spacing, button->h());
}
