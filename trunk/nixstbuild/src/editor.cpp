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

#include <QAction>
#include <QKeyEvent>
#include <QSettings>
#include <QSignalMapper>
#include <QToolBar>
#include <QVBoxLayout>

#include "qcodeedit.h"
#include "qeditor.h"
#include "qformatscheme.h"
#include "qlanguagefactory.h"
#include "qlinemarksinfocenter.h"
#include "qpanel.h"

#include "editor.h"

QFormatScheme *CEditor::formats = NULL;
QLanguageFactory *CEditor::langFactory = NULL;
bool CEditor::init = true;

CEditor::CEditor(QWidget *parent, Qt::WindowFlags f) : QWidget(parent, f)
{
    if (init)
    {
        init = false;
        
        formats = new QFormatScheme("nixstbuild/qcodeedit/qxs/formats.qxf"); // UNDONE
        QDocument::setDefaultFormatScheme(formats);
    
        QLineMarksInfoCenter::instance()->loadMarkTypes("nixstbuild/qcodeedit/qxs/marks.qxm"); // UNDONE

        langFactory = new QLanguageFactory(formats, this);
        langFactory->addDefinitionPath("nixstbuild/qcodeedit/qxs"); // UNDONE
    }

    QVBoxLayout *vbox = new QVBoxLayout(this);

    editControl = new QCodeEdit(this);
    langFactory->setLanguage(editControl->editor(), "lua"); // Just Lua for now

    // HACK: Bug? Doesn't seem to be done automatically
    editControl->editor()->document()->setLineEnding(QDocument::defaultLineEnding());

    panelSignalMapper = new QSignalMapper;
    
    addPanel("Line Number Panel", QCodeEdit::West, "F11", true);
    addPanel("Fold Panel", QCodeEdit::West, "F12", true);
    addPanel("Line Change Panel", QCodeEdit::West, "", true);
    addPanel("Status Panel", QCodeEdit::South, "", true);
    addPanel("Search Replace Panel", QCodeEdit::South);

    connect(panelSignalMapper, SIGNAL(mapped(const QString &)),
            this, SLOT(updatePanel(const QString &)));

    connect(editControl->editor()->action("wrap"), SIGNAL(toggled(bool)),
            this, SLOT(updateWrap(bool)));
    connect(editControl->editor()->document(), SIGNAL(lineEndingChanged(int)),
            this, SLOT(updateLineEnding(int)));

    vbox->addWidget(createToolbars());
    vbox->addWidget(editControl->editor());

    loadSettings();
}

void CEditor::addPanel(const QString &name, QCodeEdit::Position pos, QString key, bool add)
{
    QAction *action = editControl->addPanel(name, pos, add);
    if (!key.isEmpty())
        action->setShortcut(QKeySequence(key));
    panelSignalMapper->setMapping(action, name);
    connect(action, SIGNAL(toggled(bool)),
            panelSignalMapper, SLOT(map()));
}

QToolBar *CEditor::createToolbars()
{
    // Create toolbars (copied from QCodeEdit example)
    toolBar = new QToolBar(tr("Edit"), this);
    toolBar->setIconSize(QSize(24, 24));
    toolBar->addAction(editControl->editor()->action("undo"));
    toolBar->addAction(editControl->editor()->action("redo"));
    toolBar->addSeparator();
    toolBar->addAction(editControl->editor()->action("cut"));
    toolBar->addAction(editControl->editor()->action("copy"));
    toolBar->addAction(editControl->editor()->action("paste"));
    toolBar->addSeparator();
    toolBar->addAction(editControl->editor()->action("indent"));
    toolBar->addAction(editControl->editor()->action("unindent"));
    toolBar->addAction(editControl->editor()->action("comment"));
    toolBar->addAction(editControl->editor()->action("uncomment"));
    toolBar->addSeparator();
    toolBar->addAction(editControl->editor()->action("find"));
    toolBar->addAction(editControl->editor()->action("replace"));
    toolBar->addAction(editControl->editor()->action("goto"));
    
    return toolBar;
}

void CEditor::updateWrap(bool e)
{
    (QSettings()).setValue("editor/wrap", e);
    
    int flags = QEditor::defaultFlags();
    
    if (e)
        flags |= QEditor::LineWrap;
    else
        flags &= ~QEditor::LineWrap;
    
    QEditor::setDefaultFlags(flags);
}

void CEditor::updateLineEnding(int le)
{
    (QSettings()).setValue("editor/line_endings", le);
    // UNDONE: Update line endings for other widgets?
}

void CEditor::updatePanel(const QString &paneln)
{
    QList<QPanel*> panels = editControl->panels();
    QString type;

    if (paneln == "Line Number Panel")
        type = "Line numbers";
    else if (paneln == "Line Change Panel")
        type = "Line changes";
    else if (paneln == "Fold Panel")
        type = "Fold indicators";
    else if (paneln == "Status Panel")
        type = "Status";
    
    foreach(QPanel *p, panels)
    {
        if (type == p->type())
        {
            QSettings settings;
            settings.beginGroup("editor");
            settings.beginGroup("panels");
            settings.setValue(type.toLower().replace(' ', '_'),
                              p->isVisibleTo(editControl->editor()));
            settings.endGroup();
            settings.endGroup();
            break;
        }
    }

    // UNDONE: update panels for other widgets?
}

void CEditor::load(const char *file)
{
    editControl->editor()->load(file);
}

void CEditor::loadSettings()
{
    QSettings settings;
    settings.beginGroup("editor");

    // Load panel states
    QStringList panels = QStringList()
            << "Line numbers"
            << "Line changes"
            << "Fold indicators"
            << "Status";
    
    settings.beginGroup("panels");
    
    foreach(QString p, panels)
    {
        bool show = settings.value(p.toLower().replace(' ', '_'), p != "Line numbers").toBool();
        
        if (!show)
            editControl->sendPanelCommand(p, "hide");
        else
            editControl->sendPanelCommand(p, "show");
    }
    
    settings.endGroup();
    settings.endGroup();
}
