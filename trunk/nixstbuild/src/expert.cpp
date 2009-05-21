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
#include <QApplication>
#include <QCloseEvent>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMenu>
#include <QMenuBar>
#include <QScrollArea>
#include <QStackedWidget>
#include <QStatusBar>
#include <QSplitter>
#include <QTextEdit>
#include <QToolBar>
#include <QToolButton>
#include <QTreeView>
#include <QVBoxLayout>

#include "main/lua/luafunc.h"
#include "main/lua/luatable.h"
#include "configw.h"
#include "dirbrowser.h"
#include "editor.h"
#include "editsettings.h"
#include "expert.h"
#include "rungen.h"

CExpertScreen::CExpertScreen(QWidget *parent, Qt::WindowFlags flags) : QMainWindow(parent, flags)
{
    setWindowTitle("Nixstbuild v0.1 - Expert mode");
    statusBar()->showMessage(tr("Ready"));
    resize(700, 500);

    editSettings = new CEditSettings(this); // Do this BEFORE createMenuBar()!

    createMenuBar();

    // UNDONE: Layout handling is a bit messy (not auto)
    const int gridw = 150, gridh = 70, listw = gridw+6;
    listWidget = new QListWidget;
//     listWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);
    listWidget->setViewMode(QListView::IconMode);
//     listWidget->setIconSize(QSize(iconw, iconh));
    listWidget->setMovement(QListView::Static);
    listWidget->setFixedWidth(listw);
//     listWidget->setWordWrap(true);
    listWidget->setWrapping(true);
    listWidget->setGridSize(QSize(gridw, gridh));
//     listWidget->setSpacing(6);

    QPalette p = listWidget->palette();
    QColor c = p.color(QPalette::Highlight);
    c.setAlpha(96);
    p.setColor(QPalette::Highlight, c);
    listWidget->setPalette(p);

    // UNDONE: Paths
    addListItem("nixstbuild/gfx/tux_config.png", "General configuration");
    addListItem("nixstbuild/gfx/ark_addfile.png", "Package configuration");
    addListItem("nixstbuild/gfx/ark_addfile.png", "Runtime configuration");
    addListItem("nixstbuild/gfx/ark_addfile.png", "File manager");
    connect(listWidget, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)),
            this, SLOT(changePage(QListWidgetItem *, QListWidgetItem*)));

    widgetStack = new QStackedWidget;
    addTab(new CGeneralConfTab);
    addTab(new CPackageConfTab);
    addTab(new CRunConfTab);
    addTab(new CFileManagerTab);
    
    QWidget *cw = new QWidget;
    setCentralWidget(cw);
    
    QHBoxLayout *layout = new QHBoxLayout(cw);

    layout->addWidget(listWidget);
    layout->addWidget(widgetStack);
}

void CExpertScreen::createFileMenu()
{
    QMenu *menu = menuBar()->addMenu(tr("&File"));

    QAction *action = new QAction(tr("&New project"), this);
    action->setShortcut(tr("Ctrl+N"));
    action->setStatusTip(tr("Create a new project"));
    connect(action, SIGNAL(triggered()), this, SLOT(newProject()));
    menu->addAction(action);

    action = new QAction(tr("&Open project"), this);
    action->setShortcut(tr("Ctrl+O"));
    action->setStatusTip(tr("Open existing project directory"));
    connect(action, SIGNAL(triggered()), this, SLOT(openProject()));
    menu->addAction(action);

    menu->addSeparator();
    
    action = new QAction(tr("&Save project"), this);
    action->setShortcut(tr("Ctrl+S"));
    action->setStatusTip(tr("Save project changes"));
    connect(action, SIGNAL(triggered()), this, SLOT(saveProject()));
    menu->addAction(action);

    menu->addSeparator();

    action = new QAction(tr("&Exit"), this);
    action->setShortcut(tr("Ctrl+Q"));
    action->setStatusTip(tr("Exit program"));
    connect(action, SIGNAL(triggered()), this, SLOT(close()));
    menu->addAction(action);
}

void CExpertScreen::createBuildRunMenu()
{
    QMenu *menu = menuBar()->addMenu(tr("&Build && Run"));

    QAction *action = new QAction(tr("&Build"), this);
    action->setShortcut(tr("F7"));
    action->setStatusTip(tr("Build the installer"));
    menu->addAction(action);

    action = new QAction(tr("Build && &Run"), this);
    action->setShortcut(tr("F8"));
    action->setStatusTip(tr("Build and launch the installer"));
    menu->addAction(action);

    action = new QAction(tr("&Preview"), this);
    action->setShortcut(tr("F9"));
    action->setStatusTip(tr("Previews the installer"));
    menu->addAction(action);
}

