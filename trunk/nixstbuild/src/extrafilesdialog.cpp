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

#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include "extrafilesdialog.h"
#include "dirbrowser.h"

CExtraFilesDialog::CExtraFilesDialog(const QString &prdir, QWidget *parent,
                                     Qt::WindowFlags flags) : QDialog(parent, flags)
{
    setModal(true);

    QDir dir(prdir);
    QString dest = prdir + "/files_extra";
    dir.mkdir(dest);
    // UNDONE: Check if everything went OK?

    QVBoxLayout *vbox = new QVBoxLayout(this);
    
    vbox->addWidget(new QLabel("In this dialog screen you can select and manage all the files that are available at runtime (\"extra files\").\n"));

    vbox->addWidget(extraBrowser = new CDirBrowser(dest));
    extraBrowser->setSingleSelection(true);
    connect(extraBrowser, SIGNAL(mouseClicked()), this, SLOT(updateSelCB()));

    QHBoxLayout *hbox = new QHBoxLayout;
    vbox->addLayout(hbox);

    hbox->addWidget(selectButton =
            new QPushButton(QIcon(style()->standardIcon(QStyle::SP_DialogOkButton)),
                            "Select"));
    connect(selectButton, SIGNAL(clicked()), this, SLOT(accept()));
    selectButton->setEnabled(false);

    hbox->addWidget(addButton =
            new QPushButton(QIcon(style()->standardIcon(QStyle::SP_DialogOpenButton)),
                            "Add file"));
    connect(addButton, SIGNAL(clicked()), this, SLOT(addCB()));

    QPushButton *cancelButton =
            new QPushButton(QIcon(style()->standardIcon(QStyle::SP_DialogCancelButton)),
                            "Cancel");
    hbox->addWidget(cancelButton);
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

void CExtraFilesDialog::updateSelCB()
{
    QString sel = extraBrowser->getSelection();
    selectButton->setEnabled(!sel.isEmpty() && QFileInfo(sel).isFile());
}

void CExtraFilesDialog::addCB()
{
    QString file = QFileDialog::getOpenFileName(NULL, "Add an extra (runtime) file");

    if (!file.isEmpty())
        extraBrowser->copyFile(file);
}

QString CExtraFilesDialog::file() const
{
    return extraBrowser->getSelection();
}
