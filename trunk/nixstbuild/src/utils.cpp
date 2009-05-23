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

#include <stdlib.h>

#include <QDir>
#include <QLineEdit>
#include <QMessageBox>
#include <QObject>
#include <QRegExpValidator>

#include "utils.h"

// Required by exceptions.h
const char *GetExTranslation(const char *s)
{
    return QObject::tr(s).toLatin1().data();
}

#ifndef RELEASE
void debugline(const char *t, ...)
{
    char *txt;
    va_list v;
    
    va_start(v, t);
    vasprintf(&txt, t, v);
    va_end(v);
    
    printf("DEBUG: %s", txt);
    
    free(txt);
}
#endif

QLineEdit *createLuaEdit(QWidget *parent)
{
    QLineEdit *ret = new QLineEdit(parent);
    ret->setValidator(new QRegExpValidator(QRegExp("[\\w]+"), 0));
    return ret;
}

bool verifyLuaEdit(QLineEdit *edit)
{
    bool ret = false;
    const QStringList reserverd = QStringList() << "and" << "end" << "in" <<
            "repeat" << "break" << "false" << "local" << "return" << "do" <<
            "for" << "nil" << "then" << "else" << "function" << "not" << "true" <<
            "elseif" << "if" << "or" << "until" << "while";
    
    if (edit->text().isEmpty())
        QMessageBox::critical(NULL, "Missing information", "Please specify a variable name.");
    else if (reserverd.contains(edit->text()))
        QMessageBox::critical(NULL, "Wrong input",
                                 "The specified variable name is a reserverd keyword. Please specify another name.");
    else
        ret = true;

    if (!ret)
    {
        edit->setFocus(Qt::OtherFocusReason);
        edit->selectAll();
    }
    
    return ret;
}

bool verifyNixstPath(const QString &path)
{
    QDir d(path);
    return (d.isReadable() && d.exists("geninstall.sh"));
}
