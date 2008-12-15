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
#include "hyperlink.h"
#include "installer.h"
#include "installscreen.h"
#include "luadepscreen.h"
#include "luaprogressdialog.h"
#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Shared_Image.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Wizard.H>
#include <FL/fl_ask.H>
#include <FL/fl_draw.H>
#include <FL/x.H>

// -------------------------------------
// Main installer screen
// -------------------------------------

CInstaller::CInstaller(void) : m_pAboutDisp(NULL), m_pAboutOKButton(NULL), m_pAboutWindow(NULL),
                               m_pLogoBox(NULL), m_bPrevButtonLocked(false)
{
    fl_register_images();
    Fl::visual(FL_RGB | FL_DOUBLE | FL_INDEX);
#if defined(__APPLE__)
    // needed by OS X to set up the default mouse cursor before anything is rendered
    fl_open_display();
#endif
    Fl::scheme("plastic");
    Fl::background(230, 230, 230);
    
#ifndef __APPLE__
    // HACK: Switch that annoying bell off!
    // UNDONE: Doesn't work on NetBSD yet :(
    XKeyboardControl XKBControl;
    XKBControl.bell_duration = 0;
    XChangeKeyboardControl(fl_display, KBBellDuration, &XKBControl);
#endif
}

CInstaller::~CInstaller(void)
{
    DeleteScreens();
    
#ifndef __APPLE__
    // HACK: Restore bell volume
    XKeyboardControl XKBControl;
    XKBControl.bell_duration = -1;
    XChangeKeyboardControl(fl_display, KBBellDuration, &XKBControl);
#endif
}

void CInstaller::CreateAbout()
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

void CInstaller::ShowAbout(bool show)
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

void CInstaller::CreateHeader()
{
    m_pHeaderGroup = new Fl_Group(0, 0, WindowW(), 50); // Dummy height, real height is set at the end
    m_pHeaderGroup->color(FL_WHITE);
    m_pHeaderGroup->box(FL_FLAT_BOX);
    m_pHeaderGroup->resizable(NULL);
    
    int w = 0, h = 0;
    const char *text = GetTranslation("About");
    m_pAboutBox = new CFLTKHyperLink(0, 0, 0, 0, text);
    m_pAboutBox->labelsize(10);
    m_pAboutBox->callback(ShowAboutCB, this);
    m_pAboutBox->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
    fl_font(m_pAboutBox->labelfont(), m_pAboutBox->labelsize());
    fl_measure(text, w, h);
    m_pAboutBox->resize(WindowW() - w - HeaderSpacing(), 0, w, h);
    
    int x = (m_pLogoBox) ? m_pLogoBox->x() + m_pLogoBox->w() : 0;
    x += HeaderSpacing();
    w = m_pAboutBox->x() - x - HeaderSpacing();
    m_pTitle = new Fl_Box(x, 0, w, 0); // Height is set when title is set
    m_pTitle->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_WRAP);
    m_pTitle->labelsize(18);

    m_pHeaderGroup->end();

    h = (m_pLogoBox) ? (m_pLogoBox->y() + m_pLogoBox->h() + HeaderSpacing()) : 50;
    m_pHeaderGroup->size(m_pHeaderGroup->w(), h);
}

void CInstaller::SetTitle(const std::string &t)
{
    int w = m_pTitle->w(), h = 0;
    int minheaderh = (m_pLogoBox) ? (m_pLogoBox->y() + m_pLogoBox->h() + HeaderSpacing()) : 50;
    fl_font(m_pTitle->labelfont(), m_pTitle->labelsize());
    fl_measure(t.c_str(), w, h);
    minheaderh = std::max(minheaderh, h);

    m_pTitle->size(m_pTitle->w(), minheaderh);
    m_pTitle->label(MakeCString(t));

    if (m_pHeaderGroup->h() != minheaderh)
    {
        // Center logo (if any)
        if (m_pLogoBox)
        {
            const int y = ((minheaderh - m_pLogoBox->h()) / 2);
            m_pLogoBox->position(m_pLogoBox->x(), y);
        }

        m_pHeaderGroup->size(m_pHeaderGroup->w(), minheaderh);
        
        m_pWizard->size(m_pWizard->w(), m_pCancelButton->y()-minheaderh-ButtonHSpacing());
    }
}

CInstallScreen *CInstaller::GetScreen(Fl_Widget *w)
{
    if (!w)
        return NULL;
    
    return static_cast<CInstallScreen *>(w->user_data());
}

void CInstaller::ActivateScreen(CInstallScreen *screen)
{
    SetTitle(GetTranslation(screen->GetTitle()));
    // SetTitle() may have changed wizard's size or the screen may not have it's size initialized, so do this here
    const int x = m_pWizard->x()+ScreenSpacing(), y = m_pWizard->y()+ScreenSpacing();
    const int w = m_pWizard->w()-(2*ScreenSpacing()), h = m_pWizard->h()-(2*ScreenSpacing());
    screen->SetSize(x, y, w, h);
    CBaseAttInstall::ActivateScreen(screen);
    m_pWizard->value(screen->GetGroup());
}

