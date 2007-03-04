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

#ifndef BASE_INST_SCREEN_H
#define BASE_INST_SCREEN_H

class CBaseScreen
{
    CInstaller *m_pOwner;
    GtkWidget *m_pOwnerBox, *m_pLabel;
    const gchar *m_szOrigLabel;
    
protected:
    CInstaller *GetOwner(void) { return m_pOwner; }
    void SetLabel(const gchar *text);
    GtkWidget *GetOwnerBox(void) { return m_pOwnerBox; }
    
    virtual void UpdateTr(void) { }
    virtual void Create(GtkWidget *parentbox) = 0;
    
public:
    CBaseScreen(CInstaller *owner) : m_pOwner(owner), m_pLabel(NULL) { }
    virtual ~CBaseScreen(void) { }
    
    virtual bool Back(void) { return true; }
    virtual bool Next(void) { return true; }
    virtual bool CanActivate(void) { return true; }
    virtual void Activate(void) { }
    
    void UpdateTranslations(void);
    void CreateScreen(GtkWidget *parentbox) { m_pOwnerBox = parentbox; Create(parentbox); }
};

#endif

