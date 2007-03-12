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
#include "box.h"

namespace NNCurses {

// -------------------------------------
// Widget Box Class
// -------------------------------------

int CBox::CoreRequestWidth()
{
    TChildList childs = GetChildList();
    int ret = 0;
    
    for (TChildList::iterator it=childs.begin(); it!=childs.end(); it++)
    {
        const SBoxEntry &entry = m_BoxEntries[*it];
        
        if (m_eDirection == HORIZONTAL)
            ret += (*it)->RequestWidth() + (2 * entry.padding) + m_iSpacing;
        else
            ret = std::max(ret, (*it)->RequestWidth());
    }
    
    if (HasBox())
        ret += 2;
    
    return std::max(ret, GetMinWidth());
}

int CBox::CoreRequestHeight()
{
    TChildList childs = GetChildList();
    int ret = 0;
    
    for (TChildList::iterator it=childs.begin(); it!=childs.end(); it++)
    {
        const SBoxEntry &entry = m_BoxEntries[*it];
        
        if (m_eDirection == VERTICAL)
            ret += (*it)->RequestHeight() + (2 * entry.padding) + m_iSpacing;
        else
            ret = std::max(ret, (*it)->RequestHeight());
    }
    
    if (HasBox())
        ret += 2;

    return std::max(ret, GetMinHeight());
}

void CBox::UpdateLayout()
{
    if (Empty())
        return;
    
    TChildList childs = GetChildList();
    const TSTLVecSize size = childs.size();
    int extraw = (Width() - RequestWidth()) / size;
    int extrah = (Height() - RequestHeight()) / size;
    int begx = FieldX(), begy = FieldY(); // Coords for widgets that were packed at start
    int endx = (FieldX()+FieldWidth())-1, endy = (FieldY()+FieldHeight())-1; // Coords for widgets that were packed at end
    
    for (TChildList::iterator it=childs.begin(); it!=childs.end(); it++)
    {
        int basex = 0, basey = 0;
        int widgetw = 0, widgeth = 0;
        const SBoxEntry &entry = m_BoxEntries[*it];
        int spacing = m_iSpacing + entry.padding;
        
        if (m_eDirection == HORIZONTAL)
        {
            basex += entry.padding;
            widgetw += (*it)->RequestWidth();
            widgeth += FieldHeight();
            
            if (entry.expand)
            {
                if (entry.fill)
                    widgetw += extraw;
                else
                {
                    spacing += (extraw/2);
                    basex += (extraw/2);
                }
            }
        }
        else
        {
            basey += entry.padding;
            widgetw += FieldWidth();
            widgeth += (*it)->RequestHeight();
            
            if (entry.expand)
            {
                if (entry.fill)
                    widgeth += extrah;
                else
                {
                    spacing += (extrah/2);
                    basey += (extrah/2);
                }
            }
        }
        
        if (entry.start)
        {
            begx += basex;
            begy += basey;
            
            (*it)->SetSize(begx, begy, widgetw, widgeth);
            
            if (m_eDirection == HORIZONTAL)
                begx += (spacing + widgetw);
            else
                begy += (spacing + widgeth);
        }
        else
        {
            endx -= basex;
            endy -= basey;
            
            (*it)->SetSize(endx-(widgetw-1), endy-(widgeth-1), widgetw, widgeth);
            
            if (m_eDirection == HORIZONTAL)
                endx -= (spacing + widgetw);
            else
                endy -= (spacing + widgeth);
        }
    }
}

void CBox::CoreDraw()
{
    if (m_bUpdateLayout)
    {
        UpdateLayout();
        m_bUpdateLayout = false;
    }
    
    CGroup::CoreDraw();
}

void CBox::StartPack(CGroup *g, bool e, bool f, int p)
{
    m_BoxEntries[g] = SBoxEntry(e, f, p, true);
    CGroup::AddWidget(g);
}

void CBox::StartPack(CWidget *w, bool e, bool f, int p)
{
    m_BoxEntries[w] = SBoxEntry(e, f, p, true);
    CGroup::AddWidget(w);
}

void CBox::EndPack(CGroup *g, bool e, bool f, int p)
{
    m_BoxEntries[g] = SBoxEntry(e, f, p, false);
    CGroup::AddWidget(g);
}

void CBox::EndPack(CWidget *w, bool e, bool f, int p)
{
    m_BoxEntries[w] = SBoxEntry(e, f, p, false);
    CGroup::AddWidget(w);
}

}
