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
#include <QClipboard>
#include <QFileInfo>
#include <QHeaderView>
#include <QItemSelection>
#include <QLineEdit>
#include <QMessageBox>
#include <QMenu>
#include <QMimeData>
#include <QMouseEvent>
#include <QProgressDialog>
#include <QSettings>
#include <QTreeView>
#include <QToolBar>
#include <QUrl>
#include <QVBoxLayout>

#include "dirbrowser.h"

namespace {

QString fromRelRoot(QString path, QString root)
{
    if (path[0] != '/')
        path = "/" + path;
    
    if (root[root.length()-1] != '/')
        root += "/";

    path.replace(0, 1, root);
    return path;
}

QString toRelRoot(QString path, QString root)
{
    if (root[root.length()-1] != '/')
        root += "/";
    
    path.replace(0, root.length(), "/");
    return path;
}

}

CDirBrowser::CDirBrowser(const QString &d, QWidget *parent,
                         Qt::WindowFlags flags) : QWidget(parent, flags)
{
    QVBoxLayout *vbox = new QVBoxLayout(this);

    vbox->addWidget(createToolBar());

    browserModel = new CDirModel;
    browserModel->setReadOnly(false);
    browserModel->setFilter(browserModel->filter() | QDir::Hidden);

    browserView = new CDirView;
    connect(browserView, SIGNAL(mouseClicked()),
            this, SLOT(updateActions()));
    connect(browserView, SIGNAL(clicked(const QModelIndex &)),
            this, SLOT(setRootIndex(const QModelIndex &)));
    connect(browserView, SIGNAL(doubleClicked(const QModelIndex &)),
            this, SLOT(setRootIndex(const QModelIndex &)));

    createContextMenu();

    proxyModel = new CDirSortProxy(browserModel);
    proxyModel->setSourceModel(browserModel);
    proxyModel->setHide(true);
    browserView->setModel(proxyModel);
    browserCompleter->setModel(browserModel);

    vbox->addWidget(browserView);

    setRootDir(d);
    updateActions();

    browserView->setColumnWidth(0, 200); // This has to be as last (why?)
}

QWidget *CDirBrowser::createToolBar()
{
    QToolBar *toolBar = new QToolBar;

    // NOTE: Actions need to be added to main widget so that shortcuts
    // can be made widget-specific.
    
    // Top
    topTool = toolBar->addAction(QIcon(style()->standardIcon(QStyle::SP_FileDialogToParent)),
                                 "Top");
    connect(topTool, SIGNAL(triggered()), this, SLOT(topToolCB()));

    // Back
    backTool = toolBar->addAction(QIcon(style()->standardIcon(QStyle::SP_ArrowLeft)),
                                  "Back");
    backTool->setShortcut(tr("Alt+Left"));
    addAction(backTool);
    backTool->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(backTool, SIGNAL(triggered()), this, SLOT(backToolCB()));

    // Forward
    forwardTool = toolBar->addAction(QIcon(style()->standardIcon(QStyle::SP_ArrowRight)),
                                     "Forward");
    forwardTool->setShortcut(tr("Alt+Right"));
    forwardTool->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    addAction(forwardTool);
    connect(forwardTool, SIGNAL(triggered()), this, SLOT(forwardToolCB()));

    // Home
    homeTool = toolBar->addAction(QIcon(style()->standardIcon(QStyle::SP_DirHomeIcon)),
                                  "Home");
    connect(homeTool, SIGNAL(triggered()), this, SLOT(homeToolCB()));

    // New directory
    addDirTool = toolBar->addAction(QIcon(style()->standardIcon(QStyle::SP_FileDialogNewFolder)),
                                    "New directory");
    connect(addDirTool, SIGNAL(triggered()), this, SLOT(addDirToolCB()));

    toolBar->addSeparator();

    toolBar->addWidget(browserInput = new QLineEdit);
    browserInput->setCompleter(browserCompleter = new CDirCompleter(browserInput));
    
    return toolBar;
}

