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

#include <QCompleter>
#include <QDirModel>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QStyle>
#include <QToolButton>

#include "dirinput.h"

CDirInput::CDirInput(const QString &dir, QWidget *parent,
                     Qt::WindowFlags flags) : QWidget(parent, flags)
{
    QHBoxLayout *hbox = new QHBoxLayout(this);
    
    hbox->addWidget(dirEdit = new QLineEdit);
    if (dir.isEmpty())
        dirEdit->setText("/");
    else
        dirEdit->setText(dir);
    
    QCompleter *comp = new QCompleter(dirEdit);
    comp->setModel(new QDirModel);
    dirEdit->setCompleter(comp);
    
    QToolButton *openButton = new QToolButton;
    openButton->setIcon(QIcon(style()->standardIcon(QStyle::SP_DirOpenIcon)));
    connect(openButton, SIGNAL(clicked()), this, SLOT(openBrowser()));
    hbox->addWidget(openButton);
}

void CDirInput::openBrowser()
{
    QString dir = QFileDialog::getExistingDirectory(NULL, "Open Project Directory",
            dirEdit->text());
    if (!dir.isEmpty())
        dirEdit->setText(dir);
}

QString CDirInput::getDir() const
{
    if (dirEdit->text().isEmpty())
        return "/";
    
    return dirEdit->text();
}

void CDirInput::setDir(const QString &dir)
{
    dirEdit->setText(dir);
}
