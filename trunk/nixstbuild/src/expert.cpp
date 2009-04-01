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

#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QStackedWidget>

#include "expert.h"

CExpertScreen::CExpertScreen(QWidget *parent, Qt::WindowFlags flags)
{
    setWindowTitle("Nixstbuild v0.1 - Expert mode");

    const int listw = 156, iconw = 96, iconh = 84;
    m_pListWidget = new QListWidget;
    m_pListWidget->setViewMode(QListView::IconMode);
    m_pListWidget->setIconSize(QSize(iconw, iconh));
    m_pListWidget->setMovement(QListView::Static);
    m_pListWidget->setMaximumWidth(listw);
    m_pListWidget->setSpacing(6);

    QPalette p = m_pListWidget->palette();
    QColor c = p.color(QPalette::Highlight);
    c.setAlpha(96);
    p.setColor(QPalette::Highlight, c);
    m_pListWidget->setPalette(p);

    // UNDONE: Paths
    AddListItem("nixstbuild/gfx/tux_config.png", "General configuration");
    AddListItem("nixstbuild/gfx/ark_addfile.png", "Package configuration");
    connect(m_pListWidget, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)),
            this, SLOT(ChangePage(QListWidgetItem *, QListWidgetItem*)));

    m_pWidgetStack = new QStackedWidget;
    m_pWidgetStack->addWidget(CreateGeneralConf());
    m_pWidgetStack->addWidget(CreatePackageConf());
    
    QWidget *cw = new QWidget();
    setCentralWidget(cw);
    
    QHBoxLayout *layout = new QHBoxLayout(cw);

    layout->addWidget(m_pListWidget);
    layout->addWidget(m_pWidgetStack);
}

void CExpertScreen::ChangePage(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (!current)
        current = previous;

    m_pWidgetStack->setCurrentIndex(m_pListWidget->row(current));
}

void CExpertScreen::AddListItem(QString icon, QString name)
{
    QListWidgetItem *item = new QListWidgetItem(m_pListWidget);
    QFont font;
    font.setBold(true);
    item->setFont(font);
    item->setIcon(QIcon(icon));
    item->setText(name);
    item->setTextAlignment(Qt::AlignHCenter);
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
}

QWidget *CExpertScreen::CreateGeneralConf()
{
    QWidget *ret = new QWidget;
    QFormLayout *layout = new QFormLayout(ret);
    layout->addRow("Application Name", new QLineEdit);
    return ret;
}

QWidget *CExpertScreen::CreatePackageConf()
{
    QWidget *ret = new QWidget;
    QFormLayout *layout = new QFormLayout(ret);
    layout->addRow("Package name", new QLineEdit);
    return ret;
}