void CDirBrowser::createContextMenu(void)
{
    // UNDONE: Icons
    
    cutAction = new QAction("Cut", this);
    cutAction->setShortcut(tr("Ctrl+X"));
    connect(cutAction, SIGNAL(triggered()), this, SLOT(cutActionCB()));
    browserView->addAction(cutAction);

    copyAction = new QAction("Copy", this);
    copyAction->setShortcut(tr("Ctrl+C"));
    connect(copyAction, SIGNAL(triggered()), this, SLOT(copyActionCB()));
    browserView->addAction(copyAction);

    pasteAction = new QAction("Paste", this);
    pasteAction->setShortcut(tr("Ctrl+V"));
    connect(pasteAction, SIGNAL(triggered()), this, SLOT(pasteActionCB()));
    browserView->addAction(pasteAction);

    deleteAction = new QAction("Delete", this);
    deleteAction->setShortcut(tr("Delete"));
    connect(deleteAction, SIGNAL(triggered()), this, SLOT(deleteActionCB()));
    browserView->addAction(deleteAction);

    renameAction = new QAction("Rename", this);
    renameAction->setShortcut(tr("Rename"));
    connect(renameAction, SIGNAL(triggered()), this, SLOT(renameActionCB()));
    browserView->addAction(renameAction);

    QAction *sep = new QAction(this);
    sep->setSeparator(true);
    browserView->addAction(sep);

    viewHidden = new QAction("Show hidden files", this);
    viewHidden->setCheckable(true);
    connect(viewHidden, SIGNAL(toggled(bool)), this, SLOT(viewHiddenCB(bool)));
    browserView->addAction(viewHidden);
}

bool CDirBrowser::permissionsOK(const QString &path, bool onlyRead, bool checkParent)
{
    QFileInfo fi(path);
    if (fi.isDir())
    {
        if (!fi.isExecutable())
            return false;
        
        if (!onlyRead && checkParent)
        {
            QFileInfo dfi(fi.dir().path());
            if (!dfi.isWritable() || !dfi.isReadable())
                return false;
        }
    }
    
    return (onlyRead || fi.isWritable()) && fi.isReadable() && (!fi.isDir() || fi.isExecutable());
}

void CDirBrowser::topToolCB()
{
    if ((browserModel->rootPath() != rootDir) && (browserView->rootIndex().parent().isValid()))
        setRootIndex(browserView->rootIndex().parent());
}

void CDirBrowser::backToolCB()
{
    forwardStack.push(browserModel->rootPath());
    setRootIndex(proxyModel->mapFromSource(browserModel->index(backStack.pop())), false);
}

void CDirBrowser::forwardToolCB()
{
    backStack.push(browserModel->rootPath());
    setRootIndex(proxyModel->mapFromSource(browserModel->index(forwardStack.pop())), false);
}

void CDirBrowser::homeToolCB()
{
    if (browserModel->rootPath() != "/")
        browserView->setRootIndex(proxyModel->mapFromSource(browserModel->index("/")));
    browserView->setCurrentIndex(proxyModel->mapFromSource(browserModel->index(QDir::homePath())));
}

void CDirBrowser::addDirToolCB()
{
    QModelIndex par = browserModel->index(browserModel->rootPath());
    QModelIndex newDir = browserModel->mkdir(par, "New directory");
    if (newDir.isValid())
        browserView->edit(proxyModel->mapFromSource(newDir));
}

void CDirBrowser::setRootIndex(const QModelIndex &index, bool remember)
{
    if (!browserModel->isDir(proxyModel->mapToSource(index)))
        return;
    
    if (remember)
    {
        forwardStack.clear();
        backStack.push(browserModel->rootPath());
    }

    QString path = browserModel->filePath(proxyModel->mapToSource(index));
    browserModel->setRootPath(path);
    browserView->setRootIndex(index);
    browserInput->setText(toRelRoot(path, rootDir));

    topTool->setEnabled(browserModel->rootPath() != rootDir);
    backTool->setEnabled(!backStack.empty());
    forwardTool->setEnabled(!forwardStack.empty());
    addDirTool->setEnabled(permissionsOK(browserModel->rootPath(), false, false));
}

void CDirBrowser::copyActionCB()
{
    QMimeData *mime = proxyModel->mimeData(browserView->selectionModel()->selection().indexes());
    mime->setData("application/cut", "0");
    QApplication::clipboard()->setMimeData(mime);
}

void CDirBrowser::cutActionCB()
{
    QMimeData *mime = proxyModel->mimeData(browserView->selectionModel()->selection().indexes());
    mime->setData("nixstbuild/cut", "1");
    QApplication::clipboard()->setMimeData(mime);
}

