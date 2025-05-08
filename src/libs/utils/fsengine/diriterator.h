// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include "../filepath.h"
#include "../stringutils.h"

#include <QFileInfo>
#include <QString>

#include <QtCore/private/qabstractfileengine_p.h>

namespace Utils {
namespace Internal {

class DirIterator : public QAbstractFileEngineIterator
{
public:
    DirIterator(const QString &path, QDirListing::IteratorFlags filters, const QStringList &filterNames, FilePaths paths)
        : QAbstractFileEngineIterator(path, filters, filterNames)
        , m_filePaths(std::move(paths))
        , it(m_filePaths.begin())
    {
        it = m_filePaths.begin(); 
        m_currentPath = QString();
        if (!m_filePaths.isEmpty()) {
        }
    }

    // QAbstractFileEngineIterator interface
public:
    bool advance() override
    {
        if (it == m_filePaths.end())
            return false;
        if (!m_currentPath.isNull() || it != m_filePaths.begin()) { 
             ++it;
             if (it == m_filePaths.end()) {
                 m_currentPath = QString();
                 return false;
             }        
        } else if (it == m_filePaths.end()) {
             return false;
        }
        
        m_currentPath = chopIfEndsWith(it->toFSPathString(), '/');
        return true;
    }
    QString currentFilePath() const override
    {
        return m_currentPath;
    }

    QString currentFileName() const override
    {
        if (it == m_filePaths.end())
             return QString();
        const QString result = it->fileName();
        return chopIfEndsWith(result, '/');
    }

    QFileInfo currentFileInfo() const override
    {
        if (it == m_filePaths.end())
             return QFileInfo();
        return QFileInfo(m_currentPath);
    }

private:
    const FilePaths m_filePaths;
    FilePaths::const_iterator it;
    QString m_currentPath;
};

} // namespace Internal

} // namespace Utils
