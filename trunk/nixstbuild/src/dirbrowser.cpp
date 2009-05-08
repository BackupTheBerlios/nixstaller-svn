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
    browserView->setModel(browserModel);
    browserView->setDragEnabled(true);
    browserView->setAcceptDrops(true);
    browserView->setDropIndicatorShown(true);
    browserView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    browserView->setContextMenuPolicy(Qt::ActionsContextMenu);

    QAction *a = new QAction("Copy", this);
    a->setShortcut(tr("Ctrl+C"));
    connect(a, SIGNAL(triggered()), this, SLOT(copyAction()));
    browserView->addAction(a);

    a = new QAction("Paste", this);
    a->setShortcut(tr("Ctrl+V"));
    connect(a, SIGNAL(triggered()), this, SLOT(pasteAction()));
    browserView->addAction(a);

    vbox->addWidget(browserView);

    if (!d.isEmpty())
    {
        browserModel->setRootPath(d);
        browserView->setRootIndex(browserModel->index(d));
    }
    else
        browserModel->setRootPath("/");
}

void CDirBrowser::copyAction()
{
    QMimeData *mime = browserModel->mimeData(browserView->selectionModel()->selection().indexes());
    QApplication::clipboard()->setMimeData(mime);
}

void CDirBrowser::pasteAction()
{
    const QMimeData *mime = QApplication::clipboard()->mimeData();
    if (!mime || !mime->hasUrls())
        return;

    if (browserView->currentIndex().isValid())
    {
        const QModelIndex &par = browserView->currentIndex().parent();
        if (par.isValid())
            browserModel->copyFiles(mime, par);
    }
    else
        browserModel->copyFiles(mime, browserView->rootIndex());
}


CDirModel::CDirModel(QObject *parent) : QFileSystemModel(parent),
                                        statUpdateTime(0), statWritten(0),
                                        statSizeFact(0), handleOverWrite(DO_ASK)
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

void CDirModel::SafeCopy(const std::string &src, const std::string &dest)
{
    try
    {
        MKDirRec(DirName(dest));
        CopyFile(src, dest, &copyWritten, this);
    }
    catch(Exceptions::CExIO &e)
    {
        QMessageBox::critical(0, "Error", QString("Failed to copy file: ") + e.what());
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
            
            QMessageBox::StandardButton ret = QMessageBox::question(0, "Destination exists",
                "Destination directory exists.\nDo you want to overwrite it?",
                QMessageBox::Yes | QMessageBox::YesAll | QMessageBox::No | QMessageBox::NoAll,
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
    
    return QMessageBox::question(0, "File exists",
                                 QString("File '%1' already exists.\nOverwrite?").arg(dest.c_str()),
                                 QMessageBox::Yes | QMessageBox::No,
                                 QMessageBox::No) == QMessageBox::Yes;
}

bool CDirModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row,
                             int column, const QModelIndex &parent)
{
    if (!parent.isValid() || isReadOnly())
        return false;

    if (action == Qt::CopyAction)
    {
        copyFiles(data, parent);
        return true;
    }
    
    return false;
}

void CDirModel::copyFiles(const QMimeData *data, const QModelIndex &parent)
{
    progressDialog->setLabelText("Copying files...");
    progressDialog->setMinimumDuration(500);

    qint64 total = 0;
    QHash<QString, TStringVec> pathsMap; // Use QTL as STL has no standard hash map
    
    foreach(QUrl url, data->urls())
    {
        QString f(url.toLocalFile());
        QFileInfo fi(f);
        if (fi.isDir())
        {
            getAllSubPaths(f.toStdString(), pathsMap[f]);
            for (TStringVec::iterator it=pathsMap[f].begin(); it!=pathsMap[f].end(); it++)
            {
                QString path = f + "/" + it->c_str();
                QFileInfo subfi(path);
                if (!subfi.isDir())
                    total += subfi.size();
            }
        }
        else
            total += fi.size();
    }
    
    statSizeFact = sizeUnitFact(total);
    statWritten = 0;
    handleOverWrite = DO_ASK;

    int max = total / statSizeFact;
    progressDialog->setRange(0, max);

    std::string dest = filePath(parent).toStdString();
    foreach(QUrl url, data->urls())
    {
        if (progressDialog->wasCanceled())
            break;

        std::string src = url.toLocalFile().toStdString();

        if (!verifyExistance(src, dest))
            continue;

        if (IsDir(src))
        {
            std::string basedest = dest + "/" + BaseName(src);

            for (TStringVec::iterator it=pathsMap[src.c_str()].begin();
                 it!=pathsMap[src.c_str()].end(); it++)
            {
                if (progressDialog->wasCanceled())
                    break;

                std::string path = src + "/" + *it;
                if (!IsDir(path))
                {
                    std::string dirpath = basedest + "/" + DirName(*it);
                    if (!verifyExistance(path, dirpath))
                        continue;
                    SafeCopy(path, dirpath);
                }
            }
        }
        else
            SafeCopy(src, dest);
    }

    progressDialog->setValue(max);
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