void CDirBrowser::pasteActionCB()
{
    const QMimeData *mime = QApplication::clipboard()->mimeData();

    if (!mime || !mime->hasUrls())
        return;

    QModelIndexList sel = browserView->selectionModel()->selectedRows();
    QModelIndex dest;
    
    if (sel.size() > 1)
        return;
    else if (sel.empty())
        dest = proxyModel->mapToSource(browserView->rootIndex());
    else
    {
        QModelIndex index = proxyModel->mapToSource(sel.front());
        if (browserModel->isDir(index))
            dest = index;
        else
            dest = index.parent();
    }

    if (!dest.isValid())
        return;
    
    bool move = false;
    if (mime->hasFormat("nixstbuild/cut"))
    {
        QByteArray d = mime->data("nixstbuild/cut");
        if (d.at(0) == '1')
            move = true;
    }

    browserModel->copyFiles(mime, dest, move);
}

void CDirBrowser::deleteActionCB()
{
    QMessageBox box(QMessageBox::Question, "Delete file(s)",
                    "Are you sure you want to remove the selected file(s)?",
                    QMessageBox::Yes | QMessageBox::No);
    box.setDefaultButton(QMessageBox::No);
    QMimeData *mime = proxyModel->mimeData(browserView->selectionModel()->selection().indexes());

    QString ext = "The following will files or directories will be PERMANENTLY removed:\n\n";

    foreach(QUrl url, mime->urls())
    {
        QString f = url.toLocalFile();
        QFileInfo fi(f);
        ext += QString("%1 (%2)\n").arg(url.toLocalFile()).arg((fi.isDir() ? "directory" : "file"));
    }
    
    box.setDetailedText(ext);

    if (box.exec() == QMessageBox::Yes)
        browserModel->removeFiles(mime);
}

void CDirBrowser::renameActionCB()
{
    QModelIndexList sel = browserView->selectionModel()->selectedRows();
    
    if (sel.size() != 1)
        return;
    
    browserView->edit(sel.front());
}

void CDirBrowser::viewHiddenCB(bool e)
{
    proxyModel->setHide(!e);
}

void CDirBrowser::updateActions()
{
    QModelIndexList sel = browserView->selectionModel()->selectedRows();
    const int selSize = sel.size();
    const QMimeData *mime = QApplication::clipboard()->mimeData();
    bool hasClip = (mime && mime->hasUrls());
    
    // permissionsOK is used to check for what action to enable/disable.
    // Second bool arg should be true when path only has to be readable.
    // Third bool arg should be true when parent should be writable too
    // (required when modifying/removing a directory).
    
    if (!selSize)
    {
        QString path = browserModel->rootPath();
        cutAction->setEnabled(false);
        copyAction->setEnabled(false);
        pasteAction->setEnabled(hasClip && permissionsOK(path, false, false));
        deleteAction->setEnabled(false);
        renameAction->setEnabled(false);
    }
    else if (selSize == 1)
    {
        QString path = browserModel->filePath(proxyModel->mapToSource(sel.front()));
        bool permPOK = permissionsOK(path, false, true);

        cutAction->setEnabled(permPOK);
        copyAction->setEnabled(permissionsOK(path, true, false));

        if (QFileInfo(path).isDir())
            pasteAction->setEnabled(hasClip && permissionsOK(path, false, false));
        else if (sel.front().parent().isValid())
        {
            QString p = browserModel->filePath(proxyModel->mapToSource(sel.front().parent()));
            pasteAction->setEnabled(hasClip && permissionsOK(p, false, false));
        }
        else // Is this possible??
            pasteAction->setEnabled(false);
        
        deleteAction->setEnabled(permPOK);
        renameAction->setEnabled(permPOK);
    }
    else // Multiple items selected
    {
        bool permWOK = true, permROK = true;
        foreach(QModelIndex index, sel)
        {
            QString path = browserModel->filePath(proxyModel->mapToSource(index));
            permWOK = permWOK && permissionsOK(path, false, true);
            permROK = permROK && permissionsOK(path, false, false);
            if (!permWOK && !permROK)
                break;
        }

        cutAction->setEnabled(permWOK);
        copyAction->setEnabled(permROK);
        pasteAction->setEnabled(false);
        deleteAction->setEnabled(permWOK);
        renameAction->setEnabled(false);
    }
}

