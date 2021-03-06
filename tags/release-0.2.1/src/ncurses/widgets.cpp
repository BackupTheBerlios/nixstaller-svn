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

#include "ncurses.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <math.h>

// -------------------------------------
// Widget handler class
// -------------------------------------

CWidgetHandler::~CWidgetHandler()
{
    for (std::list<CWidgetWindow *>::iterator it=m_ChildList.begin(); it!= m_ChildList.end(); it++)
        delete *it;
}

bool CWidgetHandler::HandleKey(chtype ch)
{
    if (m_FocusedChild != m_ChildList.end())
        return ((*m_FocusedChild)->HandleKey(ch) || (m_pBoundKeyWidget && m_pBoundKeyWidget->HandleKey(ch)));
    
    return false;
}

bool CWidgetHandler::SetNextWidget(bool rec)
{
    if (m_ChildList.empty())
    {
        m_FocusedChild = m_ChildList.end();
        return !Focused();
    }
        
    std::list<CWidgetWindow *>::iterator it = ((Focused()) ? m_FocusedChild : m_ChildList.begin());
    
    while (it != m_ChildList.end())
    {
        if ((*it)->CanFocus() && (*it)->Enabled() && !(*it)->m_bDeleteMe && (!rec || (*it)->SetNextWidget()))
        {
            if (m_FocusedChild != it)
            {
                if (m_FocusedChild != m_ChildList.end())
                    (*m_FocusedChild)->LeaveFocus();
                (*it)->Focus();
                m_FocusedChild = it;
            }
            return true;
        }
        
        it++;
    }
    
    return false;
}

bool CWidgetHandler::SetPrevWidget(bool rec)
{
    if (m_ChildList.empty())
    {
        m_FocusedChild = m_ChildList.end();
        return !Focused();
    }
        
    std::list<CWidgetWindow *>::iterator it = ((Focused()) ? m_FocusedChild : m_ChildList.end());
    
    if (it == m_ChildList.end())
        it--;
    
    while (true)
    {
        if ((*it)->CanFocus() && (*it)->Enabled() && !(*it)->m_bDeleteMe && (!rec || (*it)->SetPrevWidget()))
        {
            if (m_FocusedChild != it)
            {
                if (m_FocusedChild != m_ChildList.end())
                    (*m_FocusedChild)->LeaveFocus();
                (*it)->Focus();
                m_FocusedChild = it;
            }
            return true;
        }
        
        if (it == m_ChildList.begin())
            break;
        
        it--;
    }
    
    return false;
}

void CWidgetHandler::Focus()
{
    CWidgetHandler *o = m_pOwner;
    while (o && o->Enabled() && o->Focused())
        o = o->m_pOwner;
    
    if (o && o->m_pOwner) // Some parent is disabled/not focused, don't try to focus this widget
        return;
    
    m_bFocused = true;
    
    if (m_FocusedChild != m_ChildList.end())
        (*m_FocusedChild)->Focus();
    
    for (std::list<CWidgetWindow *>::iterator it=m_ChildList.begin(); it!=m_ChildList.end(); it++)
        assert(m_ChildList.empty() || it == m_FocusedChild || !(*it)->Focused());
}

void CWidgetHandler::LeaveFocus()
{
    m_bFocused = false;
    
    if (m_FocusedChild != m_ChildList.end())
        (*m_FocusedChild)->LeaveFocus();
    
    for (std::list<CWidgetWindow *>::iterator it=m_ChildList.begin(); it!=m_ChildList.end(); it++)
        assert(m_ChildList.empty() || (*it)->Focused() == false);
}

void CWidgetHandler::PushEvent(int type)
{
    CWidgetHandler *owner = m_pOwner;
    
    while (owner && !owner->HandleEvent(this, type))
        owner = owner->m_pOwner;
}

void CWidgetHandler::_AddChild(CWidgetWindow *p)
{
    assert(this == p->m_pOwner);
    
    p->CreateInit();
    
    m_ChildList.push_back(p);
    if (p->CanFocus() && p->Enabled())
    {
        m_NeedFocChild = m_ChildList.end();
        m_NeedFocChild--;
    }
}

void CWidgetHandler::RemoveChild(CWidgetWindow *p)
{
    if (*m_FocusedChild == p)
    {
        if (!SetPrevWidget())
            SetNextWidget();
    }
    
    bool changed = (*m_FocusedChild != p);
    m_ChildList.remove(p);
    
    if (!changed)
    {
        // Have to do this after the child was removed from the list, otherwise it would be invalid
        m_FocusedChild = m_ChildList.end();
    }
    
    delete p;
}

void CWidgetHandler::ActivateChild(CWidgetWindow *p)
{
    if (!p->Focused())
    {
        if ((m_FocusedChild != m_ChildList.end()) && (*m_FocusedChild)->CanFocus())
        {
            (*m_FocusedChild)->LeaveFocus();
        }
        m_FocusedChild = std::find(m_ChildList.begin(), m_ChildList.end(), p);
        assert(m_FocusedChild != m_ChildList.end());
        p->Focus();
    }
    m_NeedFocChild = m_ChildList.end();
}

void CWidgetHandler::Enable(bool e)
{
    m_bEnabled = e;

    if (!e)
    {
        // Disabled widgets shouldn't have focus
        if ((m_pOwner->m_FocusedChild != m_pOwner->m_ChildList.end()) && (*m_pOwner->m_FocusedChild == this))
        {
            if (!m_pOwner->SetPrevWidget(false))
                m_pOwner->SetNextWidget(false);
        }
    }
}

// -------------------------------------
// Widget manager class
// -------------------------------------

CButtonBar *CWidgetManager::m_pButtonBar = NULL;

void CWidgetManager::Init()
{
    // Init all default colors
    CWidgetWindow::m_cDefaultFocusedColors = ' ' | CWidgetWindow::GetColorPair(COLOR_YELLOW, COLOR_BLUE) | A_BOLD;
    CWidgetWindow::m_cDefaultDefocusedColors = ' ' | CWidgetWindow::GetColorPair(COLOR_WHITE, COLOR_BLUE) | A_BOLD;

    CButton::m_cDefaultFocusedColors = ' ' | CWidgetWindow::GetColorPair(COLOR_YELLOW, COLOR_BLUE) | A_BOLD;
    CButton::m_cDefaultDefocusedColors = ' ' | CWidgetWindow::GetColorPair(COLOR_WHITE, COLOR_BLUE) | A_BOLD;

    CTextLabel::m_cDefaultFocusedColors = ' ' | CWidgetWindow::GetColorPair(COLOR_WHITE, COLOR_BLUE) | A_BOLD;
    CTextLabel::m_cDefaultDefocusedColors = ' ' | CWidgetWindow::GetColorPair(COLOR_WHITE, COLOR_BLUE) | A_BOLD;
    
    CTextWindow::m_cDefaultFocusedColors = ' ' | CWidgetWindow::GetColorPair(COLOR_YELLOW, COLOR_BLUE) | A_BOLD;
    CTextWindow::m_cDefaultDefocusedColors = ' ' | CWidgetWindow::GetColorPair(COLOR_WHITE, COLOR_BLUE) | A_BOLD;

    CMenu::m_cDefaultFocusedColors = ' ' | CWidgetWindow::GetColorPair(COLOR_YELLOW, COLOR_BLUE) | A_BOLD;
    CMenu::m_cDefaultDefocusedColors = ' ' | CWidgetWindow::GetColorPair(COLOR_WHITE, COLOR_BLUE) | A_BOLD;

    CInputField::m_cDefaultFocusedColors = ' ' | CWidgetWindow::GetColorPair(COLOR_YELLOW, COLOR_RED) | A_BOLD;
    CInputField::m_cDefaultDefocusedColors = ' ' | CWidgetWindow::GetColorPair(COLOR_WHITE, COLOR_RED) | A_BOLD;
    
    CProgressbar::m_cDefaultFillColors = ' ' | CWidgetWindow::GetColorPair(COLOR_BLUE, COLOR_YELLOW) | A_BOLD;
    CProgressbar::m_cDefaultEmptyColors = ' ' | CWidgetWindow::GetColorPair(COLOR_WHITE, COLOR_BLUE) | A_BOLD;
    
    CCheckbox::m_cDefaultFocusedColors = ' ' | CWidgetWindow::GetColorPair(COLOR_YELLOW, COLOR_BLUE) | A_BOLD;
    CCheckbox::m_cDefaultDefocusedColors = ' ' | CWidgetWindow::GetColorPair(COLOR_WHITE, COLOR_BLUE) | A_BOLD;

    CRadioButton::m_cDefaultFocusedColors = ' ' | CWidgetWindow::GetColorPair(COLOR_YELLOW, COLOR_BLUE) | A_BOLD;
    CRadioButton::m_cDefaultDefocusedColors = ' ' | CWidgetWindow::GetColorPair(COLOR_WHITE, COLOR_BLUE) | A_BOLD;
    
    CButtonBar::m_cDefaultColors = ' ' | CWidgetWindow::GetColorPair(COLOR_YELLOW, COLOR_RED) | A_BOLD;

    CMessageBox::m_cDefaultFocusedColors = ' ' | CWidgetWindow::GetColorPair(COLOR_YELLOW, COLOR_BLUE) | A_BOLD;
    CMessageBox::m_cDefaultDefocusedColors = ' ' | CWidgetWindow::GetColorPair(COLOR_WHITE, COLOR_BLUE) | A_BOLD;

    CWarningBox::m_cDefaultFocusedColors = ' ' | CWidgetWindow::GetColorPair(COLOR_YELLOW, COLOR_RED) | A_BOLD;
    CWarningBox::m_cDefaultDefocusedColors = ' ' | CWidgetWindow::GetColorPair(COLOR_WHITE, COLOR_RED) | A_BOLD;
    
    CYesNoBox::m_cDefaultFocusedColors = ' ' | CWidgetWindow::GetColorPair(COLOR_YELLOW, COLOR_BLUE) | A_BOLD;
    CYesNoBox::m_cDefaultDefocusedColors = ' ' | CWidgetWindow::GetColorPair(COLOR_WHITE, COLOR_BLUE) | A_BOLD;

    CChoiceBox::m_cDefaultFocusedColors = ' ' | CWidgetWindow::GetColorPair(COLOR_YELLOW, COLOR_BLUE) | A_BOLD;
    CChoiceBox::m_cDefaultDefocusedColors = ' ' | CWidgetWindow::GetColorPair(COLOR_WHITE, COLOR_BLUE) | A_BOLD;

    CInputDialog::m_cDefaultFocusedColors = ' ' | CWidgetWindow::GetColorPair(COLOR_YELLOW, COLOR_BLUE) | A_BOLD;
    CInputDialog::m_cDefaultDefocusedColors = ' ' | CWidgetWindow::GetColorPair(COLOR_WHITE, COLOR_BLUE) | A_BOLD;

    CFileDialog::m_cDefaultFocusedColors = ' ' | CWidgetWindow::GetColorPair(COLOR_YELLOW, COLOR_BLUE) | A_BOLD;
    CFileDialog::m_cDefaultDefocusedColors = ' ' | CWidgetWindow::GetColorPair(COLOR_WHITE, COLOR_BLUE) | A_BOLD;
    
    CMenuDialog::m_cDefaultFocusedColors = ' ' | CWidgetWindow::GetColorPair(COLOR_YELLOW, COLOR_BLUE) | A_BOLD;
    CMenuDialog::m_cDefaultDefocusedColors = ' ' | CWidgetWindow::GetColorPair(COLOR_WHITE, COLOR_BLUE) | A_BOLD;
    
    m_pButtonBar = AddChild(new CButtonBar(this, 2, MaxX(), RawMaxY()-2, 0));
    m_pButtonBar->AddGlobalButton("TAB", "Next field");
}

void CWidgetManager::Refresh()
{
    ::refresh();

    for (std::list<CWidgetWindow *>::iterator it=m_ChildList.begin(); it!=m_ChildList.end(); it++)
    {
        if ((*it)->Enabled() && !(*it)->m_bDeleteMe)
            (*it)->refresh();
    }
}

