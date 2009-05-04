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

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

#include "deskentrywidget.h"

CDesktopEntryWidget::CDesktopEntryWidget(QWidget *parent,
                                         Qt::WindowFlags flags) : CTreeEdit(parent, flags)
{
    QPushButton *button = new QPushButton("Add");
    connect(button, SIGNAL(clicked()), this, SLOT(addEntry()));
    insertButton(0, button);
}

void CDesktopEntryWidget::addEntry()
{
    CNewDeskEntryDialog dialog;
    
    if (dialog.exec() == QDialog::Accepted)
    {
        entryitem item(dialog.getName(), dialog.getExec(), dialog.getCategories(),
                       dialog.getIcon());
        QVariant v;
        v.setValue(item);
        QTreeWidgetItem *ti = new QTreeWidgetItem(QStringList() << item.name.c_str());
        ti->setData(0, Qt::UserRole, v);
        insertAtCurrent(ti);
    }
}


void CDesktopEntryWidget::getEntries(entryvec &entries)
{
    const int size = itemCount();
    for (int i=0; i<size; i++)
    {
        QTreeWidgetItem *item = itemAt(i);
        QVariant v = item->data(0, Qt::UserRole);
        if (!v.isNull())
            entries.push_back(v.value<entryitem>());
    }
}


CNewDeskEntryDialog::CNewDeskEntryDialog(QWidget *parent,
                                         Qt::WindowFlags flags) : QDialog(parent, flags)
{
    QVBoxLayout *vbox = new QVBoxLayout(this);

    QGroupBox *groupBox = new QGroupBox("Main", this);
    QFormLayout *form = new QFormLayout(groupBox);
    form->addRow("filename (without .desktop suffix)", fileNameEdit = new QLineEdit);
    // UNDONE: Neat way to specify pkg bins
    form->addRow("Executable (full path)", execEdit = new QLineEdit);
    vbox->addWidget(groupBox);

    // UNDONE: Nicer way to handling for all those categories?
    groupBox = new QGroupBox("Category", this);
    form = new QFormLayout(groupBox);
    QLabel *l = new QLabel("<qt>Valid categories can be found <a href=\"http://standards.freedesktop.org/menu-spec/latest/apa.html\">here</a>.");
    l->setOpenExternalLinks(true);
    form->addRow(l);
    form->addRow("Main category", mainCatEdit = new QLineEdit);
    form->addRow("Additional category", addCatEdit = new QLineEdit);
    vbox->addWidget(groupBox);
    
    groupBox = new QGroupBox("Other", this);
    form = new QFormLayout(groupBox);
    // UNDONE: extra_files/ option
    form->addRow("Icon (empty for none)", iconEdit = new QLineEdit);
    // UNDONE: Way to specify more fields?
    vbox->addWidget(groupBox);
    
    QDialogButtonBox *bbox = new QDialogButtonBox(QDialogButtonBox::Ok |
            QDialogButtonBox::Cancel);
    connect(bbox, SIGNAL(accepted()), this, SLOT(OK()));
    connect(bbox, SIGNAL(rejected()), this, SLOT(reject()));
    vbox->addWidget(bbox);
}

void CNewDeskEntryDialog::OK()
{
    if (fileNameEdit->text().isEmpty())
    {
        QMessageBox::information(NULL, "Missing information",
                                 "Please specify a file name.");
        fileNameEdit->setFocus(Qt::OtherFocusReason);
    }
    else
        accept();
}

std::string CNewDeskEntryDialog::getName(void) const
{
    return fileNameEdit->text().toStdString();
}

std::string CNewDeskEntryDialog::getExec(void) const
{
    return execEdit->text().toStdString();
}

std::string CNewDeskEntryDialog::getCategories(void) const
{
    std::string ret = mainCatEdit->text().toStdString();
    if (!addCatEdit->text().isEmpty())
    {
        if (!ret.empty())
            ret += ";";
        ret += addCatEdit->text().toStdString();
    }
    
    return ret;
}

std::string CNewDeskEntryDialog::getIcon(void) const
{
    std::string ret = iconEdit->text().toStdString();
    if (ret.empty())
        ret = "nil";
    else
        ret = "\"" + ret + "\"";
    
    return ret;
}

