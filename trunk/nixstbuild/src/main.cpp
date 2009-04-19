/***************************************************************************
 *   Copyright (C) 2009 by Rick Helmus / InternetNightmare   *
 *   rhelmus_AT_gmail.com / internetnightmare_AT_gmail.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


#include <QApplication>
#include <QMessageBox>

#include "main/exception.h"
#include "main/lua/lua.h"
#include "luaparser.h"
#include "main.h"
#include "welcome.h"
#include "configw.h"

int main(int argc, char *argv[])
{
    CNixstbApp app(argc, argv);

    QCoreApplication::setOrganizationName("Nixstaller");
    QCoreApplication::setOrganizationDomain("nixstaller.berlios.de");
    QCoreApplication::setApplicationName("Nixstbuild");
    
    (new CWelcomeScreen())->show();

    try
    {
        CLuaParser luaParser;
        luaParser.Init(argc, argv);

        app.exec();

        NLua::StackDump("Clean stack?\n");
    }
    catch(Exceptions::CException &e)
    {
        QMessageBox::critical(0, "Error", e.what());
        return 1;
    }
    
     return 0;
}

bool CNixstbApp::notify(QObject *receiver, QEvent *event)
{
    try
    {
        return QApplication::notify(receiver, event);
    }
    catch(Exceptions::CException &e)
    {
        QMessageBox::critical(0, "Error", e.what());
        QApplication::exit(1);
        return false;
    }
}