void CWidgetManager::_AddChild(CWidgetWindow *p)
{
    CWidgetHandler::_AddChild(p);
    
    if (m_NeedFocChild != m_ChildList.end())
    {
        std::list<CWidgetWindow *>::iterator it = m_NeedFocChild;
        m_NeedFocChild = m_ChildList.end(); // refresh maybe called again before if loop has finished
        
        if (m_FocusedChild != m_ChildList.end())
        {
            if ((*it)->CanFocus() && (*it)->Enabled() && (*m_FocusedChild)->CanFocus())
                (*m_FocusedChild)->LeaveFocus();
        }
        
        if ((*it)->Enabled())
        {
            m_FocusedChild = it;
            
            // HACK: Child widgets were focused before parent, so bbar is not in the right order. By refocusing all the child
            // widgets leave focus and get focus in the right order.
//             p->LeaveFocus();
            (*it)->Focus();
        }
    }
}

void CWidgetManager::RemoveChild(CWidgetWindow *p)
{
    p->erase();
    
    p->m_bDeleteMe = true;

    if ((m_FocusedChild != m_ChildList.end()) && (*m_FocusedChild == p))
    {
        if (!SetPrevWidget(false))
            SetNextWidget(false);
    }
    
    ::erase();
    Refresh();
}

void CWidgetManager::ActivateChild(CWidgetWindow *p)
{
    bool dorefresh = false;
    
    if (m_ChildList.back() != p)
    { 
        for (std::list<CWidgetWindow *>::iterator it=m_ChildList.begin(); it!=m_ChildList.end(); it++)
        {
            if (*it == p)
            {
                m_ChildList.erase(it);
                m_ChildList.push_back(p); // Put it on top
                dorefresh = true;
                break;
            }
        }
    }
    
    CWidgetHandler::ActivateChild(p);
    
    if (dorefresh)
        Refresh();
}

bool CWidgetManager::Run(unsigned delay)
{
    if (m_bQuit)
        return false;
    
    // Check for widgets that should be removed
    std::list<CWidgetWindow *>::iterator it = m_ChildList.begin(), tmp;
    for (;it!=m_ChildList.end(); it++)
    {
        if ((*it)->m_bDeleteMe)
        {
            if (m_FocusedChild == it)
            {
                if (!SetPrevWidget(false))
                    SetNextWidget(false);
            }
            
            bool changed = (m_FocusedChild != it);
            
            tmp = it;
            it--; // Decrease, so that for loop doesn't try to increment deleted iter
            
            delete *tmp;
            m_ChildList.erase(tmp);

            if (!changed)
            {
                // Have to do this after the child was removed from the list, otherwise it would be invalid
                m_FocusedChild = m_ChildList.end();
            }
        }
    }
    
    timeout(delay);
    
    if (m_FocusedChild != m_ChildList.end())
    {
        chtype ch = getch();
        if (ch != static_cast<chtype>(ERR))
        {
            if (!(*m_FocusedChild)->HandleKey(ch))
            {
                if (!m_pBoundKeyWidget || !m_pBoundKeyWidget->HandleKey(ch))
                {
                    if ((ch == 9) || (ch == CTRL('n')))
                        SetNextChildWidget();
                    else if ((ch == KEY_BTAB) || (ch == CTRL('p')))
                        SetPrevChildWidget();
                    else if (ch == CTRL('[')) // Escape pressed
                    {
                        // Set this so that later calls will also return false. This is required incase this function
                        // wasn't called from the main app.
                        m_bQuit = true;
                        return false;
                    }
                }
            }
        }
    }
    
    return true;
}

bool CWidgetManager::SetNextChildWidget()
{
    if (m_ChildList.empty())
    {
        m_FocusedChild = m_ChildList.end();
        return false;
    }
    
    std::list<CWidgetWindow *>::iterator prev = (*m_FocusedChild)->m_FocusedChild, cur;
    
    if (prev == (*m_FocusedChild)->m_ChildList.end())
        prev = (*m_FocusedChild)->m_ChildList.begin();
    
    cur = prev;

    do
    {
        if (cur == (*m_FocusedChild)->m_ChildList.end())
            cur = (*m_FocusedChild)->m_ChildList.begin();
        
        if ((*cur)->CanFocus() && (*cur)->Enabled() && !(*cur)->m_bDeleteMe && (*cur)->SetNextWidget())
        {
            if (cur != (*m_FocusedChild)->m_FocusedChild)
            {
                if ((*m_FocusedChild)->m_FocusedChild != (*m_FocusedChild)->m_ChildList.end())
                    (*(*m_FocusedChild)->m_FocusedChild)->LeaveFocus();
                (*m_FocusedChild)->m_FocusedChild = cur;
                (*cur)->Focus();
            }
            return true;
        }
        cur++;
    }
    while (cur != prev);
    
    return false;
}

bool CWidgetManager::SetPrevChildWidget()
{
    if (m_ChildList.empty())
    {
        m_FocusedChild = m_ChildList.end();
        return false;
    }
    
    std::list<CWidgetWindow *>::iterator prev = (*m_FocusedChild)->m_FocusedChild, cur;
    
    if (prev == (*m_FocusedChild)->m_ChildList.end())
        prev = (*m_FocusedChild)->m_ChildList.begin();
    
    cur = prev;

    do
    {
        if ((*cur)->CanFocus() && (*cur)->Enabled() && !(*cur)->m_bDeleteMe && (*cur)->SetPrevWidget())
        {
            if (cur != (*m_FocusedChild)->m_FocusedChild)
            {
                if ((*m_FocusedChild)->m_FocusedChild != (*m_FocusedChild)->m_ChildList.end())
                    (*(*m_FocusedChild)->m_FocusedChild)->LeaveFocus();
                (*m_FocusedChild)->m_FocusedChild = cur;
                (*cur)->Focus();
            }
            return true;
        }
        
        if (cur == (*m_FocusedChild)->m_ChildList.begin())
            cur = (*m_FocusedChild)->m_ChildList.end();

        cur--;
    }
    while (cur != prev);
    
    return false;
}

// -------------------------------------
// Formatted text class
// -------------------------------------

CFormattedText::CFormattedText(CWidgetWindow *w, const std::string &str, bool wrap,
                               TSTLVecSize maxh) : m_pWindow(w), m_TextLength(0), m_LongestLine(0), m_Width(w->width()),
                                                   m_MaxHeight(maxh), m_bWrap(wrap), m_bHandleTags(true)
{
    if (!str.empty())
        AddText(str);
}

CFormattedText::TColorTagList::iterator CFormattedText::GetNextColorTag(TColorTagList::iterator cur, TSTLStrSize curpos)
{
    TColorTagList::iterator ret = m_ColorTags.end();
    if (cur != m_ColorTags.begin())
        cur++; // Start searching on next iter
    
    for (TColorTagList::iterator it=cur; it!=m_ColorTags.end(); it++)
    {
        if (!it->second)
            continue;
        
        if ((it->first > curpos) || (it->second->count >= (curpos - it->first)+1)) // In reach
        {
            ret = it;
            break;
        }
    }
    
    return ret;
}

CFormattedText::TRevTagList::iterator CFormattedText::GetNextRevTag(TRevTagList::iterator cur, TSTLStrSize curpos)
{
    TRevTagList::iterator ret = m_ReversedTags.end();
    if (cur != m_ReversedTags.begin())
        cur++; // Start searching on next iter
    
    for (TRevTagList::iterator it=cur; it!=m_ReversedTags.end(); it++)
    {
        if (!it->second)
            continue;
        
        if ((it->first > curpos) || (it->second >= (curpos - it->first)+1)) // In reach
        {
            ret = it;
            break;
        }
    }
    
    return ret;
}

TSTLVecSize CFormattedText::CalcLines(const std::string &str, bool wrap, TSTLStrSize width)
{
    std::string newtext;
    
    TSTLStrSize strstart = 0, strend, length = str.length();
    bool handletags = true;

    while (strstart < length)
    {
        if (str[strstart] == '<')
        {
            if (!str.compare(strstart+1, 5, "notg>")) // Stop processing tags
            {
                strstart += 6; // "<notg>"
                handletags = false;
                continue;
            }
            else if (!str.compare(strstart+1, 6, "/notg>")) // Stop processing tags - end tag
            {
                strstart += 7; // "</notg>"
                handletags = true;
                continue;
            }
            else if (!handletags)
                ; // Nothing
            else if (!str.compare(strstart+1, 4, "col=")) // Color tag
            {
                strstart += 5; // length of <col= == 5
                
                // Get foreground color
                strend = str.find(":", strstart);
                
                // Get background color
                strstart = strend+1;
                strend = str.find(">", strstart);
                
                strstart = strend+1;
                continue;
            }
            else if (!str.compare(strstart+1, 5, "/col>")) // End color tag
            {
                strstart += 6;
                continue;
            }
            else if (!str.compare(strstart+1, 4, "rev>")) // Reverse tag
            {
                strstart += 5; // "<rev>";
                continue;
            }
            else if (!str.compare(strstart+1, 5, "/rev>")) // End reverse tag
            {
                strstart += 6; // "</rev>";
                continue;
            }
            else if (!str.compare(strstart+1, 2, "C>"))
            {
                strstart += 3;
                continue;
            }
        }
        
        newtext += str[strstart];
        strstart++;
    }
    
    // Now process tagless text
    strstart = 0;
    length = newtext.length();
    TSTLStrSize curlinelen = 0;
    TSTLVecSize lines = 1;
    
    if (wrap)
    {
        do
        {
            strend = strstart + (width-1);
            
            if (strend < strstart) // Overflow
                strend = std::numeric_limits<TSTLStrSize>::max()-1;
            
            if (curlinelen)
                strend -= ((strend-strstart) - (curlinelen-1));
            
            if (strend >= length)
                strend = length - 1;
            
            TSTLStrSize newline = newtext.substr(strstart, (strend-strstart)+1).find("\n");
            bool add = false;
            if (newline != std::string::npos)
            {
                strend = strstart + newline;
                add = true; // Found a newline, so create a new line entry at the end of the loop
            }
            
            if ((strend + 1) != length)
            {
                TSTLStrSize begwind = strend;
                while ((begwind >= strstart) && !isspace(newtext[begwind]))
                    begwind--;
                
                if ((begwind >= strstart) && (begwind != strend))
                {
                    TSTLStrSize endwind = newtext.find_first_of(" \t\n", begwind+1);
                    if (endwind == std::string::npos)
                        endwind = length; // NOT -1, because normally endwind would be point to a whitespace after the word
                    
                    if (((endwind - begwind)-1 <= width) && (endwind-1 != strend)) 
                        strend = begwind;
                }
            }
            
            bool toolong = (curlinelen + ((strend-strstart)+1)) > width;
            
            if (((strend+1) < length) && isspace(newtext[strend+1]) && (newtext[strend] != '\n'))
                strend++; // Don't add leading whitespace to a new line

            if (!toolong) // If it's not too long add to current line
                curlinelen += ((strend - strstart) + 1);
            else
            {
                lines++;
                curlinelen = (strend - strstart) + 1;
            }
            
            if (add)
            {
                curlinelen = 0;
                lines++;
            }
            
            strstart = strend + 1;
        }
        while (strstart < length);
    }
    else
    {
        do
        {
            strend = newtext.find("\n", strstart);
            lines++;
            strstart = strend + 1;
        }
        while ((strend != std::string::npos) && ((strend+1) < length));
    }
    
    if (!curlinelen) // Last line is usually empty
        lines--;
    
    return lines;
}

