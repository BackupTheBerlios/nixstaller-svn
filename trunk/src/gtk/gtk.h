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

#ifndef FR_GTK_H
#define FR_GTK_H

#include "main/main.h"
#include "include/gtk/gtk/gtk.h"

class CGTKBase: virtual public CMain
{
    GtkWidget *m_pMainWindow;
    GtkWidget *m_pAboutDialog;
    
    void CreateAbout(void);
    
protected:
    virtual char *GetPassword(const char *str);
    virtual void MsgBox(const char *str, ...);
    virtual bool YesNoBox(const char *str, ...);
    virtual int ChoiceBox(const char *str, const char *button1, const char *button2, const char *button3, ...);
    virtual void WarnBox(const char *str, ...);

    GtkWidget *GetMainWin(void) { return m_pMainWindow; };
    void ShowAbout(void);
    
public:
    CGTKBase(void);

    void Run(void);
    
    static void DestroyCB(GtkWidget *widget, gpointer data) { gtk_main_quit (); };
};

// Utils
void MessageBox(GtkMessageType type, const char *msg);
GtkWidget *CreateButton(GtkWidget *label, const gchar *image=NULL, bool fromstock = true);
void SetButtonStock(GtkWidget *button, const gchar *image);
void SetWidgetVisible(GtkWidget *w, bool v);
GtkWidget *CreateDirChooser(const char *title);
inline int MaxScreenHeight(void) { return 300; }
inline int WidgetGroupSpacing(void) { return 10; }
inline int MaxWidgetWidth(void) { return 600; }
inline int MaxWidgetHeight(void) { return MaxScreenHeight() - (2*WidgetGroupSpacing()); }
bool ContainerEmpty(GtkContainer *c);

#endif
