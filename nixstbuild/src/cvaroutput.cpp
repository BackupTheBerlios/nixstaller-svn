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
#include <sstream>

using namespace std;

#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QCheckBox>
#include <QStringList>
#include <QPushButton>
#include <QGridLayout>
#include <QScrollArea>

#include "nlist.h"
#include "n2list.h"
#include "cvaroutput.h"


CVarOutput::CVarOutput()
{
}


CVarOutput::~CVarOutput()
{
}

void CVarOutput::init(var_t *data)
{
    var_t *iter = vdata = data;

    layout = new QWidget;
    QGridLayout *grid = new QGridLayout(layout);
    save_btn = new QPushButton(QIcon(":/cr32-action-filesave.png"), "Save");
    grid->addWidget(save_btn, 0, 0);
    int n = 0;

    while (iter[n].type!=VT_NULL)
    {
        switch (iter[n].type)
        {
        case VT_STRING: iter[n].control = new QLineEdit(iter[n].defaultval); break;
        case VT_MULTILINE: iter[n].control = new QTextEdit(iter[n].defaultval); break;
        case VT_LIST: iter[n].control = new NList; break; // custom control
        case VT_BOOLEAN: iter[n].control = new QCheckBox();
            if (!strcmp(iter[n].defaultval, "true"))
                ((QCheckBox*)(iter[n].control))->setChecked(true); 
            else ((QCheckBox*)(iter[n].control))->setChecked(false);
            break;
        case VT_FUNC2: iter[n].control = new N2List();
            ((N2List*)iter[n].control)->setHeaderLabels(QString(iter[n].defaultval).split(","));
            break;
        }
        n++;
        grid->addWidget(new QLabel(iter[n-1].description), n, 0);
        grid->addWidget(iter[n-1].control, n, 1);
    }
}

QWidget *CVarOutput::getLayout()
{
    return layout;
}

string CVarOutput::getScript()
{
    stringstream scr (stringstream::in | stringstream::out);
    var_t *iter = vdata;
    int n = 0;
    QStringList listy;

    while (iter[n].type!=VT_NULL)
    {
        switch (iter[n].type)
        {
            case VT_STRING: scr << iter[n].name << " = "; scr << "\"" << ((QLineEdit*)iter[n].control)->text().toStdString() << "\""; break;
            case VT_MULTILINE: scr << iter[n].name << " = "; scr << "[[\n" << ((QTextEdit*)iter[n].control)->toPlainText().toStdString() << "\n]]"; break;
            case VT_LIST: listy = ((NList*)iter[n].control)->getList(); 
                scr << iter[n].name << " = ";
                if (listy.size() < 1)
                {
                    scr << "{ }";
                    break;
                }
                scr << "{ ";
                scr << "\"";
                scr << listy.at(0).toLocal8Bit().constData();
                scr << "\"";
                for (int i = 1; i < listy.size(); i++)
                {
                    scr << ", \"";
                    scr << listy.at(i).toLocal8Bit().constData();
                    scr << "\"";
                }
                scr << " }";
                break;
            case VT_BOOLEAN: 
                scr << iter[n].name << " = ";
                if (((QCheckBox*)iter[n].control)->checkState() == Qt::Checked) 
                    scr << "true";
                else scr << "false";
            break;
            case VT_FUNC2:
                QStringList *params = ((N2List*)iter[n].control)->getValues();
                for (int i = 0; i < params->count()/2; i++)
                {
                    scr << "\n";
                    scr << iter[n].name << "(\"" << params->takeFirst().toStdString() << "\", \"" << params->takeFirst().toStdString() << "\")";
                }
                delete params;
                break;
        }
        scr << "\n";
        n++;
    }
    script = scr.str();
    return script;
}

QPushButton *CVarOutput::getSaveButton()
{
    return save_btn;
}