void CFormattedText::AddText(const std::string &str)
{
    if ((m_Lines.size() > m_MaxHeight) || (str.empty()))
        return;
    
    std::string newtext;
    TSTLStrSize strstart = 0, strend, length = str.length();
    TSTLStrSize startcolor = 0, startrev = 0;
    int fgcolor = -1, bgcolor = -1;
    
    while (strstart < length)
    {
        if (str[strstart] == '<')
        {
            if (!str.compare(strstart+1, 5, "notg>")) // Stop processing tags
            {
                strstart += 6; // "<notg>"
                m_bHandleTags = false;
                continue;
            }
            else if (!str.compare(strstart+1, 6, "/notg>")) // Stop processing tags - end tag
            {
                strstart += 7; // "</notg>"
                m_bHandleTags = true;
                continue;
            }
            else if (!m_bHandleTags)
                ; // Nothing
            else if (!str.compare(strstart+1, 4, "col=")) // Color tag
            {
                startcolor = m_TextLength;
                strstart += 5; // length of <col= == 5
                
                // Get foreground color
                strend = str.find(":", strstart);
                fgcolor = atoi(str.substr(strstart, (strend-strstart)+1).c_str());
                
                // Get background color
                strstart = strend+1;
                strend = str.find(">", strstart);
                bgcolor = atoi(str.substr(strstart, (strend-strstart)+1).c_str());
                
                strstart = strend+1;
                continue;
            }
            else if (!str.compare(strstart+1, 5, "/col>")) // End color tag
            {
                if ((fgcolor != -1) && (bgcolor != -1))
                    m_ColorTags[startcolor] = new color_entry_s(m_TextLength-startcolor, fgcolor, bgcolor);
                strstart += 6;
                fgcolor = bgcolor = -1;
                continue;
            }
            else if (!str.compare(strstart+1, 4, "rev>")) // Reverse tag
            {
                startrev = m_TextLength;
                strstart += 5; // "<rev>";
                continue;
            }
            else if (!str.compare(strstart+1, 5, "/rev>")) // End reverse tag
            {
                m_ReversedTags[startrev] = m_TextLength-startrev;
                strstart += 6; // "</rev>";
                continue;
            }
            else if (!str.compare(strstart+1, 2, "C>"))
            {
                m_CenteredIndexes.insert(m_TextLength);
                strstart += 3;
                continue;
            }
        }
        
        newtext += str[strstart];
        m_TextLength++;
        strstart++;
    }
    
    // Now process tagless text
    length = newtext.length();
    strstart = 0;
    
    if (!length)
        return;
    
    if (m_Lines.empty())
    {
        m_Lines.push_back(new line_entry_s);
        m_CurrentLine = 0;
    }
    
    if (m_bWrap)
    {
        do
        {
            strend = strstart + (m_Width-1);
            
            if (strend < strstart) // Overflow
                strend = std::numeric_limits<TSTLStrSize>::max()-1;
            
            if (!m_Lines[m_CurrentLine]->text.empty())
                strend -= ((strend-strstart) - (m_Lines[m_CurrentLine]->text.length()-1));
            
            if (strend >= length)
                strend = length - 1;
            
            TSTLStrSize newline = newtext.substr(strstart, (strend-strstart)+1).find("\n");
            bool add = false;
            if (newline != std::string::npos)
            {
                strend = strstart + newline;
                add = true; // Found a newline, so create a new line entry at the end of the loop
            }
            
            if ((strend + 1) != length)
            {
                TSTLStrSize begwind = strend;
                while ((begwind >= strstart) && !isspace(newtext[begwind]))
                    begwind--;
                
                if ((begwind >= strstart) && (begwind != strend))
                {
                    TSTLStrSize endwind = newtext.find_first_of(" \t\n", begwind+1);
                    if (endwind == std::string::npos)
                        endwind = length; // NOT -1, because normally endwind would be point to a whitespace after the word
                    
                    if (((endwind - begwind)-1 <= m_Width) && (endwind-1 != strend)) 
                        strend = begwind;
                }
            }
            
            bool toolong = ((m_Lines[m_CurrentLine]->text.length() + (strend-strstart)+1) > m_Width);
            
            if ((toolong || add) && (m_Lines.size() >= m_MaxHeight))
                break;
            
            if (((strend+1) < length) && isspace(newtext[strend+1]) && (newtext[strend] != '\n'))
                strend++; // Don't add trailing whitespace to a new line

            if (!toolong) // If it's not too long add to current line
                m_Lines[m_CurrentLine]->text += newtext.substr(strstart, (strend - strstart) + 1);
            else
            {
                m_Lines.push_back(new line_entry_s);
                m_CurrentLine++;
                m_Lines[m_CurrentLine]->text = newtext.substr(strstart, (strend - strstart) + 1);
            }
            
            if (add)
            {
                m_Lines.push_back(new line_entry_s);
                m_CurrentLine++;
            }
            
            TSTLStrSize newlen = m_Lines[m_CurrentLine]->text.length();
            if (newlen > m_LongestLine)
                m_LongestLine = newlen;
            
            strstart = strend + 1;
        }
        while (strstart < length);
    }
    else
    {
        do
        {
            strend = newtext.find("\n", strstart);
            
            if (strend != std::string::npos)
                m_Lines[m_CurrentLine]->text = newtext.substr(strstart, (strend-strstart)+1);
            else
                m_Lines[m_CurrentLine]->text = newtext.substr(strstart);
            
            TSTLStrSize newlen = m_Lines[m_CurrentLine]->text.length();
            if (newlen > m_LongestLine)
                m_LongestLine = newlen;

            m_CurrentLine++;
            m_Lines.push_back(new line_entry_s);
            
            strstart = strend + 1;
        }
        while ((strend != std::string::npos) && ((strend+1) < length) && (m_CurrentLine < m_MaxHeight));
    }
}

void CFormattedText::Print(TSTLVecSize startline, TSTLStrSize startw, TSTLVecSize endline, TSTLStrSize endw)
{
    if (m_Lines.empty())
        return;
    
    TSTLStrSize chars = 0, index;
    TSTLVecSize y=0;
    
    if ((endline + (startline+1)) < endline)
        endline = std::numeric_limits<TSTLVecSize>::max(); // Overflow
    else
        endline += (startline+1);
    
    if (endline > m_Lines.size())
        endline = m_Lines.size();

    // Add characters we don't print
    if (startline > 0)
    {
        for (TSTLVecSize n=0; n<startline; n++)
            chars += m_Lines[n]->text.length();
    }
    
    TColorTagList::iterator colorit = GetNextColorTag(m_ColorTags.begin(), 0);
    TRevTagList::iterator revit = GetNextRevTag(m_ReversedTags.begin(), 0);
    
    bool initcolortag = (colorit != m_ColorTags.end());
    bool initrevtag = (revit != m_ReversedTags.end());
    
    for (TSTLVecSize n=startline; n<endline; n++, y++)
    {
        TSTLStrSize len = m_Lines[n]->text.length(), spaces = 0;
        
        index = startw;
        
        if (m_bWrap) // Only enable centering while wrapping for now
        {
            for (TCenterTagList::iterator it=m_CenteredIndexes.begin(); it!=m_CenteredIndexes.end(); it++)
            {
                TSTLStrSize centerw = m_Width;
                if ((*it >= chars) && (*it <= (chars+centerw)) && (*it <= (chars+len)))
                {
                    std::string tmp = m_Lines[n]->text;
                    EatWhite(tmp);
                    
                    spaces = (centerw-tmp.length())/2;
                    
                    if (spaces <= startw)
                        spaces = 0; // Can't center, starting width is too high
                    else
                        spaces -= startw;
                    
                    break;
                }
            }
        }
        
        TSTLStrSize count = (len > endw) ? endw : len;
        TSTLStrSize x = 0;
        
        if (((index+1) + count) < count) // Overflow
            count = std::numeric_limits<TSTLStrSize>::max(); // Overflow
        else
            count += (index+1);
    
        if (count > len)
            count = len;
        
        chars += index;
        
        while (index < count)
        {
            // Line may contain trailing whitespace or newline which should not be printed.
            bool canprint = (!m_bWrap || (index <= m_Width)) && (((index+1)!=len) || !isspace(m_Lines[n]->text[index]));
            
            if ((colorit != m_ColorTags.end()) && (colorit->first <= chars) &&
                (colorit->second->count < (chars-colorit->first)+1))
            {
                m_pWindow->attroff(CWidgetWindow::GetColorPair(colorit->second->fgcolor,
                                   colorit->second->bgcolor));
                colorit = GetNextColorTag(colorit, chars);
                initcolortag = (colorit != m_ColorTags.end());
            }
            
            if ((revit != m_ReversedTags.end()) && (revit->first <= chars) && (revit->second < (chars-revit->first)+1))
            {
                m_pWindow->attroff(A_REVERSE);
                revit = GetNextRevTag(revit, chars);
                initrevtag = (revit != m_ReversedTags.end());
            }

            if (initcolortag && (colorit != m_ColorTags.end()) && (colorit->first <= chars))
            {
                m_pWindow->attron(CWidgetWindow::GetColorPair(colorit->second->fgcolor,
                                  colorit->second->bgcolor));
                initcolortag = false;
            }
            
            if (initrevtag && (revit != m_ReversedTags.end()) && (revit->first <= chars))
            {
                m_pWindow->attron(A_REVERSE);
                initrevtag = false;
            }

            if (canprint)
            {
                // Use addstr instead of addch, since it handles multibyte characters for us
                m_pWindow->addstr(y, spaces+x, &m_Lines[n]->text[index], 1);
            }
            
            index++;
            chars++;
            x++;
        }
        chars += (len-count);
    }
    
    // Disable enabled tags
    if (colorit != m_ColorTags.end())
        m_pWindow->attroff(CWidgetWindow::GetColorPair(colorit->second->fgcolor,
                           colorit->second->bgcolor));
    
    if (revit != m_ReversedTags.end())
        m_pWindow->attroff(A_REVERSE);
}

TSTLVecSize CFormattedText::GetLines()
{
    TSTLVecSize ret = m_Lines.size();
    if (ret && m_Lines[ret-1]->text.empty()) // Last line may be empty
        ret--;
    return ret;
}

void CFormattedText::Clear()
{
    for (TColorTagList::iterator it=m_ColorTags.begin(); it!=m_ColorTags.end(); it++)
        delete it->second;
    
    for (std::vector<line_entry_s *>::iterator it=m_Lines.begin(); it!=m_Lines.end(); it++)
        delete *it;
    
    m_ColorTags.clear();
    m_ReversedTags.clear();
    m_CenteredIndexes.clear();
    m_Lines.clear();
    m_TextLength = m_CurrentLine = m_LongestLine = 0;
    m_bHandleTags = true;
}

void CFormattedText::AddCenterTag(TSTLVecSize line)
{
    TSTLStrSize chars = 0, size = m_Lines.size();
    for (TSTLStrSize n=0; n<size; n++)
    {
        if (n < line)
            chars += m_Lines[n]->text.length();
        else
            break;
    }

    m_CenteredIndexes.insert(chars+1);
}

void CFormattedText::AddRevTag(TSTLVecSize line, TSTLStrSize pos, TSTLStrSize c)
{
    TSTLStrSize chars = 0, size = m_Lines.size();
    for (TSTLStrSize n=0; n<size; n++)
    {
        if (n < line)
            chars += m_Lines[n]->text.length();
        else
        {
            chars += pos;
            break;
        }
    }

    m_ReversedTags[chars] = c;
}

void CFormattedText::AddColorTag(TSTLVecSize line, TSTLStrSize pos, TSTLStrSize c, int fg, int bg)
{
    TSTLStrSize chars = 0, size = m_Lines.size();
    for (TSTLStrSize n=0; n<size; n++)
    {
        if (n < line)
            chars += m_Lines[n]->text.length();
        else
        {
            chars += pos;
            break;
        }
    }

    if (m_ColorTags[chars])
        delete m_ColorTags[chars];
    
    m_ColorTags[chars] = new color_entry_s(c, fg, bg);
}

void CFormattedText::DelCenterTag(TSTLVecSize line)
{
    TSTLStrSize chars = 0, size = m_Lines.size();
    for (TSTLStrSize n=0; n<size; n++)
    {
        if (n < line)
            chars += m_Lines[n]->text.length();
        else
            break;
    }

    m_CenteredIndexes.erase(chars+1);
}

void CFormattedText::DelRevTag(TSTLVecSize line, TSTLStrSize pos)
{
    TSTLStrSize chars = 0, size = m_Lines.size();
    for (TSTLStrSize n=0; n<size; n++)
    {
        if (n < line)
            chars += m_Lines[n]->text.length();
        else
        {
            chars += pos;
            break;
        }
    }

    m_ReversedTags.erase(chars);
}

void CFormattedText::DelColorTag(TSTLVecSize line, TSTLStrSize pos)
{
    TSTLStrSize chars = 0, size = m_Lines.size();
    for (TSTLStrSize n=0; n<size; n++)
    {
        if (n < line)
            chars += m_Lines[n]->text.length();
        else
        {
            chars += pos;
            break;
        }
    }

    delete m_ColorTags[chars];
    m_ColorTags.erase(chars);
}

// -------------------------------------
// Widget window class
// -------------------------------------

CWidgetWindow::ColorMapType CWidgetWindow::m_ColorPairs;
int CWidgetWindow::m_iCurColorPair = 0;
chtype CWidgetWindow::m_cDefaultFocusedColors;
chtype CWidgetWindow::m_cDefaultDefocusedColors;
int CWidgetWindow::m_iCursorY = -1;
int CWidgetWindow::m_iCursorX = -1;

