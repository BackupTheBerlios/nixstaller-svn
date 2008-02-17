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
#ifndef CVAROUTPUT_H
#define CVAROUTPUT_H

/**
	@author InternetNightmare <internetnightmare@gmail.com>
*/

#define VT_NULL 0
#define VT_STRING 1
#define VT_MULTILINE 2
#define VT_LIST 3
#define VT_BOOLEAN 4
#define VT_FUNC1 5
#define VT_FUNC2 6

class QWidget;

typedef struct var_s
{
    char *name;
    char *description;
    int type;
    char *defaultval;
    QWidget *control;
} var_t;

class CVarOutput
{
public:
    CVarOutput();
    ~CVarOutput();

    void init(var_t *data);
    QWidget *getLayout();
    QPushButton *getSaveButton();
    string getScript();

private:
    var_t *vdata;
    string script;
    QWidget *layout;
    QPushButton *save_btn;
};

#endif
