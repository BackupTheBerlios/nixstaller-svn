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
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
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
    QDialog dialog;
    QVBoxLayout *vbox = new QVBoxLayout(&dialog);

    QGroupBox *groupBox = new QGroupBox("Main", &dialog);
    QFormLayout *form = new QFormLayout(groupBox);
    QLineEdit *fileNameEdit = new QLineEdit;
    form->addRow("filename (without .desktop suffix,\nempty for auto)", fileNameEdit);
    QLineEdit *execEdit = new QLineEdit;
    form->addRow("Executable (full path)", execEdit); // UNDONE: Neat way to specify pkg bins
    vbox->addWidget(groupBox);

    // UNDONE: Nicer way to handling for all those categories?
    groupBox = new QGroupBox("Category", &dialog);
    form = new QFormLayout(groupBox);
    QLabel *l = new QLabel("<qt>Valid categories can be found <a href=\"http://standards.freedesktop.org/menu-spec/latest/apa.html\">here</a>.");
    l->setOpenExternalLinks(true);
    form->addRow(l);
    QLineEdit *mainCatEdit = new QLineEdit;
    form->addRow("Main category", mainCatEdit);
    QLineEdit *addCatEdit = new QLineEdit;
    form->addRow("Additional category", addCatEdit);
    vbox->addWidget(groupBox);
    
    groupBox = new QGroupBox("Other", &dialog);
    form = new QFormLayout(groupBox);
    QLineEdit *iconEdit = new QLineEdit;
    form->addRow("Icon (empty for none)", iconEdit); // UNDONE: extra_files/ option
    vbox->addWidget(groupBox);
    
    QDialogButtonBox *bbox = new QDialogButtonBox(QDialogButtonBox::Ok |
            QDialogButtonBox::Cancel);
    connect(bbox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    connect(bbox, SIGNAL(rejected()), &dialog, SLOT(reject()));
    vbox->addWidget(bbox);
    
    if (dialog.exec() == QDialog::Accepted)
    {
        entryitem item;
        item.name = fileNameEdit->text().toStdString();
        item.exec = execEdit->text().toStdString();
        item.categories = mainCatEdit->text().toStdString();
        if (!addCatEdit->text().isEmpty())
        {
            if (!item.categories.empty())
                item.categories += ";";
            item.categories += addCatEdit->text().toStdString();
        }
        item.icon = iconEdit->text().toStdString();
        if (item.icon.empty())
            item.icon = "nil";
        else
            item.icon = "\"" + item.icon + "\"";
        
        QVariant v;
        v.setValue(item);
        QTreeWidgetItem *ti = new QTreeWidgetItem(QStringList() << fileNameEdit->text());
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