void CExpertScreen::createSettingsMenu()
{
    QMenu *menu = menuBar()->addMenu(tr("&Settings"));

    QAction *action = new QAction(tr("&Editor settings"), this);
    action->setStatusTip(tr("Text editor settings"));
    connect(action, SIGNAL(triggered()), this, SLOT(showEditSettings()));
    menu->addAction(action);

    action = new QAction(tr("&Nixstbuild settings"), this);
    action->setStatusTip(tr("Settings for Nixstbuild"));
    menu->addAction(action);
}

void CExpertScreen::createHelpMenu()
{
    QMenu *menu = menuBar()->addMenu(tr("&Help"));

    QAction *action = new QAction(tr("&Nixstaller manual"), this);
    action->setShortcut(tr("F1"));
    action->setStatusTip(tr("Displays the Nixstaller manual"));
    menu->addAction(action);

    action = new QAction(tr("&About"), this);
    action->setStatusTip(tr("About Nixstbuild"));
    menu->addAction(action);

    action = new QAction(tr("About Qt"), this);
    connect(action, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    menu->addAction(action);
}

void CExpertScreen::createMenuBar()
{
    createFileMenu();
    createBuildRunMenu();
    createSettingsMenu();
    createHelpMenu();
}

void CExpertScreen::addListItem(const QString &icon, const QString &name)
{
    QListWidgetItem *item = new QListWidgetItem(listWidget);

    if (listWidget->count() == 1)
        listWidget->setCurrentItem(item);
        
    QFont font;
    font.setBold(true);
    item->setFont(font);
    item->setIcon(QIcon(icon));
    item->setText(name);
    item->setTextAlignment(Qt::AlignHCenter);
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
}

void CExpertScreen::addTab(CBaseExpertTab *tab)
{
    tabs.push_back(tab);
    widgetStack->addWidget(tab);
}

void CExpertScreen::newProject()
{
    QFileDialog dialog(NULL, "New Project Directory");
    dialog.setModal(true);
    dialog.setFileMode(QFileDialog::DirectoryOnly);
    dialog.setOption(QFileDialog::ShowDirsOnly, true);
    dialog.setAcceptMode(QFileDialog::AcceptSave);

    if (dialog.exec() == QDialog::Accepted)
    {
        projectDir = dialog.selectedFiles()[0];
        for (std::vector<CBaseExpertTab *>::iterator it=tabs.begin();
            it!=tabs.end(); it++)
        {
            (*it)->newProject(projectDir);
        }
    }
}

void CExpertScreen::openProject()
{
    QString dir = QFileDialog::getExistingDirectory(NULL, "Open Project Directory");

    if (!dir.isEmpty())
    {
        projectDir = dir;
        for (std::vector<CBaseExpertTab *>::iterator it=tabs.begin();
             it!=tabs.end(); it++)
        {
            (*it)->loadProject(dir);
        }
    }
}

void CExpertScreen::saveProject()
{
    if (!projectDir.isEmpty())
    {
        for (std::vector<CBaseExpertTab *>::iterator it=tabs.begin();
             it!=tabs.end(); it++)
        {
            (*it)->saveProject(projectDir);
        }
    }
}

void CExpertScreen::changePage(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (!current)
        current = previous;

    widgetStack->setCurrentIndex(listWidget->row(current));
}

void CExpertScreen::showEditSettings()
{
    if (editSettings->exec() == QDialog::Accepted)
        CEditor::loadSettings(QEditor::editors());
}

void CExpertScreen::closeEvent(QCloseEvent *e)
{
    // UNDONE: Check for unsaved stuff
    e->accept();
}


CGeneralConfTab::CGeneralConfTab(QWidget *parent,
                                 Qt::WindowFlags flags) : CBaseExpertTab(parent, flags)
{
    // UNDONE
    QScrollArea *configScroll = new QScrollArea();
    configScroll->setWidget(qw = new QConfigWidget(this, "configprop.lua", "properties_config", "config.lua"));
}


CPackageConfTab::CPackageConfTab(QWidget *parent,
                                 Qt::WindowFlags flags) : CBaseExpertTab(parent, flags)
{
    // UNDONE
}


CRunConfTab::CRunConfTab(QWidget *parent,
                         Qt::WindowFlags flags) : CBaseExpertTab(parent, flags)
{
    QVBoxLayout *vbox = new QVBoxLayout(this);

    // UNDONE: Icons
    editor = new CEditor(this);
    vbox->addWidget(editor);

    editor->getToolBar()->addSeparator();
    QAction *a = editor->getToolBar()->addAction("File wizard");
    connect(a, SIGNAL(triggered()), this, SLOT(launchRunGen()));

    
    a = editor->getToolBar()->addAction("Insert code");
    QMenu *menu = new QMenu(this);
    a->setMenu(menu);
    QToolButton *tb = qobject_cast<QToolButton *>(editor->getToolBar()->widgetForAction(a));
    tb->setPopupMode(QToolButton::InstantPopup);

    a = menu->addAction("Installation screen");
    connect(a, SIGNAL(triggered()), this, SLOT(genRunScreenCB()));

    a = menu->addAction("Installation screen widget");
    QMenu *subMenu = new QMenu;
    a->setMenu(subMenu);
    TStringVec types;
    getWidgetTypes(types);
    for (TStringVec::iterator it=types.begin(); it!=types.end(); it++)
        subMenu->addAction(it->c_str());
    connect(subMenu, SIGNAL(triggered(QAction *)), this, SLOT(genRunWidgetCB(QAction *)));

    a = menu->addAction("Desktop entry");
    connect(a, SIGNAL(triggered()), this, SLOT(genRunDeskEntryCB()));

    a = menu->addAction("Generic function");
    connect(a, SIGNAL(triggered()), this, SLOT(genRunFunctionCB()));
}

void CRunConfTab::insertRunText(const QString &text)
{
    QDocumentCursor oldcur = editor->editor()->cursor();
    QStringList lines = text.split('\n');
    const int size = lines.size();
    const bool endNL = (text[text.size()-1] == '\n');

    int n = 0;
    foreach(QString line, lines)
    {
        editor->insertTextAtCurrent(line);

        if (endNL || (n != (size-1)))
            editor->indentNewLine();
        n++;
    }

    // Select new text
    QDocumentCursor cur = editor->editor()->cursor();
    cur.setSelectionBoundary(oldcur);
    editor->editor()->setCursor(cur);
}

void CRunConfTab::launchRunGen()
{
    CRunGenerator rg;
    if (rg.exec() == QDialog::Accepted)
    {
        editor->setText("");
        insertRunText(rg.getRun());
    }
}

void CRunConfTab::genRunWidgetCB(QAction *action)
{
    widgetitem wit;
    if (newInstWidget(action->text().toStdString(), wit))
    {
        NLua::CLuaFunc func("genInstWidget");
        // UNDONE: Let "screen" be configurable?
        func << wit.variable << "screen" << wit.func <<
                NLua::CLuaTable(wit.args.begin(), wit.args.end());
        func(1);
        std::string code;
        func >> code;
        insertRunText(code.c_str());
    }
}

void CRunConfTab::genRunScreenCB()
{
    CNewScreenDialog dialog;
    if (dialog.exec() == QDialog::Accepted)
    {
        NLua::CLuaFunc func("genInstScreen");
        func << dialog.variableName() << dialog.screenTitle() <<
                dialog.genCanActivate() << dialog.genActivate() <<
                dialog.genUpdate();

        CNewScreenDialog::widgetvec widgets;
        dialog.getWidgets(widgets);

        NLua::CLuaTable wtab;
        int n = 1;
        for (CNewScreenDialog::widgetvec::iterator it=widgets.begin();
             it!=widgets.end(); it++, n++)
        {
            NLua::CLuaTable wtabItem;
            wtabItem["func"] << it->func;
            wtabItem["variable"] << it->variable;
            wtabItem["args"] << NLua::CLuaTable(it->args.begin(), it->args.end());
            wtab[n] << wtabItem;
        }

        func << wtab;
        func(1);
        std::string code;
        func >> code;
        insertRunText(code.c_str());
    }
}

void CRunConfTab::genRunDeskEntryCB()
{
    CNewDeskEntryDialog dialog;
    
    if (dialog.exec() == QDialog::Accepted)
    {
        NLua::CLuaFunc func("genDeskEntry");
        func << dialog.getName() << dialog.getExec() <<
                dialog.getIcon() << dialog.getCategories();
        func(1);
        std::string code;
        func >> code;
        insertRunText(code.c_str());
    }
}

void CRunConfTab::genRunFunctionCB()
{
    QDocumentCursor cur = editor->editor()->cursor();
    insertRunText("function newfunc()\n\nend");

    // Move cursor behind "function "
    const int start = strlen("function "), end = strlen("newfunc");
    cur.movePosition(start);
    cur.movePosition(end, QDocumentCursor::NextCharacter, QDocumentCursor::KeepAnchor);
    editor->editor()->setCursor(cur);
}

void CRunConfTab::loadProject(const QString &dir)
{
    // UNDONE: Run wizard if file doesn't exist?
    editor->load(dir + "/run.lua");
}

void CRunConfTab::saveProject(const QString &dir)
{
    editor->save(dir + "/run.lua");
}

void CRunConfTab::newProject(const QString &dir)
{
    // UNDONE: Run wizard?
}


CFileManagerTab::CFileManagerTab(QWidget *parent,
                                 Qt::WindowFlags flags) : CBaseExpertTab(parent, flags)
{
    QVBoxLayout *vbox = new QVBoxLayout(this);
    
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QSplitter *mainSplit = new QSplitter(Qt::Vertical);
    vbox->addWidget(mainSplit);

    QFrame *frame = new QFrame;
    frame->setFrameStyle(QFrame::Box | QFrame::Sunken);
    mainSplit->addWidget(frame);
    QVBoxLayout *fvbox = new QVBoxLayout(frame);

    QLabel *label = new QLabel("<qt><b>File Browser</b></qt>");
    label->setAlignment(Qt::AlignHCenter);
    fvbox->addWidget(label);
    
    fvbox->addWidget(new CDirBrowser("/"));

    frame = new QFrame;
    frame->setFrameStyle(QFrame::Box | QFrame::Sunken);
    mainSplit->addWidget(frame);
    fvbox = new QVBoxLayout(frame);

    QSplitter *split = new QSplitter;
    fvbox->addWidget(split);

    QWidget *sw = new QWidget(split);
    QVBoxLayout *svbox = new QVBoxLayout(sw);

    label = new QLabel("<qt><b>Category</b></qt>");
    label->setAlignment(Qt::AlignHCenter);
    svbox->addWidget(label);
    
    svbox->addWidget(fileDestList = new QListWidget);
    fileDestList->setSortingEnabled(true);
    connect(fileDestList, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)),
            this, SLOT(changeDestBrowser(QListWidgetItem *, QListWidgetItem*)));

    sw = new QWidget(split);
    svbox = new QVBoxLayout(sw);
    
    label = new QLabel("<qt><b>Installation Files</b></qt>");
    label->setAlignment(Qt::AlignHCenter);
    svbox->addWidget(label);
    
    svbox->addWidget(fileDestBrowser = new CDirBrowser); // UNDONE? (needs proper root)
    
    split->setSizes(QList<int>() << 150 << 300);
}