CWidgetWindow::CWidgetWindow(CWidgetManager *owner, int nlines, int ncols, int begin_y, int begin_x, bool box, bool canfocus,
                             chtype fcolor, chtype dfcolor) : CWidgetHandler(owner, canfocus),
                                                              NCursesWindow(nlines, ncols, begin_y, begin_x),
                                                              m_bBox(box), m_bInitialized(false), m_bFocusing(false), m_sCurColor(0), m_cLLCorner(0),
                                                              m_cLRCorner(0), m_cULCorner(0), m_cURCorner(0)
{
    SetColors(fcolor, dfcolor);
}

CWidgetWindow::CWidgetWindow(CWidgetWindow *owner, int nlines, int ncols, int begin_y, int begin_x, char absrel,
                             bool box, bool canfocus, chtype fcolor,
                             chtype dfcolor) : CWidgetHandler(owner, canfocus),
                                               NCursesWindow(*owner, nlines, ncols, begin_y, begin_x, absrel),
                                               m_bBox(box), m_bInitialized(false), m_bFocusing(false), m_sCurColor(0), m_cLLCorner(0), m_cLRCorner(0),
                                               m_cULCorner(0), m_cURCorner(0)
{
    SetColors(fcolor, dfcolor);
}

CWidgetWindow::CWidgetWindow(CWidgetManager *owner, int nlines, int ncols, int begin_y, int begin_x,
                             bool box) : CWidgetHandler(owner), NCursesWindow(nlines, ncols, begin_y, begin_x),
                                         m_bBox(box), m_bInitialized(false), m_bFocusing(false), m_sCurColor(0), m_cLLCorner(0), m_cLRCorner(0), m_cULCorner(0),
                                         m_cURCorner(0)
{
    SetColors(m_cDefaultFocusedColors, m_cDefaultDefocusedColors);
}

CWidgetWindow::CWidgetWindow(CWidgetWindow *owner, int nlines, int ncols, int begin_y, int begin_x, char absrel,
                             bool box) : CWidgetHandler(owner),
                                         NCursesWindow(*owner, nlines, ncols, begin_y, begin_x, absrel),
                                         m_bBox(box), m_bInitialized(false), m_bFocusing(false), m_sCurColor(0), m_cLLCorner(0), m_cLRCorner(0),
                                         m_cULCorner(0), m_cURCorner(0)
{
    SetColors(m_cDefaultFocusedColors, m_cDefaultDefocusedColors);
}

void CWidgetWindow::PushBBar()
{
    // HACK: If buttonbar is NULL this widget is the buttonbar itself
    if (CWidgetManager::m_pButtonBar && !m_Buttons.empty() && CanFocus())
    {
        debugline("Push'in bbar\n");
        CWidgetManager::m_pButtonBar->Push();
            
        for (std::list<SButtonBarEntry>::iterator it=m_Buttons.begin(); it!=m_Buttons.end(); it++)
        {
            if (it->global)
                CWidgetManager::m_pButtonBar->AddGlobalButton(it->button, it->desc);
            else
                CWidgetManager::m_pButtonBar->AddButton(it->button, it->desc);
        }
        CWidgetManager::m_pButtonBar->refresh();
    }
}

void CWidgetWindow::PopBBar()
{
    // HACK: If buttonbar is NULL this widget is the buttonbar itself
    if (CWidgetManager::m_pButtonBar && !m_Buttons.empty() && CanFocus())
    {
        debugline("Pop'in bbar:\n");
        CWidgetManager::m_pButtonBar->Pop();
        CWidgetManager::m_pButtonBar->refresh();
    }
}

void CWidgetWindow::Focus()
{
    if (m_bFocusing) // HACK: When the buttonbar is push'ed it refreshes the whole screen and might therefore call Focus of this widget again
        return;
    
    m_bFocusing = true;
    
    if (!Focused())
        PushBBar();

    // Refresh twice: First apply colors, then redraw widget (this is required for ie A_REVERSE)
    bkgd(m_cFocusedColors);
    refresh();
    CWidgetHandler::Focus();
    refresh();

    // HACK: If widget couldn't focus pop bbar
    if (!Focused())
        PopBBar();
    
    m_bFocusing = false;
}

void CWidgetWindow::LeaveFocus()
{
    if (Focused())
        PopBBar();
    
    bkgd(m_cDefocusedColors);
    refresh();
    CWidgetHandler::LeaveFocus();
    refresh();
}

int CWidgetWindow::refresh()
{
    if (!Enabled() || m_bDeleteMe)
        return 0;
    
    assert(m_bInitialized);

    if (m_NeedFocChild != m_ChildList.end())
    {
        std::list<CWidgetWindow *>::iterator it = m_NeedFocChild;
        m_NeedFocChild = m_ChildList.end(); // refresh maybe called again before if loop has finished
        
        if (m_FocusedChild != m_ChildList.end())
        {
            if ((*it)->CanFocus() && (*it)->Enabled() && (*m_FocusedChild)->CanFocus())
                (*m_FocusedChild)->LeaveFocus();
        }
        
        if ((*it)->Enabled())
        {
            m_FocusedChild = it;
            
            // HACK: Child widgets were focused before parent, so bbar is not in the right order. By refocusing all the child
            // widgets leave focus and get focus in the right order.
//             p->LeaveFocus();
            (*it)->Focus();
        }
    }

    Draw();

    if (m_bBox)
        Box();
    
    if (!m_szTitle.empty())
        addstr(0, (maxx()-m_szTitle.length())/2, m_szTitle.c_str());
        
    int ret = NCursesWindow::refresh();
    
    for (std::list<CWidgetWindow *>::iterator it=m_ChildList.begin(); it!=m_ChildList.end(); it++)
    {
        if ((*it)->Enabled())
            (*it)->refresh();
    }
    
    ApplyCursorPos();
    ::refresh();
    return ret;
}

int CWidgetWindow::mvwin(int begin_y, int begin_x)
{
    int diffy = begy() - begin_y;
    int diffx = begx() - begin_x;
    
    int ret = NCursesWindow::mvwin(begin_y, begin_x);
    
    if (ret != ERR)
    {
        // Child widgets won't move without this...
        if (m_pOwner)
        {
            w->_begy = begin_y;
            w->_begx = begin_x;
        }
        
        // Move all child widgets
        for (std::list<CWidgetWindow *>::iterator it=m_ChildList.begin(); it!=m_ChildList.end(); it++)
        {
            if ((*it)->mvwin((*it)->begy() - diffy, (*it)->begx() - diffx) == ERR)
                return ERR;
        }
    }

    return ret;
}

int CWidgetWindow::GetColorPair(int fg, int bg)
{
    if (!::has_colors())
    {
        // Non-monochrome color requested?
        if (((fg != COLOR_BLACK) && (fg != COLOR_WHITE)) ||
            ((bg != COLOR_BLACK) && (bg != COLOR_WHITE)))
            return GetColorPair(COLOR_WHITE, COLOR_BLACK);
    }
    
    ColorMapType::iterator it = m_ColorPairs.find(fg);
    if (it != m_ColorPairs.end())
    {
        std::map<int, int>::iterator it2 = it->second.find(bg);
        if (it2 != it->second.end())
            return COLOR_PAIR(it2->second);
    }
    
    m_iCurColorPair++;
    ::init_pair(m_iCurColorPair, fg, bg);
    m_ColorPairs[fg][bg] = m_iCurColorPair;
    return COLOR_PAIR(m_iCurColorPair);
}

// -------------------------------------
// Button widget class
// -------------------------------------

chtype CButton::m_cDefaultFocusedColors;
chtype CButton::m_cDefaultDefocusedColors;

CButton::CButton(CWidgetWindow *owner, int nlines, int ncols, int begin_y, int begin_x, const std::string &text,
                 char absrel) : CWidgetWindow(owner, nlines, ncols, begin_y, begin_x, absrel, false, true,
                 m_cDefaultFocusedColors, m_cDefaultDefocusedColors), m_FMText(this, text, true)
{
    m_FMText.AddCenterTag(0);
}

bool CButton::HandleKey(chtype ch)
{
    if (CWidgetWindow::HandleKey(ch))
        return true;
    
    if (ENTER(ch))
    {
        PushEvent(EVENT_CALLBACK);
        return true;
    }
    
    return false;
}

void CButton::Draw()
{
    erase();
    m_FMText.Print();
    debugline("button: %s\n", m_FMText.GetText(0).c_str());
    
    if (Focused())
    {
        // Add < and > at the end of the button text. We do this here so that the rest of the text can center
        addch(0, 0, '<');
        addch(0, maxx(), '>');
    }
}

// -------------------------------------
// Scrollbar widget class
// -------------------------------------

CScrollbar::CScrollbar(CWidgetWindow *owner, int nlines, int ncols, int begin_y, int begin_x, float min, float max,
                       bool vertical, char absrel) : CWidgetWindow(owner, nlines, ncols, begin_y, begin_x,
                                                                   absrel, false, false, m_cDefaultFocusedColors,
                                                                   m_cDefaultDefocusedColors),
                                                     m_fMinVal(min), m_fMaxVal(max), m_fCurVal(min),
                                                     m_bVertical(vertical)
{
}

void CScrollbar::Draw()
{
    if (par)
        bkgd(par->getbkgd()|A_REVERSE);
    else
        bkgd(::getbkgd(stdscr)|A_REVERSE);
    
    // Calc slide position
    float fac = fabs(m_fCurVal / (m_fMaxVal-m_fMinVal));
    float posx, posy;
    
    if (m_bVertical)
    {
        posx = 0.0f;
        posy = (float)maxy() * fac;
    }
    else
    {
        posx = (float)maxx() * fac;
        posy = 0.0f;
    }
    
    if (posx < 0.0f)
        posx = 0.0f;
    if (posy < 0.0f)
        posy = 0.0f;
    
    addch(SafeConvert<int>(posy), SafeConvert<int>(posx), ACS_CKBOARD);
}

void CScrollbar::Scroll(float n)
{
    m_fCurVal += n;
    
    if (m_fCurVal < m_fMinVal)
        m_fCurVal = m_fMinVal;
    else if (m_fCurVal > m_fMaxVal)
        m_fCurVal = m_fMaxVal;
}

// -------------------------------------
// Text label class
// -------------------------------------

chtype CTextLabel::m_cDefaultFocusedColors;
chtype CTextLabel::m_cDefaultDefocusedColors;

CTextLabel::CTextLabel(CWidgetWindow *owner, int nlines, int ncols, int begin_y, int begin_x,
                       char absrel) : CWidgetWindow(owner, 1, ncols, begin_y, begin_x, absrel, false,
                                                   false, m_cDefaultFocusedColors, m_cDefaultDefocusedColors),
                                      m_iMaxHeight(nlines), m_CurLines(1), m_FMText(this, "", true, nlines)
{
}

void CTextLabel::AddText(std::string text)
{
    TSTLVecSize curlines = m_CurLines;
    m_FMText.AddText(text);
    
    if (curlines != m_FMText.GetLines())
    {
        m_CurLines = m_FMText.GetLines();
        resize(SafeConvert<int>(m_CurLines), width());
    }
}

void CTextLabel::Draw()
{
    erase();
    m_FMText.Print();
}

// -------------------------------------
// Text window class
// -------------------------------------

chtype CTextWindow::m_cDefaultFocusedColors;
chtype CTextWindow::m_cDefaultDefocusedColors;

CTextWindow::CTextWindow(CWidgetWindow *owner, int nlines, int ncols, int begin_y, int begin_x, bool wrap, bool follow,
                         char absrel, bool box) : CWidgetWindow(owner, nlines, ncols, begin_y, begin_x, absrel, box,
                                                                box, m_cDefaultFocusedColors, m_cDefaultDefocusedColors),
                                                  m_bWrap(wrap), m_bFollow(follow), m_CurrentLine(0)
{
}

void CTextWindow::CreateInit()
{
    CWidgetWindow::CreateInit();
    
    if (HasBox())
        m_pTextWin = AddChild(new CWidgetWindow(this, height()-2, width()-2, 1, 1, 'r', false)); // Create a new text window
    else
        m_pTextWin = this; // Use current
    
    m_pFMText = new CFormattedText(m_pTextWin, "", m_bWrap);
    
    m_pVScrollbar = AddChild(new CScrollbar(this, height()-2, 1, 1, width()-1, 0, 0, true, 'r'));
    m_pHScrollbar = AddChild(new CScrollbar(this, 1, width()-2, height()-1, 1, 0, 0, false, 'r'));
    
    AddButton("ARROWS", "Scroll");
}

