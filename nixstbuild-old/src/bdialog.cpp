/***************************************************************************
 *   Copyright (C) 2007 by InternetNightmare   *
 *   internetnightmare@gmail.com   *
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

#include <string>

using namespace std;

#include <stdio.h>

#include <QtGui>

#include <QDialog>
#include <QTextEdit>
#include <QMessageBox>

#include <QThread>

#include "bdialog.h"

void BThread::run()
{
    QSettings settings("INightmare", "Nixstbuild");
    char line[132];

    string cmd = "cd " + QFileInfo(settings.value("geninstall").toString()).absolutePath().toStdString() + " && ";
    cmd += settings.value("geninstall").toString().toStdString() + " " + settings.value("dir").toString().toStdString();

    pipe = popen(cmd.c_str(), "r");
    //pipe = popen("ls -al", "r");

    while (fgets(line, sizeof(line), pipe))
    {
        emit lineReceived(line);
    }

    pclose(pipe);
}

BDialog::BDialog(QWidget *parent): QWidget(parent)
{
    view = new QTextEdit(this);
    view->setReadOnly(true);
}

void BDialog::append(QString text)
{
    view->append(text);
}