bool CInstaller::FirstValidScreen()
{
    int start = m_pWizard->find(m_pWizard->value());
    
    if (!start || (start == m_pWizard->children()))
        return true;
    
    CInstallScreen *screen;
    start--;
    while((start >= 0) && (screen = GetScreen(m_pWizard->child(start))))
    {
        if (screen->CanActivate())
            return false;
        start--;
    }
    
    return true;
}

bool CInstaller::LastValidScreen()
{
    const int size = m_pWizard->children();
    int start = m_pWizard->find(m_pWizard->value());
    
    if ((start >= (size-1)))
        return true;
    
    CInstallScreen *screen;
    start++;
    while((start<size) && (screen = GetScreen(m_pWizard->child(start))))
    {
        if (screen->CanActivate())
            return false;
        start++;
    }
    
    return true;
}

void CInstaller::UpdateButtonPack()
{
    // HACK: FLTK seems to add half of the spacing at the beginning, so compensate here
    const int spacing = m_pButtonPack->spacing() * 1.5;
    const int w = m_pBackButton->w() + m_pNextButton->w() + spacing;
    const int x = WindowW() - w - ButtonWOffset();
    m_pButtonPack->resize(x, m_pButtonPack->y(), w, m_pButtonPack->h());
}

void CInstaller::UpdateButtons(void)
{
    CInstallScreen *curscreen = GetScreen(m_pWizard->value());
    
    if (FirstValidScreen() && !curscreen->HasPrevWidgets())
        m_pBackButton->deactivate();
    else if (!m_bPrevButtonLocked)
        m_pBackButton->activate();
    
    if (LastValidScreen() && !curscreen->HasNextWidgets())
        m_pNextButton->label(CreateText("%s    @->|", GetTranslation("Finish")));
    else
        m_pNextButton->label(CreateText("%s    @->", GetTranslation("Next")));
}

void CInstaller::Back()
{
    CInstallScreen *screen = GetScreen(m_pWizard->value());
    
    if (!screen)
        return;
    
    if (screen->SubBack())
    {
        UpdateButtons();
        return;
    }
    
    if (!screen->Back())
        return;
    
    for (int start = m_pWizard->find(m_pWizard->value())-1; (start>=0); start--)
    {
        screen = GetScreen(m_pWizard->child(start));
        
        if (screen)
        {
            if (screen->CanActivate())
            {
                m_pWizard->value(m_pWizard->child(start));
                ActivateScreen(screen);
                screen->SubLast();
                UpdateButtons();
                return;
            }
        }
        else
            return;
    }
}

void CInstaller::Next()
{
    CInstallScreen *screen = GetScreen(m_pWizard->value());
    
    if (!screen)
        return;
    
    if (screen->SubNext())
    {
        UpdateButtons();
        return;
    }
    
    if (!screen->Next())
        return;
    
    const int size = m_pWizard->children();
    for (int start = m_pWizard->find(m_pWizard->value())+1; (start<size); start++)
    {
        screen = GetScreen(m_pWizard->child(start));
        
        if (screen)
        {
            if (screen->CanActivate())
            {
                m_pWizard->value(m_pWizard->child(start));
                ActivateScreen(screen);
                UpdateButtons();
                return;
            }
        }
        else
            break;
    }
    
    // No screens left
    m_pMainWindow->hide(); // Close main window, will shutdown the rest
}

char *CInstaller::GetPassword(const char *str)
{
    const char *passwd = fl_password(str);
    
    if (!passwd)
        return NULL;
    
    return StrDup(passwd);
}

void CInstaller::MsgBox(const char *str, ...)
{
    char *text;
    va_list v;
    
    va_start(v, str);
    vasprintf(&text, str, v);
    va_end(v);
    
    fl_message(text);
    
    free(text);
}

bool CInstaller::YesNoBox(const char *str, ...)
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

int CInstaller::ChoiceBox(const char *str, const char *button1, const char *button2, const char *button3, ...)
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

void CInstaller::WarnBox(const char *str, ...)
{
    char *text;
    va_list v;
    
    va_start(v, str);
    vasprintf(&text, str, v);
    va_end(v);
    
    fl_alert(text);
    
    free(text);
}

CBaseScreen *CInstaller::CreateScreen(const std::string &title)
{
    return new CInstallScreen(title);
}

void CInstaller::CoreAddScreen(CBaseScreen *screen)
{
    CInstallScreen *fltkscreen = dynamic_cast<CInstallScreen *>(screen);
    m_pWizard->add(fltkscreen->GetGroup());
    fltkscreen->GetGroup()->user_data(fltkscreen);
}

CBaseLuaProgressDialog *CInstaller::CoreCreateProgDialog(int r)
{
    return new CLuaProgressDialog(r);
}

CBaseLuaDepScreen *CInstaller::CoreCreateDepScreen(int f)
{
    return new CLuaDepScreen(this, f);
}

void CInstaller::CoreUpdate()
{
    CBaseAttInstall::CoreUpdate();
    Fl::wait(0.0f);
}