void CTextWindow::HScroll(int n)
{
    if (m_bWrap || (m_pFMText->GetLongestLine() <= static_cast<TSTLVecSize>(m_pTextWin->width())))
        return;
    
    m_pHScrollbar->Scroll(n);
    refresh();
}

void CTextWindow::VScroll(int n)
{
    if (m_pFMText->Empty())
        return;
    
    m_pVScrollbar->Scroll(n);
    TSTLVecSize diff = SafeConvert<TSTLVecSize>(m_pVScrollbar->GetValue()) - m_CurrentLine;
    
    if (diff)
    {
        m_CurrentLine = SafeConvert<TSTLVecSize>(m_pVScrollbar->GetValue());
        refresh();
    }
}

void CTextWindow::ScrollToBottom()
{
    int h = SafeConvert<int>(m_pFMText->GetLines()) - m_pTextWin->height();
    if (h < 0)
        h = 0;
    
    m_pVScrollbar->SetCurrent(h);
    m_CurrentLine = SafeConvert<TSTLVecSize>(m_pVScrollbar->GetValue());
}

bool CTextWindow::HandleKey(chtype ch)
{
    if (CWidgetWindow::HandleKey(ch))
        return true;
    
    bool handled = true;
    
    switch (ch)
    {
        case KEY_LEFT:
            HScroll(-1);
            break;
        case KEY_RIGHT:
            HScroll(1);
            break;
        case KEY_UP:
            VScroll(-1);
            break;
        case KEY_DOWN:
            VScroll(1);
            break;
        case KEY_NPAGE:
            VScroll(m_pTextWin->height());
            break;
        case KEY_PPAGE:
            VScroll(-m_pTextWin->height());
            break;
        default:
            handled = false;
            break;
    }
    
    return handled;
}

void CTextWindow::AddText(std::string text)
{
    m_pFMText->AddText(text);
    
    if (m_pFMText->GetLines() > static_cast<TSTLVecSize>(m_pTextWin->height()))
        m_pVScrollbar->SetMinMax(0.0f, (SafeConvert<int>(m_pFMText->GetLines()) - m_pTextWin->height()));
    
    if (m_pFMText->GetLongestLine() > static_cast<TSTLVecSize>(m_pTextWin->width()))
        m_pHScrollbar->SetMinMax(0.0f, (SafeConvert<int>(m_pFMText->GetLongestLine()) - m_pTextWin->width()));
    
    if (m_bFollow)
        ScrollToBottom();
}

void CTextWindow::Clear()
{
    m_pFMText->Clear();
    m_CurrentLine = 0;
    m_pVScrollbar->SetMinMax(0, 0);
    m_pHScrollbar->SetMinMax(0, 0);
}

void CTextWindow::LoadFile(const char *fname)
{
    std::ifstream file(fname);
    std::string buf;
    char c;
    
    while (file && file.get(c))
        buf += c;
        
    AddText(buf);
}

void CTextWindow::Draw()
{
    erase();
    m_pFMText->Print(m_CurrentLine, SafeConvert<TSTLVecSize>(m_pHScrollbar->GetValue()), m_pTextWin->height(),
                     m_pTextWin->width());
    m_pVScrollbar->Enable((HasBox() && (m_pFMText->GetLines() > SafeConvert<TSTLVecSize>(m_pTextWin->height()))));
    m_pHScrollbar->Enable(!m_bWrap && HasBox() && (m_pFMText->GetLongestLine() > SafeConvert<TSTLVecSize>(m_pTextWin->width())));
}

// -------------------------------------
// Menu text sub class
// -------------------------------------

void CMenu::CMenuText::AddText(const std::string &str)
{
    CFormattedText::AddText(str);
    
    TSTLStrSize chars = 0;
    
    std::list<unsorted_tag_s<color_entry_s *> > ColTagList;
    std::list<unsorted_tag_s<TSTLStrSize> > RevTagList;
    std::list<unsorted_tag_s<TSTLStrSize> > CenterTagList;
    
    for (TSTLVecSize n=0; n<m_Lines.size(); n++)
    {
        TSTLStrSize len = m_Lines[n]->text.length();
        
        for (TColorTagList::iterator colit=m_ColorTags.begin(); colit!=m_ColorTags.end(); colit++)
        {
            if ((colit->first >= chars) && (colit->first < (chars+len)))
            {
                ColTagList.push_back(unsorted_tag_s<color_entry_s *>(colit->second, colit->first-chars, m_Lines[n]));
                TColorTagList::iterator tmpit = colit;
                colit--;
                m_ColorTags.erase(tmpit);
            }
        }
        
        for (TRevTagList::iterator revit=m_ReversedTags.begin(); revit!=m_ReversedTags.end(); revit++)
        {
            if ((revit->first >= chars) && (revit->first < (chars+len)))
            {
                RevTagList.push_back(unsorted_tag_s<TSTLStrSize>(revit->second, revit->first-chars, m_Lines[n]));
                TRevTagList::iterator tmpit = revit;
                revit--;
                m_ReversedTags.erase(tmpit);
            }
        }
        
        for (TCenterTagList::iterator centit=m_CenteredIndexes.begin(); centit!=m_CenteredIndexes.end(); centit++)
        {
            if ((*centit >= chars) && (*centit < (chars+len)))
            {
                CenterTagList.push_back(unsorted_tag_s<TSTLStrSize>(*centit, 0, m_Lines[n]));
                TCenterTagList::iterator tmpit = centit;
                centit--;
                m_CenteredIndexes.erase(tmpit);
            }
        }

        chars += len;
    }
    
    std::sort(m_Lines.begin(), m_Lines.end(), LessThan);
    
    // Restore tags
    for (std::list<unsorted_tag_s<color_entry_s *> >::iterator it=ColTagList.begin(); it!=ColTagList.end(); it++)
    {
        std::vector<line_entry_s *>::iterator newlineit = std::find(m_Lines.begin(), m_Lines.end(), it->line);
        AddColorTag(std::distance(m_Lines.begin(), newlineit), it->lineoffset, it->entry->count, it->entry->fgcolor, it->entry->bgcolor);
    }
    
    for (std::list<unsorted_tag_s<TSTLStrSize> >::iterator it=RevTagList.begin(); it!=RevTagList.end(); it++)
    {
        std::vector<line_entry_s *>::iterator newlineit = std::find(m_Lines.begin(), m_Lines.end(), it->line);
        AddRevTag(std::distance(m_Lines.begin(), newlineit), it->lineoffset, it->entry);
    }

    for (std::list<unsorted_tag_s<TSTLStrSize> >::iterator it=CenterTagList.begin(); it!=CenterTagList.end(); it++)
    {
        std::vector<line_entry_s *>::iterator newlineit = std::find(m_Lines.begin(), m_Lines.end(), it->line);
        AddCenterTag(std::distance(m_Lines.begin(), newlineit));
    }
}

void CMenu::CMenuText::HighLight(TSTLVecSize line, bool h)
{
    if (h)
        AddRevTag(line, 0, m_Lines[line]->text.length());
    else
        DelRevTag(line, 0);
}

TSTLVecSize CMenu::CMenuText::Search(TSTLVecSize min, TSTLVecSize max, const std::string &key)
{
    std::vector<line_entry_s *>::iterator beg = m_Lines.begin()+min, end = m_Lines.begin()+max;
    std::vector<line_entry_s *>::iterator it=std::lower_bound(beg, end, key, LessThanStr);
    return ((*it)->text == key) ? std::distance(m_Lines.begin(), it) : GetLines();
}

TSTLVecSize CMenu::CMenuText::Search(TSTLVecSize min, TSTLVecSize max, char key)
{
    std::vector<line_entry_s *>::iterator beg = m_Lines.begin()+min, end = m_Lines.begin()+max;
    std::vector<line_entry_s *>::iterator it=std::lower_bound(beg, end, key, LessThanCh);
    return (((*it)->text[0] == key) ? std::distance(m_Lines.begin(), it) : GetLines());
}

// -------------------------------------
// Menu class
// -------------------------------------

chtype CMenu::m_cDefaultFocusedColors;
chtype CMenu::m_cDefaultDefocusedColors;

CMenu::CMenu(CWidgetWindow *owner, int nlines, int ncols, int begin_y, int begin_x,
               char absrel) : CWidgetWindow(owner, nlines, ncols, begin_y, begin_x, absrel, true,
                                            true, m_cDefaultFocusedColors, m_cDefaultDefocusedColors),
                              m_bInitCursor(true), m_CursorLine(0), m_StartEntry(0)
{
}

void CMenu::CreateInit()
{
    CWidgetWindow::CreateInit();
    
    m_pTextWin = AddChild(new CWidgetWindow(this, height()-2, width()-2, 1, 1, 'r', false));
    m_pMenuText = new CMenuText(m_pTextWin);
    m_pVScrollbar = AddChild(new CScrollbar(this, height()-2, 1, 1, width()-1, 0, 100, true, 'r'));
    m_pHScrollbar = AddChild(new CScrollbar(this, 1, width()-2, height()-1, 1, 0, 100, false, 'r'));

    AddButton("ARROWS", "Scroll");
}

void CMenu::HScroll(int n)
{
    if (m_pMenuText->GetLongestLine() <= static_cast<TSTLVecSize>(m_pTextWin->width()))
        return;
    
    m_pHScrollbar->Scroll(n);
    refresh();
}

void CMenu::VScroll(int n)
{
#if 1
    TSTLVecSize lines = m_pMenuText->GetLines(), cur = GetCurrent(), curstart = m_StartEntry;
    int h = m_pTextWin->height()-1; // -1 because last y pos is window border

    if (n < 0)
    {
        TSTLStrSize nabs = abs(n);
        if (nabs > m_CursorLine)
        {
            TSTLStrSize rest = (nabs - m_CursorLine);
            
            if (rest > m_StartEntry)
            {
                m_StartEntry = 0;
                m_pVScrollbar->SetCurrent(0);
            }
            else
                m_StartEntry -= rest;
            
            m_CursorLine = 0;
        }
        else
            m_CursorLine -= nabs;
    }
    else
    {
        TSTLVecSize newpos = m_CursorLine + n;
        if (((newpos+m_StartEntry) >= lines) || (newpos < m_CursorLine)) // Don't go past menu size and prevent overflows
            newpos = ((lines-1) - m_StartEntry);
        
        m_CursorLine = newpos;
    }

    if (m_CursorLine > static_cast<TSTLStrSize>(h))
    {
        m_StartEntry += (m_CursorLine - h);
        
        TSTLStrSize max = lines - h;
        if (m_StartEntry > max)
            m_StartEntry = max;
                
        m_CursorLine = h;
    }
    
    if (curstart != m_StartEntry)
        m_pVScrollbar->SetCurrent(SafeConvert<float>(m_StartEntry));
    
    if (cur != GetCurrent())
    {
        m_pMenuText->HighLight(cur, false);
        m_pMenuText->HighLight(GetCurrent(), true);
    }

#else
    bool scroll = false;
    int newline = m_CursorLine + n;
    int cur = GetCurrent();
    
    if (newline < 0)
    {
        if ((m_StartEntry + newline) < 0)
        {
            m_StartEntry = m_CursorLine = 0;
            m_pVScrollbar->SetCurrent(0);
        }
        else
            scroll = true;
    }
    else if (newline >= m_pTextWin->height())
    {
        if ((m_StartEntry + newline) >= m_pMenuText->GetLines())
        {
            m_StartEntry = (m_pMenuText->GetLines() - m_pTextWin->height());
            if (m_StartEntry < 0)
                m_StartEntry = 0;
            m_CursorLine = m_pTextWin->height()-1;
            m_pVScrollbar->SetCurrent(SafeConvert<float>(m_pMenuText->GetLines() - m_pTextWin->height()));
        }
        else
            scroll = true;
    }
    else if (newline >= m_pMenuText->GetLines())
        m_CursorLine = ((m_pMenuText->GetLines()-1)-m_StartEntry);
    else
        m_CursorLine = newline;

    if (scroll)
    {
        int oldstart = m_StartEntry;
        m_pVScrollbar->Scroll(n);
        m_StartEntry = (int)m_pVScrollbar->GetValue();
        
        if (GetCurrent() != (oldstart + newline)) // Not scrolled enough yet?
        {
            m_CursorLine += ((oldstart + newline) - GetCurrent()); // Add(or substract) remaining
            
            if (m_CursorLine < 0)
                m_CursorLine = 0;
            else if (m_CursorLine >= m_pTextWin->height())
                m_CursorLine = m_pTextWin->height()-1;
        }
    }
    
    if (cur != GetCurrent())
    {
        m_pMenuText->HighLight(cur, false);
        m_pMenuText->HighLight(GetCurrent(), true);
    }
#endif
    refresh();
}

