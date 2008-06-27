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
                                                                m_bDone(false), m_lTimer(0)
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
    Check(curl_easy_setopt(m_pCurl, CURLOPT_USERAGENT, "nixstaller"));
    Check(curl_easy_setopt(m_pCurl, CURLOPT_CONNECTTIMEOUT, 30));
//     Check(curl_easy_setopt(m_pCurl, CURLOPT_NOSIGNAL, 1));
    Check(curl_easy_setopt(m_pCurl, CURLOPT_LOW_SPEED_LIMIT, 0));
    Check(curl_easy_setopt(m_pCurl, CURLOPT_LOW_SPEED_TIME, 30));
        
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
    if (m_bDone)
        return false;
    
    if (m_lTimer > GetTime())
        return true;
    
    Perform();
    
    const long maxwait = 250; // msec
    long timeout = 0;
    
    CheckM(curl_multi_timeout(m_pCurlMulti, &timeout));
    timeout = std::min(timeout, maxwait);
    m_lTimer = GetTime() + timeout;
    
    CURLMsg *msg;
    int qleft;
    while ((msg = curl_multi_info_read(m_pCurlMulti, &qleft)) != NULL)
    {
        if (msg->msg == CURLMSG_DONE)
        {
            m_bDone = true;
            if (msg->data.result != CURLE_OK)
                m_Result = curl_easy_strerror(msg->data.result);
            else
            {
                long status;
                Check(curl_easy_getinfo(m_pCurl, CURLINFO_RESPONSE_CODE, &status));
                if ((status >= 400) && (status < 600)) // FTP/HTTP 4xx or 5xx code?
                    m_Result = "Error starting download"; // UNDONE?
            }
            break;
        }
    }
    
    return !m_bDone;
}
