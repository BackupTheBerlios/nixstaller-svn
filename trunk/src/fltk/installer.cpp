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
#include "hyperlink.h"
#include "installer.h"
#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Shared_Image.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Wizard.H>
#include <FL/fl_ask.H>
#include <FL/fl_draw.H>

// -------------------------------------
// Main installer screen
// -------------------------------------

void CInstaller::CreateHeader()
{
    int headerh = 50;
    m_pHeaderGroup = new Fl_Group(0, 0, WindowW(), headerh);
    m_pHeaderGroup->color(FL_WHITE);
    m_pHeaderGroup->box(FL_FLAT_BOX);
    
    Fl_Shared_Image *img = Fl_Shared_Image::get("installer.png");
    if (img)
    {
        const int spacing = 5;
        Fl_Box *imgbox = new Fl_Box(spacing, spacing, img->w()+spacing, img->h()+spacing);
        imgbox->align(FL_ALIGN_TOP | FL_ALIGN_INSIDE);
        imgbox->image(img);
        headerh = std::max(headerh, imgbox->y() + imgbox->h());
    }
    
    int w = 0, h = 0, spacing = 3;
    const char *text = GetTranslation("About");
    m_pAboutBox = new CFLTKHyperLink(0, 0, 0, 0, text);
    
    m_pAboutBox->callback(ShowAboutCB, this);
    m_pAboutBox->align(FL_ALIGN_TOP | FL_ALIGN_INSIDE);
    fl_font(m_pAboutBox->labelfont(), m_pHeaderGroup->labelsize());
    fl_measure(text, w, h);
    m_pAboutBox->resize(WindowW() - w - spacing, spacing, w, spacing + h);
    m_pHeaderGroup->end();
    headerh = std::max(headerh, m_pAboutBox->y() + m_pAboutBox->h());
    
    if (m_pHeaderGroup->h() != headerh)
    {
        m_pHeaderGroup->size(m_pHeaderGroup->w(), headerh);
        m_pHeaderGroup->redraw();
    }
}

CInstallScreen *CInstaller::GetScreen(Fl_Widget *w)
{
    return static_cast<CInstallScreen *>(w->user_data());
}

void CInstaller::Back()
{
    
}

void CInstaller::Next()
{
    int hh = m_pHeaderGroup->h();
    int newh = hh + 2;
    
    m_pHeaderGroup->size(m_pHeaderGroup->w(), newh);
    m_pWizard->size(m_pWizard->w(), m_pWizard->h()-2);
    
    m_pHeaderGroup->redraw();
    m_pWizard->redraw();
}

void CInstaller::InstallThink(void)
{
    Fl::wait(0.0f);
}

void CInstaller::UpdateLanguage(void)
{
    CFLTKBase::UpdateLanguage();
    CBaseInstall::UpdateLanguage();
    
    // Update main buttons
    m_pCancelButton->label(GetTranslation("Cancel"));
    m_pBackButton->label(CreateText("@<-    %s", GetTranslation("Back")));
    m_pNextButton->label(CreateText("%s    @->", GetTranslation("Next")));
}

void CInstaller::Init(int argc, char **argv)
{
    const int buttonsy = WindowH()-30, wizardbuttonspacing = 10;
    
    m_pMainWindow = new Fl_Window(WindowW(), WindowH(), "Nixstaller");
    m_pMainWindow->callback(CancelCB, this);

    Fl_Group *maingroup = new Fl_Group(0, 0, WindowW(), WindowH());
    maingroup->box(FL_FLAT_BOX);
    maingroup->color(fl_lighter(FL_BACKGROUND_COLOR));

    Fl_Pack *mainpack = new Fl_Pack(0, 0, WindowW(), WindowH()-90);
    mainpack->type(Fl_Pack::VERTICAL);
    
    CreateHeader();
    
    m_pWizard = new Fl_Wizard(0, m_pHeaderGroup->h(), WindowW(),
                              (buttonsy-m_pHeaderGroup->h()-wizardbuttonspacing));
    m_pWizard->box(FL_ENGRAVED_BOX);
    m_pWizard->end();

    mainpack->end();

    m_pCancelButton = new Fl_Button(20, buttonsy, 120, 25, GetTranslation("Cancel"));
    SetButtonWidth(m_pCancelButton, GetTranslation("Cancel"));
    m_pCancelButton->callback(CancelCB, this);
    
    m_pBackButton = new Fl_Button(WindowW()-280, buttonsy, 120, 25, "@<-    Back");
    m_pBackButton->callback(BackCB, this);
    
    m_pNextButton = new Fl_Button(WindowW()-140, buttonsy, 120, 25, "Next    @->");
    m_pNextButton->callback(NextCB, this);
    
    maingroup->end();
    
    CBaseInstall::Init(argc, argv);

//     int size = m_pWizard->children();
//     for (int i=0; i<size; i++)
//     {
//         CInstallScreen *screen = GetScreen(m_pWizard->child(i));
//         if (screen->CanActivate())
//         {
//             screen->Activate();
//             m_pWizard->value(m_pWizard->child(i));
//             break;
//         }
//     }
    
    m_pMainWindow->end();
    m_pMainWindow->show(argc, argv);
}

void CInstaller::CancelCB(Fl_Widget *w, void *p)
{
    CInstaller *installer = static_cast<CInstaller *>(p);
    
    char *msg;
    if (installer->Installing())
        msg = GetTranslation("Install commands are still running\n"
                "If you abort now this may lead to a broken installation\n"
                "Are you sure?");
    else
        msg = GetTranslation("This will abort the installation\nAre you sure?");
    
    if (fl_choice(msg, GetTranslation("No"), GetTranslation("Yes"), NULL))
        throw Exceptions::CExUser();
}
