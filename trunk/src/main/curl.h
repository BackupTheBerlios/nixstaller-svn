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

#include <string>
#include "curl/curl.h"

class CCURLWrapper
{
    CURL *m_pCurl;
    CURLM *m_pCurlMulti;
    FILE *m_pDestFile;
    bool m_bDone, m_bInit;
    long m_lTimer;
    CURLcode m_Result;
    
    void Check(CURLcode code);
    void CheckM(CURLMcode code);
    void Perform(void);
    
public:
    CCURLWrapper(const char *url, const char *dest);
    ~CCURLWrapper(void) { Close(); }
    
    void SetProgFunc(curl_progress_callback progfunc, void *data);
    void Close(void);
    bool Process(void);
    bool Success(void) { return m_Result == CURLE_OK; }
    const char *ErrorMessage(void) { return curl_easy_strerror(m_Result); }
};
