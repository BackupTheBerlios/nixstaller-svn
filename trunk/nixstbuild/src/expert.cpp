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
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QRadioButton>
#include <QScrollArea>
#include <QSettings>
#include <QStackedWidget>
#include <QStatusBar>
#include <QSplitter>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>

#include "main/lua/luafunc.h"
#include "main/lua/luatable.h"
#include "main/pseudoterminal.h"
#include "configw.h"
#include "configwidget.h"
#include "dirbrowser.h"
#include "dirinput.h"
#include "editor.h"
#include "editsettings.h"
#include "expert.h"
#include "rungen.h"
#include "utils.h"

CExpertScreen::CExpertScreen(QWidget *parent, Qt::WindowFlags flags) : QMainWindow(parent, flags)
{
    updateTitle();
    statusBar()->showMessage(tr("Ready"));
    resize(700, 500);

    editSettings = new CEditSettings(this); // Do this BEFORE createMenuBar()!

    createMenuBar();

    mainStack = new QStackedWidget;
    setCentralWidget(mainStack);

    QWidget *sw = new QWidget;
    QVBoxLayout *vbox = new QVBoxLayout(sw);
    mainStack->addWidget(sw);

    QSplitter *stackS = new QSplitter(Qt::Vertical);
    stackS->setChildrenCollapsible(false);
    vbox->addWidget(stackS);

    stackS->addWidget(sw = new QWidget);
    QHBoxLayout *hbox = new QHBoxLayout(sw);

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
    hbox->addWidget(listWidget);

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
    hbox->addWidget(widgetStack);

    stackS->addWidget(consoleWidget = new QPlainTextEdit);
    consoleWidget->setReadOnly(true);
    consoleWidget->setCenterOnScroll(true);

    QToolBar *tbar = new QToolBar;
    QAction *a = tbar->addAction("Console");
    connect(a, SIGNAL(toggled(bool)), this, SLOT(toggleConsole(bool)));
    a->setCheckable(true);
    a->setChecked(true); // UNDONE: Setting
    vbox->addWidget(tbar);

    QWidget *stackW = new QWidget;
    mainStack->addWidget(stackW);
    hbox = new QHBoxLayout(stackW);
    QLabel *label = new QLabel("<qt>Welcome to Nixstbuild!<br>You can load or create projects from the <b>File</b> menu.</qt>");
    label->setFrameStyle(QFrame::Box | QFrame::Sunken);
    label->setAlignment(Qt::AlignCenter);
    label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    label->setMargin(25);
    hbox->addWidget(label);

    mainStack->setCurrentIndex(1);
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

    recentFilesMenu = menu->addMenu(tr("&Recent projects"));
    // NOTE: updateRecentMenu() is connected to parent menu. This is so
    // we can correctly enable/disable the recent files menu
    connect(menu, SIGNAL(aboutToShow()), this, SLOT(updateRecentMenu()));
    connect(recentFilesMenu, SIGNAL(triggered(QAction *)),
            this, SLOT(openRecentCB(QAction *)));

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
    buildMenu = menuBar()->addMenu(tr("&Build && Run"));

    QAction *action = new QAction(tr("&Build"), this);
    action->setShortcut(tr("F7"));
    action->setStatusTip(tr("Build the installer"));
    connect(action, SIGNAL(triggered()), this, SLOT(build()));
    buildMenu->addAction(action);

    action = new QAction(tr("Build && &Run"), this);
    action->setShortcut(tr("F8"));
    action->setStatusTip(tr("Build and launch the installer"));
    connect(action, SIGNAL(triggered()), this, SLOT(buildAndRun()));
    buildMenu->addAction(action);

    action = new QAction(tr("&Fast run"), this);
    action->setShortcut(tr("F9"));
    action->setStatusTip(tr("Quickly runs the installer without building it"));
    connect(action, SIGNAL(triggered()), this, SLOT(fastrun()));
    buildMenu->addAction(action);

    buildMenu->setEnabled(false); // Disabled by default (no project loaded yet)
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
    connect(action, SIGNAL(triggered()), this, SLOT(showNixstbSettings()));
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

void CExpertScreen::updateTitle()
{
    QString tag;
    if (projectDir.isEmpty())
        tag = "(No project loaded)";
    else
        tag = QString("%1 - [%2]").arg(QDir(projectDir).dirName()).arg(projectDir);

    setWindowTitle(QString("Nixstbuild v0.1 - Expert mode - %1").arg(tag));
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

void CExpertScreen::cleanRecent()
{
    QSettings settings;
    QStringList list = settings.value("recent").toStringList();
    QStringList cleanList;
    bool update = false;
    
    foreach(QString p, list)
    {
        if (!QFile::exists(p))
            update = true;
        else
            cleanList << p;
    }

    if (update)
        settings.setValue("recent", cleanList);
}

void CExpertScreen::addRecent(const QString &path)
{
    cleanRecent();
    QSettings settings;
    QStringList recent = settings.value("recent").toStringList();
    recent.removeAll(path);
    recent.push_front(path);

    if (recent.size() > 10) // UNDONE: Setting?
        recent.pop_back();
    settings.setValue("recent", recent);
}

void CExpertScreen::setProjectDir(const QString &dir)
{
    projectDir = dir;
    updateTitle();
    addRecent(projectDir);
    mainStack->setCurrentIndex(0);
    buildMenu->setEnabled(true);
}

void CExpertScreen::loadProject(const QString &dir)
{
    setProjectDir(dir);
    for (std::vector<CBaseExpertTab *>::iterator it=tabs.begin();
         it!=tabs.end(); it++)
    {
        (*it)->loadProject(dir);
    }
}

QString CExpertScreen::getNixstPath()
{
    QSettings settings;
    QString path = settings.value("nixstaller_path").toString();
    
    if (!verifyNixstPath(path))
    {
        QMessageBox::critical(0, "Error", "Wrong Nixstaller path"); // UNDONE
        return QString();
    }

    return path;
}

void CExpertScreen::clearConsole()
{
    consoleWidget->setPlainText("");
}

// Appends text (without adding new paragraph)
void CExpertScreen::appendConsoleText(const QString &text)
{
    QTextCursor cur = consoleWidget->textCursor();
    cur.movePosition(QTextCursor::End);
    consoleWidget->setTextCursor(cur);
    consoleWidget->insertPlainText(text);
}

bool CExpertScreen::runInConsole(const QString &command, bool silent)
{
    consoleWidget->appendPlainText(QString("Executing %1\n\n").arg(command));

    try
    {
        CPseudoTerminal ps;
        ps.Exec(command.toStdString());
        while (ps)
        {
            QCoreApplication::processEvents();
            
            if (ps.CheckForData())
            {
                std::string line;
                CPseudoTerminal::EReadStatus stat = ps.ReadLine(line);

                if (stat != CPseudoTerminal::READ_AGAIN)
                {
                    if (stat == CPseudoTerminal::READ_LINE)
                        line += "\n";

                    appendConsoleText(line.c_str());
                }
            }
        }

        int ret = ps.GetRetStatus();
        consoleWidget->appendPlainText(QString("Command exited with code %1").arg(ret));

        if (ret != 0)
        {
            if (!silent)
                QMessageBox::critical(NULL, "Error", "Failed to run command!");
            return false;
        }
    }
    catch (Exceptions::CExIO &e)
    {
        if (!silent)
            QMessageBox::critical(NULL, "Error", QString("Failed to run command: %1").arg(e.what()));
        return false;
    }

    return true;
}

void CExpertScreen::newProject()
{
    QFileDialog dialog(NULL, "New Project Directory");
    dialog.setModal(true);
    dialog.setFileMode(QFileDialog::DirectoryOnly);
//     dialog.setOptions(QFileDialog::ShowDirsOnly, true);
    dialog.setAcceptMode(QFileDialog::AcceptSave);

    if (dialog.exec() == QDialog::Accepted)
    {
        setProjectDir(dialog.selectedFiles()[0]);
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
        loadProject(dir);
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

void CExpertScreen::updateRecentMenu()
{
    cleanRecent();
    QSettings settings;
    QStringList list = settings.value("recent").toStringList();
    recentFilesMenu->clear();

    if (list.empty())
        recentFilesMenu->setEnabled(false);
    else
    {
        recentFilesMenu->setEnabled(true);
        foreach(QString p, list)
            recentFilesMenu->addAction(p);
    }
}

void CExpertScreen::openRecentCB(QAction *action)
{
    loadProject(action->text());
}

bool CExpertScreen::build()
{
    QString npath = getNixstPath();
    if (!npath.isEmpty())
    {
        QString command = QString("cd %1 && %2/geninstall.sh %1 2>&1").arg(projectDir).arg(npath);
        clearConsole();
        return runInConsole(command);
    }

    return false;
}

void CExpertScreen::buildAndRun()
{
    if (build())
    {
        // NOTE: We don't clean the console here so user is able
        // to view from both building & running

        // UNDONE: Change if outname can be specified
        runInConsole(QString("%1/setup.sh").arg(projectDir), true);
    }
}

void CExpertScreen::fastrun()
{
    QSettings settings;
    QDialog dialog;
    dialog.setModal(true);
    QFormLayout *form = new QFormLayout(&dialog);
    
    QComboBox *combo = new QComboBox;
    
    combo->addItem("Default");
    
    NLua::CLuaFunc frFunc("frontends");
    TStringVec frlist;
    frFunc(1);
    frFunc >> frlist;
    
    for (TStringVec::iterator it=frlist.begin(); it!=frlist.end(); it++)
        combo->addItem(it->c_str());
    int prev = settings.value("fastrun/frontend", 0).toInt();
    if ((prev > 0) && (prev < frlist.size()))
        combo->setCurrentIndex(prev);
    form->addRow("Frontend", combo);
    
    QDialogButtonBox *bbox = new QDialogButtonBox(QDialogButtonBox::Ok |
            QDialogButtonBox::Cancel);
    connect(bbox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    connect(bbox, SIGNAL(rejected()), &dialog, SLOT(reject()));
    form->addWidget(bbox);
    
    if (dialog.exec() == QDialog::Accepted)
    {
        QString npath = getNixstPath();
        if (!npath.isEmpty())
        {
            clearConsole();
            
            QString frarg;
            
            if (combo->currentIndex()) // Default == 0 --> don't specify
            {
                NLua::CLuaFunc f("internFrontendName");
                f << combo->currentText().toStdString();
                f(1);
                std::string frontend;
                f >> frontend;
                frarg = QString("-f %1").arg(frontend.c_str());
            }
            
            // UNDONE: handle ncurses
            settings.setValue("fastrun/frontend", combo->currentIndex());
            
            runInConsole(QString("%1/fastrun.sh %2 %3").arg(npath).arg(frarg).arg(projectDir), true);
        }
    }
}

void CExpertScreen::showEditSettings()
{
    if (editSettings->exec() == QDialog::Accepted)
        CEditor::loadSettings(QEditor::editors());
}

void CExpertScreen::showNixstbSettings()
{
    QSettings settings;
    QDialog dialog;
    dialog.setModal(true);
    QVBoxLayout *vbox = new QVBoxLayout(&dialog);
    
    QGroupBox *box = new QGroupBox;
    vbox->addWidget(box);
    QFormLayout *form = new QFormLayout(box);
    
    CDirInput *dirInput = new CDirInput(settings.value("nixstaller_path").toString());
    form->addRow("Nixstaller path", dirInput);
    
    QDialogButtonBox *bbox = new QDialogButtonBox(QDialogButtonBox::Ok |
            QDialogButtonBox::Cancel);
    connect(bbox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    connect(bbox, SIGNAL(rejected()), &dialog, SLOT(reject()));
    vbox->addWidget(bbox);
    
    while (dialog.exec() == QDialog::Accepted)
    {
        if (verifyNixstPath(dirInput->getDir()))
        {
            settings.setValue("nixstaller_path", dirInput->getDir());
            break;
        }
        else
            QMessageBox::critical(0, "Error", "Invalid Nixstaller path specified.");
    }
}

void CExpertScreen::changePage(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (!current)
        current = previous;

    widgetStack->setCurrentIndex(listWidget->row(current));
}

void CExpertScreen::toggleConsole(bool checked)
{
    consoleWidget->setVisible(checked);
}

void CExpertScreen::closeEvent(QCloseEvent *e)
{
    // UNDONE: Check for unsaved stuff
    e->accept();
}


CGeneralConfTab::CGeneralConfTab(QWidget *parent,
                                 Qt::WindowFlags flags) : CBaseExpertTab(parent, flags)
{
    QVBoxLayout *vbox = new QVBoxLayout(this);
    
    QScrollArea *configScroll = new QScrollArea;
    configScroll->setFrameStyle(QFrame::Plain | QFrame::NoFrame);
    configScroll->setWidget(configWidget = new CConfigWidget);
    
    vbox->addWidget(configScroll);
}

void CGeneralConfTab::loadProject(const QString &dir)
{
    configWidget->loadConfig(dir.toStdString());
}

void CGeneralConfTab::saveProject(const QString &dir)
{
    configWidget->saveConfig(dir.toStdString());
}

void CGeneralConfTab::newProject(const QString &dir)
{
    configWidget->newConfig(dir.toStdString());
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
    editor->getToolBar()->addAction("File wizard", this, SLOT(launchRunGen()));

    
    QAction *a = editor->getToolBar()->addAction("Insert code");
    QMenu *menu = new QMenu(this);
    a->setMenu(menu);
    QToolButton *tb = qobject_cast<QToolButton *>(editor->getToolBar()->widgetForAction(a));
    tb->setPopupMode(QToolButton::InstantPopup);

    menu->addAction("Installation screen", this, SLOT(genRunScreenCB()));

    QMenu *subMenu = menu->addMenu("Installation screen widget");
    TStringVec types;
    getWidgetTypes(types);
    for (TStringVec::iterator it=types.begin(); it!=types.end(); it++)
        subMenu->addAction(it->c_str());
    connect(subMenu, SIGNAL(triggered(QAction *)), this, SLOT(genRunWidgetCB(QAction *)));

    menu->addAction("Desktop entry", this, SLOT(genRunDeskEntryCB()));

    menu->addAction("Generic function", this, SLOT(genRunFunctionCB()));
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

    // Move cursor behind "function " and select function name
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
    mainSplit->setChildrenCollapsible(false);
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
    split->setChildrenCollapsible(false);
    fvbox->addWidget(split);

    QWidget *sw = new QWidget(split);
    QVBoxLayout *svbox = new QVBoxLayout(sw);

    label = new QLabel("<qt><b>Category</b></qt>");
    label->setAlignment(Qt::AlignHCenter);
    svbox->addWidget(label);

    QHBoxLayout *hbox = new QHBoxLayout;
    svbox->addLayout(hbox);
    
    QPushButton *but = new QPushButton(QIcon(":/edit_add.png"), "Add");
    but->setMenu(addInstDirMenu = new QMenu(but));
    connect(addInstDirMenu, SIGNAL(aboutToShow()), this, SLOT(updateAddDirMenu()));
    connect(addInstDirMenu, SIGNAL(triggered(QAction *)), this, SLOT(instDirMenuCB(QAction *)));
    hbox->addWidget(but);

    but = new QPushButton(QIcon(":/edit_remove.png"), "Remove");
    connect(but, SIGNAL(clicked()), this, SLOT(removeInstDir()));
    hbox->addWidget(but);

    svbox->addWidget(fileDestList = new QListWidget);
    fileDestList->setSortingEnabled(true);
    connect(fileDestList, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)),
            this, SLOT(changeDestBrowser(QListWidgetItem *, QListWidgetItem*)));

    sw = new QWidget(split);
    svbox = new QVBoxLayout(sw);
    
    label = new QLabel("<qt><b>Installation Files</b></qt>");
    label->setAlignment(Qt::AlignHCenter);
    svbox->addWidget(label);

    browserStack = new QStackedWidget;
    svbox->addWidget(browserStack);
    
    browserStack->addWidget(fileDestBrowser = new CDirBrowser);
    browserStack->addWidget(new QLabel("<qt>Use the <b>Add</b> button to add installation directories.</qt>"));
    browserStack->setCurrentIndex(1);
    
    split->setSizes(QList<int>() << 150 << 300);
}

QListWidgetItem * CFileManagerTab::createDestFilesItem(const QString &title,
                                        const QString &dir)
{
    QListWidgetItem *i = new QListWidgetItem(title, fileDestList);
    i->setData(Qt::UserRole, dir);
    return i;
}

bool CFileManagerTab::hasDir(const QString &dir)
{
    const int count = fileDestList->count();

    for (int i=0; i<count; i++)
    {
        QString p = fileDestList->item(i)->data(Qt::UserRole).toString();
        if (QDir(p).dirName() == dir)
            return true;
    }

    return false;
}

void CFileManagerTab::getOSs(TStringVec &out)
{
    NLua::CLuaTable tab;
    NLua::CLuaFunc func("getFilesOSs");
    
    func(1);
    func >> tab;
    tab.ToVec<std::string>(out);
}

void CFileManagerTab::getArchs(TStringVec &out)
{
    NLua::CLuaTable tab;
    NLua::CLuaFunc func("getFilesCPUArchs");
    
    func(1);
    func >> tab;
    tab.ToVec<std::string>(out);
}

void CFileManagerTab::addMenuAction(QMenu *menu, const QString &title, const QString &dir)
{
    QAction *a = menu->addAction(title);
    a->setData(dir);
}

void CFileManagerTab::updateAddDirMenu()
{
    addInstDirMenu->clear();

    TStringVec OSs, Archs;
    getOSs(OSs);
    getArchs(Archs);

    QMenu *genericMenu = NULL;

    if (!hasDir("files_all"))
    {
        genericMenu = addInstDirMenu->addMenu("Generic OS");
        addMenuAction(genericMenu, "Generic", "files_all");
    }

    for (TStringVec::iterator os=OSs.begin(); os!=OSs.end(); os++)
    {
        QMenu *osMenu = NULL;

        std::string friendlyOS;
        NLua::CLuaFunc func("getFriendlyOS");
        func << *os;
        func(1);
        func >> friendlyOS;
        
        QString dir = QString("files_%1_all").arg(os->c_str());
        if (!hasDir(dir))
        {
            osMenu = addInstDirMenu->addMenu(friendlyOS.c_str());
            addMenuAction(osMenu, "Generic CPU Arch", dir);
        }

        for (TStringVec::iterator arch=Archs.begin(); arch!=Archs.end(); arch++)
        {
            dir = QString("files_%1_%2").arg(os->c_str()).arg(arch->c_str());
            if (!hasDir(dir))
            {
                if (!osMenu)
                    osMenu = addInstDirMenu->addMenu(friendlyOS.c_str());
                
                std::string friendlyArch;
                NLua::CLuaFunc func("getFriendlyArch");
                func << *arch;
                func(1);
                func >> friendlyArch;

                addMenuAction(osMenu, friendlyArch.c_str(), dir);
            }
        }
    }

    for (TStringVec::iterator arch=Archs.begin(); arch!=Archs.end(); arch++)
    {
        QString dir = QString("files_all_%1").arg(arch->c_str());
        if (!hasDir(dir))
        {
            if (!genericMenu)
                genericMenu = addInstDirMenu->addMenu("Generic OS");
            addMenuAction(genericMenu, arch->c_str(), dir);
        }
    }

    if (!hasDir("files_extra"))
    {
        addInstDirMenu->addSeparator();
        addMenuAction(addInstDirMenu, "Extra (runtime)", "files_extra");
    }
}

void CFileManagerTab::instDirMenuCB(QAction *action)
{
    if (projectDir.isEmpty())
        return;

    std::string subDir = action->data().toString().toStdString();
    std::string path = JoinPath(projectDir.toStdString(), subDir);

    try
    {
        MKDirRec(path);
        NLua::CLuaFunc func("getFriendlyFilesName");
        func << subDir;
        func(1);
        
        std::string title;
        func >> title;
        fileDestList->setCurrentItem(createDestFilesItem(title.c_str(), path.c_str()));
        
        if (fileDestList->count() == 1)
            browserStack->setCurrentIndex(0);
    }
    catch(Exceptions::CExIO &e)
    {
        QMessageBox::critical(0, "Error", QString("Failed to create directory: %1").arg(e.what()));
    }
}

void CFileManagerTab::removeInstDir()
{
    QListWidgetItem *current = fileDestList->currentItem();

    if (current)
    {
        if (QMessageBox::question(0, "Remove directory",
                                  QString("Are you sure you want to remove the selected directory?"),
                                  QMessageBox::Yes | QMessageBox::No,
                                  QMessageBox::No) == QMessageBox::Yes)
        {
            fileDestBrowser->removeDir(current->data(Qt::UserRole).toString());
            delete current;
        }

        if (!fileDestList->count())
            browserStack->setCurrentIndex(1);
    }
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
    projectDir = dir;
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
    {
        fileDestList->setCurrentRow(0);
        browserStack->setCurrentIndex(0);
    }
    else
        browserStack->setCurrentIndex(1);
}

void CFileManagerTab::newProject(const QString &dir)
{
    QDir d(dir);
    d.mkdir("files_all");
    d.mkdir("files_extra");
    loadProject(dir);
}