bool CMenu::HandleKey(chtype ch)
{
    if (CWidgetWindow::HandleKey(ch))
        return true;
    
    bool handled = true;
    
    switch (ch)
    {
        case KEY_LEFT:
            HScroll(-1);
            PushEvent(EVENT_DATACHANGED);
            break;
        case KEY_RIGHT:
            HScroll(1);
            PushEvent(EVENT_DATACHANGED);
            break;
        case KEY_UP:
            VScroll(-1);
            PushEvent(EVENT_DATACHANGED);
            break;
        case KEY_DOWN:
            VScroll(1);
            PushEvent(EVENT_DATACHANGED);
            break;
        case KEY_ENTER:
        case '\n':
        case '\r':
        {
            PushEvent(EVENT_CALLBACK);
            break;
        }
        case KEY_NPAGE:
            VScroll(m_pTextWin->height()-1);
            PushEvent(EVENT_DATACHANGED);
            break;
        case KEY_PPAGE:
            VScroll(-m_pTextWin->height()+1);
            PushEvent(EVENT_DATACHANGED);
            break;
        default:
            if (!isprint(ch))
                handled = false;
            else // Go to item which starts with typed character
            {
                // First try from current position
                TSTLVecSize line = m_pMenuText->Search(GetCurrent()+1, m_pMenuText->GetLines(), ch);
                if (line == m_pMenuText->GetLines()) // Not found, start from begin
                    line = m_pMenuText->Search(0, GetCurrent(), ch);
                
                if (line != m_pMenuText->GetLines())
                {
                    VScroll(SafeConvert<int>(line) - SafeConvert<int>(GetCurrent()));
                    PushEvent(EVENT_DATACHANGED);
                }
            }
            break;
    }
    
    return handled;
}

void CMenu::Draw()
{
    if (m_bInitCursor && !m_pMenuText->Empty())
    {
        m_bInitCursor = false;
        m_pMenuText->HighLight(GetCurrent(), true);
    }
    
    m_pTextWin->erase();
    m_pMenuText->Print(m_StartEntry, SafeConvert<TSTLVecSize>(m_pHScrollbar->GetValue()), m_pTextWin->height(),
                       m_pTextWin->width());
}

void CMenu::AddItem(std::string s, void *udata, bool tags)
{
    if (udata)
        m_DataMap[s] = udata;
    
    if (!tags) // Don't want to process tags?
        s.insert(0, "<notg>");

    m_pMenuText->AddText(s);

    TSTLVecSize h;
    if (m_pMenuText->GetLines() > static_cast<TSTLVecSize>(m_pTextWin->height()))
        h = m_pMenuText->GetLines() - m_pTextWin->height();
    else
        h = 0;
    
    m_pVScrollbar->SetMinMax(0, SafeConvert<float>(h));
    m_pVScrollbar->Enable((m_pMenuText->GetLines() > SafeConvert<TSTLVecSize>(m_pTextWin->height())));
    
    if (m_pMenuText->GetLongestLine() > static_cast<TSTLVecSize>(m_pTextWin->width()))
    {
        m_pHScrollbar->SetMinMax(0, SafeConvert<float>(m_pMenuText->GetLongestLine() - m_pTextWin->width()));
        m_pHScrollbar->Enable(true);
    }
    else
        m_pHScrollbar->Enable(false);
}

void CMenu::SetCurrent(const std::string &str)
{
    TSTLVecSize line = m_pMenuText->Search(0, m_pMenuText->GetLines(), str);
    
    assert(line < m_pMenuText->GetLines());
    
    if (line < m_pMenuText->GetLines())
    {
        VScroll(SafeConvert<int>(line - GetCurrent()));
        PushEvent(EVENT_DATACHANGED);
    }
}

void CMenu::Clear()
{
    m_pMenuText->Clear();
    m_CursorLine = m_StartEntry = 0;
    m_bInitCursor = true;
    m_pHScrollbar->SetCurrent(0);
    m_pVScrollbar->SetCurrent(0);
    m_pHScrollbar->Enable(false);
    m_pVScrollbar->Enable(false);
}

// -------------------------------------
// Inputfield class
// -------------------------------------

chtype CInputField::m_cDefaultFocusedColors;
chtype CInputField::m_cDefaultDefocusedColors;

CInputField::CInputField(CWidgetWindow *owner, int nlines, int ncols, int begin_y, int begin_x, char absrel, int max,
                         chtype out, EInputType type) : CWidgetWindow(owner, nlines, ncols, begin_y, begin_x,
                                                                      absrel, false, true, m_cDefaultFocusedColors,
                                                                      m_cDefaultDefocusedColors),
                                                        m_chOutChar(out), m_iMaxChars(max), m_CursorPos(0),
                                                        m_ScrollOffset(0), m_FMText(this, "", false), m_eInpType(type)
{
}

void CInputField::Addch(chtype ch)
{
    if ((m_iMaxChars > -1) && (m_szText.length() >= static_cast<TSTLStrSize>(m_iMaxChars)))
        return;

    TSTLStrSize pos = m_ScrollOffset + m_CursorPos;
    if (pos == m_szText.length())
        m_szText += ch;
    else
        m_szText.insert(pos, 1, ch);
    
    ChangeFMText();
    
    MoveCursor(1);
}

void CInputField::Delch(bool backspace)
{
    if (m_szText.length() < 1)
        return;
    
    TSTLStrSize pos = m_ScrollOffset + m_CursorPos;
    
    if (backspace && pos)
        pos--;
    
    if (pos <= m_szText.length())
        m_szText.erase(pos, 1);
    
    ChangeFMText();
    
    if (backspace)
        MoveCursor(-1);
    else if (pos <= m_szText.length())
        refresh(); // MoveCursor would call it otherwise
}

void CInputField::MoveCursor(int n, bool relative)
{
    TSTLStrSize len = m_szText.length();
    int w = width()-1; // -1, because we want to scroll before char is at last available position
    
    if (relative)
    {
        if (n < 0)
        {
            TSTLStrSize nabs = abs(n);
            if (nabs > m_CursorPos)
            {
                TSTLStrSize rest = (nabs - m_CursorPos);
                
                if (rest > m_ScrollOffset)
                    m_ScrollOffset = 0;
                else
                    m_ScrollOffset -= rest;
                
                m_CursorPos = 0;
            }
            else
                m_CursorPos -= nabs;
        }
        else
        {
            TSTLStrSize newpos = m_CursorPos + n;
            if (((newpos+m_ScrollOffset) > len) || (newpos < m_CursorPos)) // Don't go past str length and prevent overflows
                newpos = (len - m_ScrollOffset);
            
            m_CursorPos = newpos;
        }
    }
    else
    {
        m_ScrollOffset = 0;
        m_CursorPos = n;
    }

    if (m_CursorPos > static_cast<TSTLStrSize>(w))
    {
        m_ScrollOffset += (m_CursorPos - w);
        
        TSTLStrSize max = len - w;
        if (m_ScrollOffset > max)
            m_ScrollOffset = max;
                
        m_CursorPos = w;
    }
    
    refresh();
}

void CInputField::ChangeFMText()
{
    std::string text;
    
    if (m_chOutChar)
        text.append(m_szText.length(), SafeConvert<char>(m_chOutChar));
    else
        text = "<notg>" + m_szText;
    
    //text.append(width(), '_');
    
    m_FMText.SetText(text);
}

void CInputField::Focus()
{
    CWidgetWindow::Focus();
    curs_set(1);
    SetCursorPos(begy(), begx()+m_CursorPos);
}

bool CInputField::HandleKey(chtype ch)
{
    if (CWidgetWindow::HandleKey(ch))
        return true;
    
    bool handled = true;
    
    switch (ch)
    {
        case KEY_LEFT:
            MoveCursor(-1);
            break;
        case KEY_RIGHT:
            MoveCursor(1);
            break;
        case KEY_HOME:
            m_CursorPos = m_ScrollOffset = 0;
            refresh();
            break;
        case KEY_END:
            MoveCursor(m_szText.length(), false);
            break;
        case KEY_DC:
            Delch(false);
            PushEvent(EVENT_DATACHANGED);
            break;
        case KEY_BACKSPACE:
        case 0x07f: // Some terminals may give this instead of KEY_BACKSPACE
            Delch(true);
            PushEvent(EVENT_DATACHANGED);
            break;
        case KEY_ENTER:
        case '\n':
        case '\r':
            PushEvent(EVENT_CALLBACK);
            break;
        default:
            if (!isprint(ch))
                handled = false;
            else
            {
                if ((m_eInpType == INPUT_INT) || (m_eInpType == INPUT_FLOAT))
                {
                    std::string legal = "1234567890";

                    lconv *lc = localeconv();
                    assert(lc != NULL);
                    
                    std::string decpoint = std::string(lc->decimal_point) + std::string(lc->mon_decimal_point) + std::string(".");
                    std::string plusminsign = std::string(lc->positive_sign) + std::string(lc->negative_sign) + std::string("-+");
                    
                    if ((m_eInpType == INPUT_FLOAT) && (m_szText.find_first_of(decpoint) == std::string::npos))
                        legal += decpoint;
                    
                    if (((m_CursorPos+m_ScrollOffset) == 0) && (m_szText.find_first_of(plusminsign) == std::string::npos))
                        legal += plusminsign;
                    
                    if (legal.find(ch) == std::string::npos)
                        break; // Illegal char
                }
                
                Addch(ch);
                PushEvent(EVENT_DATACHANGED);
            }
            break;
    }
    
    return handled;
}

void CInputField::Draw()
{
    erase();
    m_FMText.Print(0, m_ScrollOffset, 1, width());
    
    if (Focused())
        SetCursorPos(begy(), begx() + m_CursorPos);
}

// -------------------------------------
// Progressbar class
// -------------------------------------

chtype CProgressbar::m_cDefaultFillColors;
chtype CProgressbar::m_cDefaultEmptyColors;

CProgressbar::CProgressbar(CWidgetWindow *owner, int nlines, int ncols, int begin_y, int begin_x,
                           float min, float max, char absrel) : CWidgetWindow(owner, nlines, ncols, begin_y,
                                                                              begin_x, absrel, true, false,
                                                                              m_cDefaultEmptyColors,
                                                                              m_cDefaultEmptyColors),
                                                                m_fMin(min), m_fMax(max), m_fCurrent(0.0f)
{
}

void CProgressbar::Draw()
{
    float maxw = SafeConvert<float>(width()) - 2.0f; // -2 for the borders
    float fac = fabs(m_fCurrent / (m_fMax-m_fMin));
    float w = (maxw * fac);
    
    move(1, 1);
    clrtoeol();
    
    attron(m_cDefaultFillColors);
    
    for (int i=0; i<SafeConvert<int>(w); i++)
        addch(1, i+1, ' ');
    
    attroff(m_cDefaultFillColors);
}

// -------------------------------------
// Checkbox class
// -------------------------------------

chtype CCheckbox::m_cDefaultFocusedColors;
chtype CCheckbox::m_cDefaultDefocusedColors;

CCheckbox::CCheckbox(CWidgetWindow *owner, int nlines, int ncols, int begin_y, int begin_x,
                     char absrel) : CWidgetWindow(owner, nlines, ncols, begin_y, begin_x, absrel,
                     false, true, m_cDefaultFocusedColors, m_cDefaultDefocusedColors),
                     m_SelectedButton(1)
{
}

bool CCheckbox::HandleKey(chtype ch)
{
    if (CWidgetWindow::HandleKey(ch))
        return true;
    
    bool handled = true;
    
    switch (ch)
    {
        case KEY_UP:
            m_SelectedButton--;
            if (m_SelectedButton < 1)
                m_SelectedButton = m_BoxList.size();
            break;
        case KEY_DOWN:
            m_SelectedButton++;
            if (m_SelectedButton > m_BoxList.size())
                m_SelectedButton = 1;
            break;
        case KEY_ENTER:
        case '\n':
        case '\r':
        {
            PushEvent(EVENT_CALLBACK);
            break;
        }
        case ' ':
            if (IsEnabled(m_SelectedButton))
                DisableBox(m_SelectedButton);
            else
                EnableBox(m_SelectedButton);
            PushEvent(EVENT_DATACHANGED);
            break;
        default:
            handled = false;
            break;
    }
    
    if (handled)
        refresh();
    
    return handled;
}