void CDirBrowser::setRootDir(const QString &dir)
{
    rootDir = dir;
    
    if (rootDir.isEmpty())
        rootDir = "/";

    backStack.clear();
    forwardStack.clear();
    setRootIndex(proxyModel->mapFromSource(browserModel->index(rootDir)), false);
    browserCompleter->setRootDir(dir);

    if (rootDir == "/")
    {
        homeTool->setVisible(true);
        homeToolCB(); // Select home directory
    }
    else
    {
        homeTool->setVisible(false);
    }
}


QString CDirCompleter::pathFromIndex(const QModelIndex &index) const
{
    // While QCompleter happily supports QDirModel, it doesn't do QFileSystemModel yet (4.5.1)
    // So here we do it similar like it does for QDirModel. Note that it's assumed that the model
    // consists of file paths

    if (!index.isValid() || !model())
        return QString();

    QModelIndex idx = index;
    QStringList list;
    do
    {
        QString t = model()->data(idx, Qt::EditRole).toString();
        list.prepend(t);
        QModelIndex parent = idx.parent();
        idx = parent.sibling(parent.row(), index.column());
    }
    while (idx.isValid());

    if (list.count() == 1)
        return list[0];
    
    list[0].clear();

    return toRelRoot(list.join(QDir::separator()), rootDir);
}

QStringList CDirCompleter::splitPath(const QString &path) const
{
    // Like pathFromIndex, we use a similar way to split paths as the regular
    // completer does for QDirModel

    if (path.isEmpty())
        return QStringList(completionPrefix());

    QString pathCopy = fromRelRoot(QDir::toNativeSeparators(path), rootDir);
    QString sep = QDir::separator();

    QRegExp re(QLatin1String("[") + QRegExp::escape(sep) + QLatin1String("]"));
    QStringList parts = pathCopy.split(re);

    if (pathCopy[0] == sep[0]) // readd the "/" at the beginning as the split removed it
        parts[0] = QDir::fromNativeSeparators(QString(sep[0]));

    return parts;
}


CDirView::CDirView(QWidget *parent) : QTreeView(parent)
{
    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setContextMenuPolicy(Qt::ActionsContextMenu);
    setSortingEnabled(true);
    sortByColumn(0, Qt::AscendingOrder);
}

void CDirView::mousePressEvent(QMouseEvent *event)
{
    QTreeView::mousePressEvent(event);
    emit mouseClicked();
}


bool CDirSortProxy::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

    return !hide || (sourceModel()->data(index).toString()[0] != '.');
}

bool CDirSortProxy::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    QFileSystemModel *model = qobject_cast<QFileSystemModel *>(sourceModel());
    QString lpath = model->filePath(left);
    QString rpath = model->filePath(right);
    QFileInfo li(lpath), ri(rpath);
    bool ldir = li.isDir();
    bool rdir = ri.isDir();
    
    // Directories are < files
    if (ldir && !rdir)
        return true;
    else if (!ldir && rdir)
        return false;
    
    // Both dirs or files
    return QSortFilterProxyModel::lessThan(left, right);
}


CDirModel::CDirModel(QObject *parent) : QFileSystemModel(parent),
                                        statUpdateTime(0), statWritten(0),
                                        statSizeFact(0), handleOverWrite(DO_ASK),
                                        multipleFiles(false)
{
    progressDialog = new QProgressDialog;
    progressDialog->setWindowModality(Qt::WindowModal);
}

int CDirModel::sizeUnitFact(qint64 size)
{
    int ret = 1;
    while (size > 1024)
    {
        size /= 1024;
        ret++;
    }
    return ret;
}

void CDirModel::safeCopy(const std::string &src, const std::string &dest, bool move)
{
    try
    {
        MKDirRec(dest);

        if (!move)
            CopyFile(src, dest, &copyWritten, this);
        else
            MoveFile(src, dest, &copyWritten, this);
    }
    catch(Exceptions::CExChMod &)
    {
        // Silently ignore those annoying permission changing errors ;-)
    }
    catch(Exceptions::CExIO &e)
    {
        QMessageBox::critical(0, "Error", QString("Failed to operate on file: ") + e.what());
    }
}

