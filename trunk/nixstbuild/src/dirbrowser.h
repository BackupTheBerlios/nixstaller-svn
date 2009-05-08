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

#ifndef DIRBROWSER_H
#define DIRBROWSER_H

#include <QFileSystemModel>
#include <QWidget>

#include "main/main.h"

class QProgressDialog;
class QTreeView;

class CDirModel;

class CDirBrowser: public QWidget
{
    Q_OBJECT
    
    CDirModel *browserModel;
    QTreeView *browserView;

private slots:
    void copyAction(void);
    void pasteAction(void);
    
public:
    CDirBrowser(const QString &d = QString(), QWidget *parent = 0, Qt::WindowFlags flags = 0);
};

class CDirModel: public QFileSystemModel
{
    QProgressDialog *progressDialog;
    long statUpdateTime;
    int statWritten, statSizeFact;

    enum asktype { DO_ALL, DO_NONE, DO_ASK };
    asktype handleOverWrite;
    
    int sizeUnitFact(qint64 size);
    void SafeCopy(const std::string &src, const std::string &dest);
    bool getAllSubPaths(const std::string &dir, TStringVec &paths);
    bool verifyExistance(const std::string &src, std::string dest);

public:
    CDirModel(QObject *parent = 0);

    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row,
                                int column, const QModelIndex &parent);

    void copyFiles(const QMimeData *data, const QModelIndex &parent);

    static void copyWritten(int bytes, void *data);
};

#endif