void CCheckbox::Draw()
{
    TSTLVecSize counter = 1;
    for (std::vector<SEntry>::iterator it=m_BoxList.begin(); it!=m_BoxList.end(); it++)
    {
        std::string out;
        if (IsEnabled(counter))
            out = "[X] ";
        else
            out = "[ ] ";
        
        out += it->name;
        
        if (counter == m_SelectedButton)
            attron(A_REVERSE);
        
        addstr(counter-1, 0, out.c_str());
        
        if (counter == m_SelectedButton)
            attroff(A_REVERSE);

        counter++;
    }
}

// -------------------------------------
// Radio button class
// -------------------------------------

chtype CRadioButton::m_cDefaultFocusedColors;
chtype CRadioButton::m_cDefaultDefocusedColors;

CRadioButton::CRadioButton(CWidgetWindow *owner, int nlines, int ncols, int begin_y, int begin_x,
                           char absrel) : CWidgetWindow(owner, nlines, ncols, begin_y, begin_x, absrel,
                           false, true, m_cDefaultFocusedColors, m_cDefaultDefocusedColors),
                           m_CheckedButton(1), m_SelectedButton(1)
{
}

bool CRadioButton::HandleKey(chtype ch)
{
    if (CWidgetWindow::HandleKey(ch))
        return true;
    
    bool handled = true;
    
    switch (ch)
    {
        case KEY_UP:
            m_SelectedButton--;
            if (m_SelectedButton < 1)
                m_SelectedButton = m_ButtonList.size();
            break;
        case KEY_DOWN:
            m_SelectedButton++;
            if (m_SelectedButton > m_ButtonList.size())
                m_SelectedButton = 1;
            break;
        case KEY_ENTER:
        case '\n':
        case '\r':
        {
            PushEvent(EVENT_CALLBACK);
            break;
        }
        case ' ':
            EnableButton(m_SelectedButton);
            PushEvent(EVENT_DATACHANGED);
            break;
        default:
            handled = false;
            break;
    }
    
    if (handled)
        refresh();
    
    return handled;
}

void CRadioButton::Draw()
{
    TSTLVecSize counter = 1;
    for (std::vector<std::string>::iterator it=m_ButtonList.begin(); it!=m_ButtonList.end(); it++)
    {
        std::string out;
        if (counter == EnabledButton())
            out = "(+) ";
        else
            out = "( ) ";
        
        out += *it;
        
        if (counter == m_SelectedButton)
            attron(A_REVERSE);
        
        addstr(counter-1, 0, out.c_str());
        
        if (counter == m_SelectedButton)
            attroff(A_REVERSE);
        
        counter++;
    }
}

// -------------------------------------
// Button bar class
// -------------------------------------

chtype CButtonBar::m_cDefaultColors;

CButtonBar::CButtonBar(CWidgetManager *owner, int nlines, int ncols, int begin_y,
                       int begin_x) : CWidgetWindow(owner, nlines, ncols, begin_y, begin_x, false, false,
                                                    m_cDefaultColors, m_cDefaultColors), m_pWidgetManager(owner), m_bDirty(false)
{
}

void CButtonBar::CreateInit()
{
    CWidgetWindow::CreateInit();
    m_pButtonText = AddChild(new CTextLabel(this, height(), width(), 0, 0, 'r'));
    m_pButtonText->SetColors(m_cDefaultColors, m_cDefaultColors);
}

void CButtonBar::Draw()
{
    erase();
    if (m_bDirty)
    {
        m_bDirty = false;

        if (height() != 2)
            resize(2, width()); // Resize to max height
        
        Clear();
        
        if (!m_ButtonTexts.empty())
        {
            for (TButtonList::iterator it=m_ButtonTexts.back().begin(); it!=m_ButtonTexts.back().end(); it++)
                m_pButtonText->AddText(CreateText("%s: <col=7:1>%s</col> ", GetTranslation(it->button), GetTranslation(it->desc)));
        }
        
//         if (height() != m_pButtonText->height())
        {
            resize(m_pButtonText->height(), width());
            mvwin(RawMaxY()-m_pButtonText->height(), 0);
            
            ::erase();
            m_pWidgetManager->Refresh();
        }
    }
}

void CButtonBar::Push()
{
    debugline("Cur BBar size: %u\n", m_ButtonTexts.size());
    m_ButtonTexts.push_back(TButtonList());
    
    // Check for global buttons
    for (std::list<TButtonList>::iterator it=m_ButtonTexts.begin(); it!=m_ButtonTexts.end(); it++)
    {
        for (TButtonList::iterator it2=it->begin(); it2!=it->end(); it2++)
        {
            if (it2->global)
                AddButton(it2->button, it2->desc);
        }
    }
    
    debugline("Push:");
    for (TButtonList::iterator it=m_ButtonTexts.back().begin(); it!=m_ButtonTexts.back().end(); it++)
        debugline("%s: %s ", it->button, it->desc);
    debugline("\n");
}

void CButtonBar::AddButton(const char *button, const char *desc)
{
    if (m_ButtonTexts.empty())
        Push();
    
    // Check if button is already there
    for (TButtonList::iterator it=m_ButtonTexts.back().begin(); it!=m_ButtonTexts.back().end(); it++)
    {
        if (!strcmp(it->button, button))
        {
            // Overwrite
            it->desc = desc;
            it->global = false;
            return;
        }
    }
    
    debugline("Button: %s: %s\n", button, desc);
    
    m_ButtonTexts.back().push_back(SButtonBarEntry(button, desc, false));
    m_bDirty = true;
}

void CButtonBar::AddGlobalButton(const char *button, const char *desc)
{
    if (m_ButtonTexts.empty())
        Push();
    
    // Check if button is already there
    for (TButtonList::iterator it=m_ButtonTexts.back().begin(); it!=m_ButtonTexts.back().end(); it++)
    {
        if (!strcmp(it->button, button))
        {
            // Overwrite
            it->desc = desc;
            it->global = true;
            return;
        }
    }
    
    debugline("Button(global): %s: %s\n", button, desc);
    
    m_ButtonTexts.back().push_back(SButtonBarEntry(button, desc, true));
    m_bDirty = true;
}

void CButtonBar::Pop()
{
    if (!m_ButtonTexts.empty())
    {
        debugline("Cur BBar size: %u\n", m_ButtonTexts.size());
        debugline("Pop:");
        for (TButtonList::iterator it=m_ButtonTexts.back().begin(); it!=m_ButtonTexts.back().end(); it++)
            debugline("%s: %s ", it->button, it->desc);
        debugline("\n");
        m_ButtonTexts.pop_back();
    }
    
    m_bDirty = true;
}

// -------------------------------------
// Widget Box base class
// -------------------------------------

CWidgetBox::CWidgetBox(CWidgetManager *owner, int maxlines, int ncols, int begin_y, int begin_x,
                       const char *info, chtype fcolor,
                       chtype dfcolor) : CWidgetWindow(owner, maxlines, ncols, begin_y, begin_x, true, true,
                                                       fcolor, dfcolor), m_bFinished(false),
                                         m_bCanceled(false), m_pWidgetManager(owner)
{
    if (info)
        m_szInfo = info;
}

CWidgetBox::CWidgetBox(CWidgetManager *owner, int maxlines, int ncols, int begin_y, int begin_x,
                       const char *info) : CWidgetWindow(owner, maxlines, ncols, begin_y, begin_x, true),
                                           m_bFinished(false), m_bCanceled(false), m_pWidgetManager(owner)
{
    if (info)
        m_szInfo = info;
}

void CWidgetBox::CreateInit()
{
    CWidgetWindow::CreateInit();
    
    if (!m_szInfo.empty())
    {
        m_pLabel = AddChild(new CTextLabel(this, height()-4, width()-4, 2, 2, 'r'));
        m_pLabel->AddText(m_szInfo);
    }
    else
        m_pLabel = NULL;
}

bool CWidgetBox::HandleKey(chtype ch)
{
    if (CWidgetWindow::HandleKey(ch))
        return true;
    
    if (ch == CTRL('[')) // Escape
    {
        m_bFinished = m_bCanceled = true;
        return true;
    }
    
    return false;
}

void CWidgetBox::Fit(int nlines)
{
    resize(nlines, maxx());
    mvwin(((MaxY() - maxy())/2), ((MaxX() - maxx())/2));
    
    // Refresh main screen, this is required after a window has been resized or moved
    ::erase();
    m_pWidgetManager->Refresh();
}

// -------------------------------------
// Message Box class
// -------------------------------------

chtype CMessageBox::m_cDefaultFocusedColors;
chtype CMessageBox::m_cDefaultDefocusedColors;

CMessageBox::CMessageBox(CWidgetManager *owner, int maxlines, int ncols, int begin_y, int begin_x,
                         const char *text) : CWidgetBox(owner, maxlines, ncols, begin_y, begin_x,
                                                        text, m_cDefaultFocusedColors,
                                                        m_cDefaultDefocusedColors)
{
}

void CMessageBox::CreateInit()
{
    CWidgetBox::CreateInit();
    m_pOKButton = AddChild(new CButton(this, 1, 10, (m_pLabel->rely()+m_pLabel->maxy()+2), (width()-10)/2, GetTranslation("OK"), 'r'));
    Fit(m_pOKButton->rely()+m_pOKButton->maxy()+2);
}

bool CMessageBox::HandleEvent(CWidgetHandler *p, int type)
{
    if (p == m_pOKButton)
    {
        m_bFinished = true;
        return true;
    }
    
    return false;
}

void CMessageBox::Run()
{
    m_bFinished = m_bCanceled = false;
    
    while (m_pWidgetManager->Run() && !m_bFinished) 
        ;
}

// -------------------------------------
// Warning Box class
// -------------------------------------

chtype CWarningBox::m_cDefaultFocusedColors;
chtype CWarningBox::m_cDefaultDefocusedColors;

CWarningBox::CWarningBox(CWidgetManager *owner, int maxlines, int ncols, int begin_y, int begin_x,
                         const char *text) : CMessageBox(owner, maxlines, ncols, begin_y, begin_x, text)
{
}

void CWarningBox::CreateInit()
{
    CMessageBox::CreateInit();
    
    SetTitle(GetTranslation("Warning"));
    SetColors(m_cDefaultFocusedColors, m_cDefaultDefocusedColors);
    m_pLabel->SetColors(m_cDefaultFocusedColors, m_cDefaultDefocusedColors);
    m_pOKButton->SetColors(m_cDefaultFocusedColors, m_cDefaultDefocusedColors);
    refresh();
}

// -------------------------------------
// Yes-No Box class
// -------------------------------------

chtype CYesNoBox::m_cDefaultFocusedColors;
chtype CYesNoBox::m_cDefaultDefocusedColors;

CYesNoBox::CYesNoBox(CWidgetManager *owner, int maxlines, int ncols, int begin_y, int begin_x,
                     const char *text) : CWidgetBox(owner, maxlines, ncols, begin_y, begin_x,
                                                    text, m_cDefaultFocusedColors,
                                                    m_cDefaultDefocusedColors)
{
}

void CYesNoBox::CreateInit()
{
    CWidgetBox::CreateInit();
    
    int x = (maxx()-((2*10)+2))/2; // Center 2 buttons of 10 length and one 2 sized space
    int y = (m_pLabel->rely()+m_pLabel->maxy()+2);
    m_pNoButton = AddChild(new CButton(this, 1, 10, y, x, GetTranslation("No"), 'r'));
    m_pYesButton = AddChild(new CButton(this, 1, 10, y, (x+m_pNoButton->maxx()+2), GetTranslation("Yes"), 'r'));
    
    Fit(y+m_pNoButton->maxy()+2);
}

bool CYesNoBox::HandleEvent(CWidgetHandler *p, int type)
{
    if (p == m_pYesButton)
    {
        m_bFinished = true;
        return true;
    }
    else if (p == m_pNoButton)
    {
        m_bFinished = m_bCanceled = true;
        return true;
    }
    
    return false;
}

bool CYesNoBox::Run()
{
    m_bFinished = m_bCanceled = false;
    
    while (m_pWidgetManager->Run() && !m_bFinished) 
        ;
    
    return !m_bCanceled;
}