off_t CDirModel::safeFileSize(const std::string &file)
{
    try
    {
        return FileSize(file);
    }
    catch (Exceptions::CExIO &)
    {
    }
    return 0;
}

void CDirModel::safeMKDirRec(const std::string &dir)
{
    try
    {
        MKDirRec(dir);
    }
    catch(Exceptions::CExIO &e)
    {
        QMessageBox::critical(0, "Error",
                              QString("Failed to create directory %1: %2").arg(dir.c_str()).arg(e.what()));
    }
}

bool CDirModel::getAllSubPaths(const std::string &dir, TStringVec &paths)
{
    TStringVec searchdirs;
    searchdirs.push_back(".");

    while (!searchdirs.empty())
    {
        try
        {
            std::string sub = searchdirs.back();
            searchdirs.pop_back();

            std::string subpath = dir + "/" + sub;
            CDirIter dirIt(subpath);
            while (dirIt)
            {
                if (strcmp(dirIt->d_name, ".") && strcmp(dirIt->d_name, ".."))
                {
                    std::string fpath = subpath + "/" + dirIt->d_name;
                    std::string subf = sub + "/" + dirIt->d_name;
                    paths.push_back(subf);
                    if (IsDir(fpath))
                        searchdirs.push_back(subf);
                }
                dirIt++;
            }
        }
        catch(Exceptions::CExIO &e)
        {
            QMessageBox::critical(0, "Error", QString("Directory read error: ") + e.what());
            return false;
        }
    }
    
    return true;
}

bool CDirModel::askOverWrite(const QString &t, const QString &l)
{
    QMessageBox::StandardButtons buttons = (QMessageBox::Yes | QMessageBox::No);

    if (multipleFiles)
        buttons |= (QMessageBox::YesAll | QMessageBox::NoAll | QMessageBox::Cancel);
    
    QMessageBox::StandardButton ret = QMessageBox::question(0, t, l, buttons,
            QMessageBox::No);
            
    if (ret == QMessageBox::Yes)
        return true;
    else if (ret == QMessageBox::YesAll)
    {
        handleOverWrite = DO_ALL;
        return true;
    }
    else if (ret == QMessageBox::No)
        return false;
    else if (ret == QMessageBox::NoAll)
    {
        handleOverWrite = DO_NONE;
        return false;
    }
    
    // Cancel
    progressDialog->cancel();
    return false;
}

bool CDirModel::verifyExistance(const std::string &src, std::string dest)
{
    dest += "/" + BaseName(src);
    
    if (!FileExists(dest))
        return true; // Clear to go...

    if (IsDir(dest))
    {
        if (IsDir(src))
        {
            if (handleOverWrite != DO_ASK)
                return (handleOverWrite == DO_ALL);

            return askOverWrite("Destination exists",
                                "Destination directory exists.\nDo you want to overwrite it?");
        }
        else
        {
            QMessageBox::critical(0, "Error",
                QString("Destination '%1' exists as directory while source is a file").arg(dest.c_str()));
            return false;
        }
    }
    else if (IsDir(src))
    {
        QMessageBox::critical(0, "Error",
            QString("Destination '%1' exists as file while source is a directory.").arg(dest.c_str()));
        return false;
    }

    if (handleOverWrite != DO_ASK)
        return (handleOverWrite == DO_ALL);

    return askOverWrite("File exists",
                        QString("File '%1' already exists.\nOverwrite?").arg(dest.c_str()));
}

void CDirModel::RmDir(const std::string &dir)
{
    if (::rmdir(dir.c_str()) != 0)
    {
        QMessageBox::critical(0, "Error",
                              QString("Failed to remove directory %1").arg(dir.c_str()));
    }
}

void CDirModel::safeUnlink(const std::string &file)
{
    try
    {
        Unlink(file);
    }
    catch(Exceptions::CExUnlink &e)
    {
        QMessageBox::critical(0, "Error",
                              QString("Failed to remove file: %1").arg(e.what()));
    }
}

bool CDirModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int,
                             int, const QModelIndex &parent)
{
    if (!parent.isValid() || isReadOnly())
        return false;

    if (action == Qt::CopyAction)
    {
        copyFiles(data, parent, false);
        return true;
    }
    
    return false;
}