void CFileManagerTab::createDestFilesItem(const QString &title,
                                        const QString &dir)
{
    QListWidgetItem *i = new QListWidgetItem(title, fileDestList);
    i->setData(Qt::UserRole, dir);
}

void CFileManagerTab::changeDestBrowser(QListWidgetItem *current,
                                      QListWidgetItem *previous)
{
    if (!current)
        current = previous;

    fileDestBrowser->setRootDir(current->data(Qt::UserRole).toString());
}

void CFileManagerTab::loadProject(const QString &dir)
{
    fileDestList->clear();

    CDirIter dirIter(dir.toStdString());
    while (dirIter)
    {
        std::string path = JoinPath(dir.toStdString(), dirIter->d_name);
        if (IsDir(path))
        {
            NLua::CLuaFunc func("getFriendlyFilesName");
            func << dirIter->d_name;
            if (func(1))
            {
                std::string title;
                func >> title;
                createDestFilesItem(title.c_str(), path.c_str());
            }
        }
        dirIter++;
    }

    if (fileDestList->count())
        fileDestList->setCurrentRow(0);
}

void CFileManagerTab::newProject(const QString &dir)
{
    QDir d(dir);
    d.mkdir("files_all");
    d.mkdir("files_extra");
    loadProject(dir);
}
