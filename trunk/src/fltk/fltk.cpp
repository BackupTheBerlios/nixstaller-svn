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
#include "installer.h"
#include <FL/Fl.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Shared_Image.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Window.H>
#include <FL/x.H>

namespace {
CFLTKBase *pInterface = NULL;
}

void StartFrontend(int argc, char **argv)
{
//     if ((argc > 1) && !strcmp(argv[1], "inst"))
        pInterface = new CInstaller;
/*    else
        pInterface = new CAppManager;*/
    
    // Init
    pInterface->Init(argc, argv);
    pInterface->Run();
}

void StopFrontend()
{
    delete pInterface;
    pInterface = NULL;
}

void ReportError(const char *msg)
{
    fl_message(msg);
}

#ifndef RELEASE
void debugline(const char *t, ...)
{
    char *txt;
    va_list v;
    
    va_start(v, t);
    vasprintf(&txt, t, v);
    va_end(v);
    
    printf("DEBUG: %s", txt);
    
    free(txt);
}
#endif

// -------------------------------------
// FLTK Base screen class
// -------------------------------------

CFLTKBase::CFLTKBase(void) : m_pAboutDisp(NULL), m_pAboutOKButton(NULL), m_pAboutWindow(NULL)
{
    fl_register_images();
    Fl::visual(FL_RGB | FL_DOUBLE | FL_INDEX);
#if defined(__APPLE__)
	// needed by OS X to set up the default mouse cursor before anything is rendered
	fl_open_display();
#endif
    Fl::scheme("plastic");
    Fl::background(230, 230, 230);
    
    // HACK: Switch that annoying bell off!
    // UNDONE: Doesn't work on NetBSD yet :(
	#ifndef __APPLE__
    XKeyboardControl XKBControl;
    XKBControl.bell_duration = 0;
    XChangeKeyboardControl(fl_display, KBBellDuration, &XKBControl);
	#endif
}

CFLTKBase::~CFLTKBase()
{
    // HACK: Restore bell volume
	#ifndef __APPLE__
    XKeyboardControl XKBControl;
    XKBControl.bell_duration = -1;
    XChangeKeyboardControl(fl_display, KBBellDuration, &XKBControl);
	#endif
}

void CFLTKBase::CreateAbout()
{
    m_pAboutWindow = new Fl_Window(580, 400, GetTranslation("About"));
    m_pAboutWindow->set_modal();
    m_pAboutWindow->hide();
    m_pAboutWindow->begin();

    Fl_Text_Buffer *pBuffer = new Fl_Text_Buffer;
    pBuffer->loadfile(GetAboutFName());
    
    m_pAboutDisp = new Fl_Text_Display(20, 20, 540, 350, GetTranslation("About"));
    m_pAboutDisp->buffer(pBuffer);
    
    m_pAboutOKButton = new Fl_Return_Button((580-80)/2, 370, 80, 25, GetTranslation("OK"));
    m_pAboutOKButton->callback(AboutOKCB, this);
    
    m_pAboutWindow->end();
}

char *CFLTKBase::GetPassword(const char *str)
{
    const char *passwd = fl_password(str);
    
    if (!passwd)
        return NULL;
    
    return StrDup(passwd);
}

void CFLTKBase::MsgBox(const char *str, ...)
{
    char *text;
    va_list v;
    
    va_start(v, str);
        vasprintf(&text, str, v);
    va_end(v);
    
    fl_message(text);
    
    free(text);
}

bool CFLTKBase::YesNoBox(const char *str, ...)
{
    char *text;
    va_list v;
    
    va_start(v, str);
        vasprintf(&text, str, v);
    va_end(v);
    
    int ret = fl_choice(text, GetTranslation("No"), GetTranslation("Yes"), NULL);
    free(text);
    
    return (ret!=0);
}

int CFLTKBase::ChoiceBox(const char *str, const char *button1, const char *button2, const char *button3, ...)
{
    char *text;
    va_list v;
    
    va_start(v, button3);
        vasprintf(&text, str, v);
    va_end(v);
    
    int ret = fl_choice(text, button3, button2, button1); // FLTK displays buttons in other order
    free(text);
    
    return (2 - ret);
}

void CFLTKBase::WarnBox(const char *str, ...)
{
    char *text;
    va_list v;
    
    va_start(v, str);
        vasprintf(&text, str, v);
    va_end(v);
    
    fl_alert(text);
    
    free(text);
}

void CFLTKBase::ShowAbout(bool show)
{
    if (show)
    {
        m_pAboutWindow->hotspot(m_pAboutOKButton);
        m_pAboutWindow->take_focus();
        m_pAboutWindow->show();
    }
    else
        m_pAboutWindow->hide();
}

void CFLTKBase::CoreUpdateLanguage()
{
    // Translations for FL ASK dialogs
    fl_yes = GetTranslation("Yes");
    fl_no = GetTranslation("No");
    fl_ok = GetTranslation("OK");
    fl_cancel = GetTranslation("Cancel");
    fl_close = GetTranslation("Close");

    // Translations for FLTK's File Chooser
    Fl_File_Chooser::add_favorites_label = GetTranslation("Add to Favorites");
    Fl_File_Chooser::all_files_label = GetTranslation("All Files (*)");
    Fl_File_Chooser::custom_filter_label = GetTranslation("Custom Filter");
    Fl_File_Chooser::favorites_label = GetTranslation("Favorites");
    Fl_File_Chooser::filename_label = GetTranslation("Filename:");
    Fl_File_Chooser::filesystems_label = GetTranslation("File Systems");
    Fl_File_Chooser::manage_favorites_label = GetTranslation("Manage Favorites");
    Fl_File_Chooser::new_directory_label = GetTranslation("Enter name of new directory");
    Fl_File_Chooser::new_directory_tooltip = GetTranslation("Create new directory");
    Fl_File_Chooser::show_label = GetTranslation("Show:");
    
    // About window
    if (m_pAboutWindow) // May be called before CreateAbout()
    {
        m_pAboutWindow->label(GetTranslation("About"));
        m_pAboutDisp->label(GetTranslation("About"));
        m_pAboutOKButton->label(GetTranslation("OK"));
    }
}

void CFLTKBase::Run()
{
    CreateAbout(); // Create after everything is initialized: only then GetAboutFName() returns a valid filename
    Fl::run();
}