// -------------------------------------
// Choice Box class
// -------------------------------------

chtype CChoiceBox::m_cDefaultFocusedColors;
chtype CChoiceBox::m_cDefaultDefocusedColors;

CChoiceBox::CChoiceBox(CWidgetManager *owner, int maxlines, int ncols, int begin_y, int begin_x,
                       const char *text, const char *but1, const char *but2,
                       const char *but3) : CWidgetBox(owner, maxlines, ncols, begin_y, begin_x, text,
                                                      m_cDefaultFocusedColors, m_cDefaultDefocusedColors)
{
    m_ButtonTitles[0] = but1;
    m_ButtonTitles[1] = but2;
    m_ButtonTitles[2] = (but3) ? but3 : "";
}

void CChoiceBox::CreateInit()
{
    CWidgetBox::CreateInit();
    
    // Button 3 is optional
//     int bcount = (but3) ? 3 : 2;
    int bcount = (!m_ButtonTitles[2].empty()) ? 3 : 2;
    
    // Find out longest text
    int w = std::max(std::max(m_ButtonTitles[0].length(), m_ButtonTitles[1].length()), m_ButtonTitles[2].length());
    
    w += 4; // Buttons use 4 additional tokens for focusing
    
    int x = (maxx()-((bcount*w)+bcount-1))/2; // Center bcount buttons, with bcount-1 2 sized spaces.
    int y = (m_pLabel->rely()+m_pLabel->maxy()+2);

    m_pButtons[0] = AddChild(new CButton(this, 1, w, y, x, m_ButtonTitles[0], 'r'));
    
    x += (w + 2);
    m_pButtons[1] = AddChild(new CButton(this, 1, w, y, x, m_ButtonTitles[1], 'r'));
    
    if (!m_ButtonTitles[2].empty())
    {
        x += (w + 2);
        m_pButtons[2] = AddChild(new CButton(this, 1, w, y, x, m_ButtonTitles[2], 'r'));
    }
    else
        m_pButtons[2] = NULL;
    
    Fit(y+m_pButtons[0]->maxy()+2);
}

bool CChoiceBox::HandleEvent(CWidgetHandler *p, int type)
{
    if (p == m_pButtons[0])
    {
        m_iSelectedButton = 0;
        m_bFinished = true;
    }
    else if (p == m_pButtons[1])
    {
        m_iSelectedButton = 1;
        m_bFinished = true;
    }
    else if (p == m_pButtons[2])
    {
        m_iSelectedButton = 2;
        m_bFinished = true;
    }

    return false;
}

int CChoiceBox::Run()
{
    m_bFinished = m_bCanceled = false;
    
    while (m_pWidgetManager->Run() && !m_bFinished)
        ;
    
    return (m_bCanceled) ? -1 : m_iSelectedButton;
}

// -------------------------------------
// Input dialog class
// -------------------------------------

chtype CInputDialog::m_cDefaultFocusedColors;
chtype CInputDialog::m_cDefaultDefocusedColors;

CInputDialog::CInputDialog(CWidgetManager *owner, int maxlines, int ncols, int begin_y, int begin_x,
                           const char *info, int max,
                           bool sec) : CWidgetBox(owner, maxlines, ncols, begin_y, begin_x, info,
                                                  m_cDefaultFocusedColors, m_cDefaultDefocusedColors),
                                       m_bSecure(sec), m_iMax(max)
{
}

void CInputDialog::CreateInit()
{
    CWidgetBox::CreateInit();
    
    const char *OKTXT = GetTranslation("OK");
    const char *CancelTXT = GetTranslation("Cancel");
    
    char out = (m_bSecure) ? '*' : 0;
    int y = (m_pLabel->rely()+m_pLabel->maxy()+2);
    m_pTextField = AddChild(new CInputField(this, 1, width()-4, y, 2, 'r', m_iMax, out));
    
    size_t buttonw = std::max(strlen(OKTXT), strlen(CancelTXT)) + 4;
    int x = (width() - (2 * buttonw + 2)) / 2;
    y += (m_pTextField->maxy() + 2);
    m_pOKButton = AddChild(new CButton(this, 1, buttonw, y, x, OKTXT, 'r'));
    
    m_pCancelButton = AddChild(new CButton(this, 1, buttonw, y, x+buttonw+2, CancelTXT, 'r'));
    
    ActivateChild(m_pTextField);
    
    Fit(m_pOKButton->rely()+m_pOKButton->maxy()+2);
}

bool CInputDialog::HandleEvent(CWidgetHandler *p, int type)
{
    if (p == m_pTextField)
    {
        if (type == EVENT_CALLBACK)
        {
            m_bFinished = true;
            m_bCanceled = false;
            return true;
        }
    }
    else if (p == m_pOKButton)
    {
        m_bFinished = true;
        m_bCanceled = false;
        return true;
    }
    else if (p == m_pCancelButton)
    {
        m_bFinished = m_bCanceled = true;
        return true;
    }
    
    return false;
}

const std::string &CInputDialog::Run()
{
    m_bFinished = m_bCanceled = false;
    
    while (m_pWidgetManager->Run() && !m_bFinished)
        ;
    
    if (m_bCanceled)
        m_pTextField->SetText("");
    
    return m_pTextField->GetText();
}

// -------------------------------------
// File dialog class
// -------------------------------------

chtype CFileDialog::m_cDefaultFocusedColors;
chtype CFileDialog::m_cDefaultDefocusedColors;

CFileDialog::CFileDialog(CWidgetManager *owner, int maxlines, int ncols, int begin_y, int begin_x, const char *s,
                         const char *info) : CWidgetBox(owner, maxlines, ncols, begin_y, begin_x, info,
                                                        m_cDefaultFocusedColors, m_cDefaultDefocusedColors), m_szInfo(info)
{
    m_szStartDir = m_szSelectedDir = GetFirstValidDir(s);
}

void CFileDialog::CreateInit()
{
    CWidgetBox::CreateInit();
    
    m_pFileMenu = AddChild(new CMenu(this, height()-10, width()-4, (m_pLabel->rely()+m_pLabel->maxy()+2), 2, 'r'));
    
    m_pFileField = AddChild(new CInputField(this, 1, width()-4, (m_pFileMenu->rely()+m_pFileMenu->maxy()+2), 2, 'r'));
    m_pFileField->SetText(m_szStartDir);
    
    const char *OpenTXT = GetTranslation("Open directory");
    const char *CancelTXT = GetTranslation("Cancel");
    size_t buttonw = std::max(strlen(OpenTXT), strlen(CancelTXT)) + 4;
    const int startx = (width() - (2 * buttonw + 2)) / 2;
    const int starty = (m_pFileField->rely()+m_pFileField->maxy()+2);
    
    m_pOpenButton = AddChild(new CButton(this, 1, buttonw, starty, startx, OpenTXT, 'r'));
    m_pCancelButton = AddChild(new CButton(this, 1, buttonw, starty, startx+buttonw+2, CancelTXT, 'r'));
    
    ActivateChild(m_pFileMenu);
    
    Fit(m_pOpenButton->rely()+m_pOpenButton->maxy()+2);
    
    OpenDir();
    
    AddGlobalButton("F2", "Create new directory");
}

void CFileDialog::OpenDir(std::string newdir)
{
    if (newdir.empty())
    {
        newdir = m_szSelectedDir;

        if (!m_pFileMenu->Empty())
            newdir += '/' + m_pFileMenu->GetCurrentItemName();
    }
    
    try
    {
        CHDir(newdir);
        m_szSelectedDir = GetCWD();
        
        CDirIter dir(m_szSelectedDir);
        struct stat filestat;
        bool isrootdir = (m_szSelectedDir == "/");
    
        m_pFileMenu->Clear();
        
        while (dir)
        {
            // Valid directory?
            if ((lstat(dir->d_name, &filestat) == 0) && S_ISDIR(filestat.st_mode) && (!isrootdir || strcmp(dir->d_name, "..")))
            {
                m_pFileMenu->AddItem(dir->d_name);
            }
            
            dir++;
        }
        
        m_pFileMenu->refresh();
        
        // Update AFTER file menu, since it may sort items in Draw()
        UpdateDirField();
    }
    catch(Exceptions::CExIO &e)
    {
        // Couldn't open directory(probably no read access)
        WarningBox(e.what());
        
        // If no directory is open yet, just open /
        if (m_pFileMenu->Empty())
        {
            if (newdir == "/")
                throw; // Couldn't even open / ...
            
            OpenDir("/");
        }
    }
}

bool CFileDialog::HandleEvent(CWidgetHandler *p, int type)
{
    if (p == m_pFileMenu)
    {
        if (type == EVENT_CALLBACK)
        {
            OpenDir();
            return true;
        }
    }
    else if (p == m_pFileField)
    {
        if (type == EVENT_CALLBACK)
        {
            OpenDir(m_pFileField->GetText());
            return true;
        }
    }
    else if (p == m_pOpenButton)
    {
        m_bFinished = true;
        return true;
    }
    else if (p == m_pCancelButton)
    {
        m_szSelectedDir.clear();
        m_bFinished = m_bCanceled = true;
        return true;
    }
    
    return false;
}

bool CFileDialog::HandleKey(chtype ch)
{
    if (CWidgetBox::HandleKey(ch))
        return true;
    
    if (ch == KEY_F(2))
    {
        std::string newdir = InputDialog(GetTranslation("Enter name of new directory"), NULL, 1024);
        
        if (newdir.empty())
            return true;
        
        try
        {
            MKDir(newdir, (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH));
            OpenDir(newdir);
        }
        catch(Exceptions::CExMKDir &e)
        {
            WarningBox("%s\n%s\n%s", GetTranslation("Could not create directory"), newdir.c_str(), e.what());
        }
            
        return true;
    }
    
    return false;
}

const std::string &CFileDialog::Run()
{
    m_bFinished = m_bCanceled = false;
    
    while (m_pWidgetManager->Run() && !m_bFinished)
        ;
    
    if (m_bCanceled)
        m_szSelectedDir = "";
    
    return m_szSelectedDir;
}

// -------------------------------------
// Menu dialog class
// -------------------------------------

chtype CMenuDialog::m_cDefaultFocusedColors;
chtype CMenuDialog::m_cDefaultDefocusedColors;
std::string CMenuDialog::m_szBlank;

CMenuDialog::CMenuDialog(CWidgetManager *owner, int maxlines, int ncols, int begin_y, int begin_x,
                         const char *info) : CWidgetBox(owner, maxlines, ncols, begin_y, begin_x, info,
                                             m_cDefaultFocusedColors, m_cDefaultDefocusedColors)
{
}

void CMenuDialog::CreateInit()
{
    CWidgetBox::CreateInit();
    
    const char *OKTXT = GetTranslation("OK"), *CancelTXT = GetTranslation("Cancel");
    int y = (m_pLabel->rely()+m_pLabel->maxy()+2);
    const int menuh = height() - y - 4;
    m_pMenu = AddChild(new CMenu(this, menuh, width()-4, y, 2, 'r'));
    
    size_t buttonw = std::max(strlen(OKTXT), strlen(CancelTXT)) + 4;
    int x = (width() - (2 * buttonw + 2)) / 2;
    y += (menuh + 1);
    
    m_pOKButton = AddChild(new CButton(this, 1, buttonw, y, x, OKTXT, 'r'));
    m_pCancelButton = AddChild(new CButton(this, 1, buttonw, y, x+buttonw+2, CancelTXT, 'r'));
    
    ActivateChild(m_pMenu);
    
    Fit(m_pOKButton->rely()+m_pOKButton->maxy()+2);
    
    AddGlobalButton("ESC", "Cancel");
}

bool CMenuDialog::HandleEvent(CWidgetHandler *p, int type)
{
    if (p == m_pMenu)
    {
        if (type == EVENT_CALLBACK)
        {
            m_bFinished = true;
            m_bCanceled = false;
            m_szSelection = m_pMenu->GetCurrentItemName();
            return true;
        }
    }
    else if (p == m_pOKButton)
    {
        m_bFinished = true;
        m_bCanceled = false;
        m_szSelection = m_pMenu->GetCurrentItemName();
        return true;
    }
    else if (p == m_pCancelButton)
    {
        m_bFinished = m_bCanceled = true;
        m_szSelection = "";
        return true;
    }
    
    return false;
}

void CMenuDialog::Run()
{
    m_bFinished = m_bCanceled = false;
    
    while (m_pWidgetManager->Run() && !m_bFinished)
        ;
}
