/*
    Copyright (C) 2007 Rick Helmus (rhelmus_AT_gmail.com)

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

#include <fstream>
#include "gtk.h"
#include "luatextfield.h"

// -------------------------------------
// Lua Text Field Class
// -------------------------------------

CLuaTextField::CLuaTextField(const char *desc, bool wrap) : CBaseLuaWidget(desc)
{
    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolled), GTK_SHADOW_ETCHED_IN);
    gtk_widget_set_size_request(scrolled, -1, 150);
    gtk_widget_show(scrolled);
    gtk_container_add(GTK_CONTAINER(GetBox()), scrolled);
    
    m_pTextField = gtk_text_view_new();
    
    if (wrap)
        gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(m_pTextField), GTK_WRAP_WORD_CHAR);
    
    // Set up mark for scrolling to bottom
    GtkTextBuffer *textbuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(m_pTextField));
    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(textbuffer, &iter);
    gtk_text_buffer_create_mark(textbuffer, "scroll", &iter, TRUE);
    
    gtk_widget_show(m_pTextField);
    gtk_container_add(GTK_CONTAINER(scrolled), m_pTextField);
}

void CLuaTextField::Load(const char *file)
{
    GtkTextBuffer *textbuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(m_pTextField));
    std::ifstream infile(file);
    std::string buf;
    char c;
    
    while (infile.get(c))
        buf += c;

    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(textbuffer, &iter);
    gtk_text_buffer_insert(textbuffer, &iter, buf.c_str(), -1);
    
    if (Follow())
        DoFollow(textbuffer, &iter);
}

void CLuaTextField::AddText(const char *text)
{
    GtkTextBuffer *textbuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(m_pTextField));
    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(textbuffer, &iter);
    gtk_text_buffer_insert(textbuffer, &iter, text, -1);
    
    if (Follow())
        DoFollow(textbuffer, &iter);
}

void CLuaTextField::UpdateFollow()
{
    if (Follow())
    {
        GtkTextBuffer *textbuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(m_pTextField));
        GtkTextIter iter;
        gtk_text_buffer_get_end_iter(textbuffer, &iter);
        DoFollow(textbuffer, &iter);
    }
}

void CLuaTextField::CoreActivateWidget()
{
    gtk_widget_grab_focus(m_pTextField);
}

void CLuaTextField::DoFollow(GtkTextBuffer *textbuffer, GtkTextIter *iter)
{
    GtkTextMark *mark = gtk_text_buffer_get_mark(textbuffer, "scroll");
    gtk_text_buffer_move_mark(textbuffer, mark, iter);
    gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(m_pTextField), mark);
}