void CInstaller::LockScreen(bool cancel, bool prev, bool next)
{
    (cancel) ? m_pCancelButton->deactivate() : m_pCancelButton->activate();
    (prev) ? m_pBackButton->deactivate() : m_pBackButton->activate();
    (next) ? m_pNextButton->deactivate() : m_pNextButton->activate();
    m_bPrevButtonLocked = prev;
}

void CInstaller::CoreUpdateLanguage(void)
{
    CBaseAttInstall::CoreUpdateLanguage();
    
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

    CInstallScreen *screen = GetScreen(m_pWizard->value());
    if (screen)
        SetTitle(GetTranslation(screen->GetTitle()));
    
    m_pAboutBox->label(GetTranslation("About"));
    m_pCancelButton->label(GetTranslation("Cancel"));
    m_pBackButton->label(CreateText("@<-    %s", GetTranslation("Back")));
    m_pNextButton->label(CreateText("%s    @->", GetTranslation("Next")));
}

void CInstaller::Init(int argc, char **argv)
{
    const int buttonsy = WindowH()-ButtonHeight()-ButtonHSpacing();
    
    m_pMainWindow = new Fl_Double_Window(WindowW(), WindowH(), "Nixstaller");
    m_pMainWindow->callback(CancelCB, this);

    Fl_Group *maingroup = new Fl_Group(0, 0, WindowW(), WindowH());
    maingroup->resizable(NULL);
    maingroup->box(FL_FLAT_BOX);
    maingroup->color(fl_lighter(FL_BACKGROUND_COLOR));

    Fl_Pack *mainpack = new Fl_Pack(0, 0, WindowW(), WindowH()-90);
    mainpack->resizable(NULL);
    mainpack->type(Fl_Pack::VERTICAL);
    
    CreateHeader();
    
    m_pWizard = new Fl_Wizard(0, m_pHeaderGroup->h(), WindowW(),
                              (buttonsy-m_pHeaderGroup->h()-ButtonHSpacing()));
    m_pWizard->box(FL_ENGRAVED_BOX);
    m_pWizard->end();

    mainpack->end();

    m_pCancelButton = new Fl_Button(ButtonWOffset(), buttonsy, 0, ButtonHeight(), GetTranslation("Cancel"));
    SetButtonWidth(m_pCancelButton);
    m_pCancelButton->callback(CancelCB, this);
    
    m_pButtonPack = new Fl_Pack(0, buttonsy, 0, ButtonHeight());
    m_pButtonPack->type(Fl_Pack::HORIZONTAL);
    m_pButtonPack->spacing(ButtonWSpacing());
    
    m_pBackButton = new Fl_Button(0, buttonsy, 0, ButtonHeight(),
                                  CreateText("@<-    %s", GetTranslation("Back")));
    SetButtonWidth(m_pBackButton);
    m_pBackButton->callback(BackCB, this);
    
    m_pNextButton = new Fl_Button(0, buttonsy, 0, ButtonHeight(),
                                  CreateText("%s    @->", GetTranslation("Next")));
    SetButtonWidth(m_pNextButton);
    m_pNextButton->callback(NextCB, this);
    
    m_pButtonPack->end();
    UpdateButtonPack();
    
    maingroup->end();
    
    CBaseAttInstall::Init(argc, argv);
    
    Fl_Shared_Image *img = Fl_Shared_Image::get(GetLogoFName());
    if (img)
    {
        m_pLogoBox = new Fl_Box(HeaderSpacing(), HeaderSpacing(), img->w()+HeaderSpacing(), img->h());
        m_pLogoBox->align(FL_ALIGN_TOP | FL_ALIGN_INSIDE);
        m_pLogoBox->image(img);
        m_pHeaderGroup->add(m_pLogoBox);
    }

    int size = m_pWizard->children();
    for (int i=0; i<size; i++)
    {
        CInstallScreen *screen = GetScreen(m_pWizard->child(i));
        if (screen->CanActivate())
        {
            ActivateScreen(screen);
            break;
        }
    }
    
    m_pMainWindow->end();
    m_pMainWindow->show(argc, argv);
}

void CInstaller::Run()
{
    CreateAbout(); // Create after everything is initialized: only then GetAboutFName() returns a valid filename
    while (Fl::check())
        Update();
}

void CInstaller::Cancel()
{
	if (AskQuit())
	{
		Fl::delete_widget(GetMainWindow());
		// Call this here, because this function may be called during Lua execution
		::Quit(EXIT_FAILURE);
	}
}

void CInstaller::CancelCB(Fl_Widget *w, void *p)
{
    CInstaller *installer = static_cast<CInstaller *>(p);
    installer->Cancel();
}

void CInstaller::BackCB(Fl_Widget *w, void *p)
{
	CInstaller *installer = static_cast<CInstaller *>(p);

	try
	{
		installer->Back();
	}
	catch (Exceptions::CException &)
	{
		HandleError();
	}
};

void CInstaller::NextCB(Fl_Widget *w, void *p)
{
	CInstaller *installer = static_cast<CInstaller *>(p);
	
	try
	{
		installer->Next();
	}
	catch (Exceptions::CException &)
	{
		HandleError();
	}
};
