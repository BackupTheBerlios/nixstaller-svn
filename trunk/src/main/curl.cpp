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

#include <errno.h>

#include "curl.h"
#include "main/main.h"
#include "main/exception.h"

// -------------------------------------
// Curl Wrapper Class
// -------------------------------------

CCURLWrapper::CCURLWrapper(const char *url, const char *dest) : m_pCurl(NULL), m_pCurlMulti(NULL), m_pDestFile(NULL),
                                                                m_bDone(false), m_bInit(true), m_lTimer(0),
                                                                m_Result(CURLE_OK)
{
    m_pCurl = curl_easy_init();
    m_pCurlMulti = curl_multi_init();
    
    if (!m_pCurl || !m_pCurlMulti)
        throw Exceptions::CExCURL("Could not create curl handle.");

    m_pDestFile = fopen(dest, "w");
    if (!m_pDestFile)
        throw Exceptions::CExOpen(errno, dest);
    
    Check(curl_easy_setopt(m_pCurl, CURLOPT_URL, url));
    Check(curl_easy_setopt(m_pCurl, CURLOPT_WRITEDATA, m_pDestFile));
    Check(curl_easy_setopt(m_pCurl, CURLOPT_NOPROGRESS, false));
        
    CheckM(curl_multi_add_handle(m_pCurlMulti, m_pCurl));
}

void CCURLWrapper::Check(CURLcode code)
{
    if (code != CURLE_OK)
        throw Exceptions::CExCURL(curl_easy_strerror(code));
}

void CCURLWrapper::CheckM(CURLMcode code)
{
    if (code != CURLM_OK)
        throw Exceptions::CExCURL(curl_multi_strerror(code));
}

void CCURLWrapper::Perform()
{
    int running;
    CURLMcode code = curl_multi_perform(m_pCurlMulti, &running);
    
    while (code == CURLM_CALL_MULTI_PERFORM)
        code = curl_multi_perform(m_pCurlMulti, &running);
    
    CheckM(code);
}

void CCURLWrapper::SetProgFunc(curl_progress_callback progfunc, void *data)
{
    Check(curl_easy_setopt(m_pCurl, CURLOPT_PROGRESSFUNCTION, progfunc));
    Check(curl_easy_setopt(m_pCurl, CURLOPT_PROGRESSDATA, data));
}

void CCURLWrapper::Close()
{
    if (m_pCurl)
    {
        curl_easy_cleanup(m_pCurl);
        m_pCurl = NULL;
    }
    
    if (m_pCurlMulti)
    {
        curl_multi_cleanup(m_pCurlMulti);
        m_pCurlMulti = NULL;
    }
    
    if (m_pDestFile)
    {
        fclose(m_pDestFile);
        m_pDestFile = NULL;
    }
}

bool CCURLWrapper::Process()
{
    if (m_bDone || (m_lTimer > GetTime()))
        return !m_bDone;
    
    if (m_bInit)
    {
        m_bInit = false;
        Perform(); // Perform atleast once
    }
    
    const long maxwait = 100;
    long timeout;
    int maxfd = 0;
    fd_set sread, swrite, serror;
    
    FD_ZERO(&sread);
    FD_ZERO(&swrite);
    FD_ZERO(&serror);
    
    CheckM(curl_multi_fdset(m_pCurlMulti, &sread, &swrite, &serror, &maxfd));
    CheckM(curl_multi_timeout(m_pCurlMulti, &timeout));
    
    if (timeout == -1)
        timeout = maxwait;
    
    timeout = std::min(timeout, maxwait);

    m_lTimer = GetTime() + maxwait;
    
    if (maxfd == -1)
    {
        // UNDONE
        debugline("maxfd == -1\n");
        return true;
    }
    
    // Change this if maxwait ever happens to be >= 1 sec!
    timeval tval;
    tval.tv_sec = 0;
    tval.tv_usec = timeout;
    int ret = Select(maxfd+1, &sread, &swrite, &serror, &tval);
    
    if (ret)
        Perform();
    
    CURLMsg *msg;
    int qleft;
    while ((msg = curl_multi_info_read(m_pCurlMulti, &qleft)) != NULL)
    {
        if (msg->msg == CURLMSG_DONE)
        {
            debugline("Transfer done (%d)\n", msg->data.result);
            m_bDone = true;
            m_Result = msg->data.result;
            break;
        }
    }
    
    return !m_bDone;
}
