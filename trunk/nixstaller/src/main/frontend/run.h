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

#ifndef RUN_H
#define RUN_H

#include "main/main.h"

bool HadError(void);
void Quit(int ret);
void HandleError(void);

// These functions should be defined for each frontend
void StartFrontend(int argc, char **argv);
void StopFrontend(void);
void ReportError(const char *msg);

class CLuaRunner: public CMain
{
public:
    virtual void Init(int argc, char **argv);
};

#endif
