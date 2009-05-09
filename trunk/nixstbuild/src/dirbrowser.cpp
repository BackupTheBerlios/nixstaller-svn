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
#include <QMessageBox>
#include <QMimeData>
#include <QMouseEvent>
#include <QProgressDialog>
#include <QTreeView>
#include <QUrl>
#include <QVBoxLayout>

#include "dirbrowser.h"

CDirBrowser::CDirBrowser(const QString &d, QWidget *parent,
                         Qt::WindowFlags flags) : QWidget(parent, flags)
{
    QVBoxLayout *vbox = new QVBoxLayout(this);

    browserModel = new CDirModel;
    browserModel->setReadOnly(false);

    browserView = new QTreeView;
    browserView->setDragEnabled(true);
    browserView->setAcceptDrops(true);
    browserView->setDropIndicatorShown(true);
    browserView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    browserView->setContextMenuPolicy(Qt::ActionsContextMenu);
    browserView->setSortingEnabled(true);
    browserView->sortByColumn(0, Qt::AscendingOrder);

    QAction *a = new QAction("Cut", this);
    a->setShortcut(tr("Ctrl+X"));
    connect(a, SIGNAL(triggered()), this, SLOT(cutAction()));
    browserView->addAction(a);

    a = new QAction("Copy", this);
    a->setShortcut(tr("Ctrl+C"));
    connect(a, SIGNAL(triggered()), this, SLOT(copyAction()));
    browserView->addAction(a);

    a = new QAction("Paste", this);
    a->setShortcut(tr("Ctrl+V"));
    connect(a, SIGNAL(triggered()), this, SLOT(pasteAction()));
    browserView->addAction(a);

    proxyModel = new CDirSortProxy(browserModel);
    proxyModel->setSourceModel(browserModel);
    browserView->setModel(proxyModel);

    vbox->addWidget(browserView);

    if (!d.isEmpty())
    {
        browserModel->setRootPath(d);
        browserView->setRootIndex(proxyModel->mapFromSource(browserModel->index(d)));
    }
    else
        browserModel->setRootPath("/");

    browserView->setColumnWidth(0, 200); // This has to be as last (why?)
}

void CDirBrowser::copyAction()
{
    QMimeData *mime = proxyModel->mimeData(browserView->selectionModel()->selection().indexes());
    mime->setData("application/cut", "0");
    QApplication::clipboard()->setMimeData(mime);
}

void CDirBrowser::cutAction()
{
    QMimeData *mime = proxyModel->mimeData(browserView->selectionModel()->selection().indexes());
    mime->setData("nixstbuild/cut", "1");
    QApplication::clipboard()->setMimeData(mime);
}

void CDirBrowser::pasteAction()
{
    const QMimeData *mime = QApplication::clipboard()->mimeData();

    if (!mime || !mime->hasUrls())
        return;

    CDirModel::optype ot = CDirModel::COPY;

    if (mime->hasFormat("nixstbuild/cut"))
    {
        QByteArray d = mime->data("nixstbuild/cut");
        if (d.at(0) == '1')
            ot = CDirModel::MOVE;
    }
    
    if (browserView->currentIndex().isValid())
    {
        QModelIndex par = proxyModel->mapToSource(browserView->currentIndex()).parent();
        if (par.isValid())
            browserModel->operateOnFiles(mime, par, ot);
    }
    else
        browserModel->operateOnFiles(mime, proxyModel->mapToSource(browserView->rootIndex()), ot);
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

void CDirModel::safeFileOp(const std::string &src, const std::string &dest, optype opType)
{
    try
    {
        MKDirRec(dest);

        if (opType == COPY)
            CopyFile(src, dest, &opWritten, this);
        else // Move
            MoveFile(src, dest, &opWritten, this);
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

bool CDirModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row,
                             int column, const QModelIndex &parent)
{
    if (!parent.isValid() || isReadOnly())
        return false;

    if (action == Qt::CopyAction)
    {
        operateOnFiles(data, parent, COPY);
        return true;
    }
    
    return false;
}

void CDirModel::operateOnFiles(const QMimeData *data, const QModelIndex &parent,
                               optype opType)
{
    progressDialog->reset();
    progressDialog->setLabelText("Copying files...");
    progressDialog->setMinimumDuration(500);

    qint64 total = 0;
    std::map<std::string, TStringVec> pathsMap;
    TStringVec sources;

    multipleFiles = (data->urls().size() > 1);

    std::string dest = filePath(parent).toStdString();

    foreach(QUrl url, data->urls())
    {
        std::string f = url.toLocalFile().toStdString();

        if (DirName(f) == dest)
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
                    total += FileSize(path);
            }
        }
        else
            total += FileSize(f);
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

                    safeFileOp(path, dirpath, opType);
                }
            }

            if ((opType == COPY) || (opType == MOVE))
            {
                // Remove all directories (silently ignore any errors) and
                // add missing (empty dirs aren't created yet)
                for (TStringVec::iterator subit=pathsMap[*it].begin();
                     subit!=pathsMap[*it].end(); subit++)
                {
                    if (opType == MOVE)
                    {
                        std::string path = JoinPath(*it, *subit);
                        if (IsDir(path))
                            ::rmdir(path.c_str());
                    }

                    std::string dirpath = JoinPath(basedest, DirName(*subit));
                    qDebug("dirpath: %s(%d)", dirpath.c_str(), FileExists(dirpath));
                    if (!FileExists(dirpath))
                        safeMKDirRec(dirpath);
                }

                if (opType == MOVE)
                    ::rmdir(it->c_str());

                safeMKDirRec(basedest);
            }
        }
        else
            safeFileOp(*it, dest, opType);
    }

    progressDialog->setValue(max);
}

void CDirModel::opWritten(int bytes, void *data)
{
    CDirModel *me = static_cast<CDirModel *>(data);
    me->statWritten += bytes;

    if (me->statUpdateTime < GetTime())
    {
        me->statUpdateTime = GetTime() + 50;
        me->progressDialog->setValue(me->statWritten / me->statSizeFact);
    }
}