void CDirModel::copyFiles(const QMimeData *data, const QModelIndex &parent,
                          bool move)
{
    progressDialog->reset();
    progressDialog->setMinimumDuration(500);

    if (!move)
        progressDialog->setLabelText("Copying files...");
    else
        progressDialog->setLabelText("Moving files...");
    
    qint64 total = 0;
    std::map<std::string, TStringVec> pathsMap;
    TStringVec sources;
    std::string dest = filePath(parent).toStdString();

    multipleFiles = (data->urls().size() > 1);

    foreach(QUrl url, data->urls())
    {
        std::string f = url.toLocalFile().toStdString();

        if (!FileExists(f) || (DirName(f) == dest))
            continue;
        
        sources.push_back(f);
        
        if (IsDir(f))
        {
            multipleFiles = true;
            
            getAllSubPaths(f, pathsMap[f]);
            for (TStringVec::iterator it=pathsMap[f].begin(); it!=pathsMap[f].end(); it++)
            {
                std::string path = f + "/" + *it;
                if (!IsDir(path))
                    total += safeFileSize(path);
            }
        }
        else
            total += safeFileSize(f);
    }
    
    statSizeFact = sizeUnitFact(total);
    statWritten = 0;
    handleOverWrite = DO_ASK;

    int max = total / statSizeFact;
    progressDialog->setRange(0, max);

    for(TStringVec::iterator it=sources.begin(); it!=sources.end(); it++)
    {
        if (progressDialog->wasCanceled())
            break;

        if (!verifyExistance(*it, dest))
            continue;

        if (IsDir(*it))
        {
            std::string basedest = dest + "/" + BaseName(*it);
            TStringVec subDirs;

            for (TStringVec::iterator subit=pathsMap[*it].begin();
                 subit!=pathsMap[*it].end(); subit++)
            {
                if (progressDialog->wasCanceled())
                    break;

                std::string path = JoinPath(*it, *subit);
                if (!IsDir(path))
                {
                    std::string dirpath = JoinPath(basedest, DirName(*subit));
                    if (!verifyExistance(path, dirpath))
                        continue;

                    safeCopy(path, dirpath, move);
                }
                else
                    subDirs.insert(subDirs.begin(), *subit);
            }

            // Remove all directories (silently ignore any errors) and
            // add missing (empty dirs aren't created yet)
            for (TStringVec::iterator subit=subDirs.begin(); subit!=subDirs.end(); subit++)
            {
                if (move)
                {
                    std::string path = JoinPath(*it, *subit);
                    if (IsDir(path))
                        ::rmdir(path.c_str());
                }

                std::string dirpath = JoinPath(basedest, DirName(*subit));
                if (!FileExists(dirpath))
                    safeMKDirRec(dirpath);
            }

            if (move)
                ::rmdir(it->c_str());

            safeMKDirRec(basedest);
        }
        else
            safeCopy(*it, dest, move);
    }

    progressDialog->setValue(max);
}

void CDirModel::removeFiles(const QMimeData *data)
{
    progressDialog->reset();
    progressDialog->setMinimumDuration(500);
    progressDialog->setLabelText("Deleting files...");
    progressDialog->setRange(0, data->urls().size());

    int n = 0;

    foreach(QUrl url, data->urls())
    {
        if (progressDialog->wasCanceled())
            break;

        std::string file = url.toLocalFile().toStdString();
        if (IsDir(file))
        {
            TStringVec subFiles;
            TStringVec remDirs;
            getAllSubPaths(file, subFiles);

            for (TStringVec::iterator it=subFiles.begin(); it!=subFiles.end(); it++)
            {
                std::string subpath = JoinPath(file, *it);
                if (IsDir(subpath))
                    remDirs.insert(remDirs.begin(), subpath);
                else
                    safeUnlink(subpath);
            }

            // Remove all dirs after they are cleared
            for (TStringVec::iterator it=remDirs.begin(); it!=remDirs.end(); it++)
                RmDir(*it);

            RmDir(file);
        }
        else
            safeUnlink(file);

        n++;
        progressDialog->setValue(n);
    }
}

void CDirModel::copyWritten(int bytes, void *data)
{
    CDirModel *me = static_cast<CDirModel *>(data);
    me->statWritten += bytes;

    if (me->statUpdateTime < GetTime())
    {
        me->statUpdateTime = GetTime() + 50;
        me->progressDialog->setValue(me->statWritten / me->statSizeFact);
    }
}
