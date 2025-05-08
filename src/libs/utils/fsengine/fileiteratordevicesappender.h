// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include "../filepath.h"
#include "../algorithm.h"
#include "fsengine.h"
#include "diriterator.h"

#include <QtCore/private/qabstractfileengine_p.h>
#include <memory>
#include <QDirListing>

namespace Utils {
namespace Internal {

class FileIteratorWrapper : public QAbstractFileEngineIterator
{
    enum class State {
        IteratingBase,
        IteratingDevices,
        Ended,
    };

public:
    FileIteratorWrapper(const QString &path, QDirListing::IteratorFlags filters, const QStringList &filterNames, std::unique_ptr<QAbstractFileEngineIterator> &&baseIterator)
        : QAbstractFileEngineIterator(path, filters, filterNames)
        , m_baseIterator(std::move(baseIterator))
        , m_deviceIterator(nullptr)
    {
        if (path.compare(QDir::rootPath(), Qt::CaseInsensitive) == 0) {
             m_status = State::IteratingBase;
             m_rootPath = path;
        } else {
             m_status = State::IteratingBase;
        }
    }

public:
    bool advance() override
    {
        while (true) {
            switch (m_status) {
            case State::IteratingBase:
                if (m_baseIterator && m_baseIterator->advance()) {
                     m_currentPath = m_baseIterator->currentFilePath();
                     return true;
                }
                if (!m_rootPath.isEmpty()) {
                    m_status = State::IteratingDevices;
                    FilePaths devicePaths = Utils::transform(FSEngine::registeredDeviceSchemes(),
                                                            [this](const QString &scheme) {
                                                                return FilePath::fromString(m_rootPath).pathAppended(scheme);
                                                            });
                    m_deviceIterator = std::make_unique<DirIterator>(m_rootPath, QDirListing::IteratorFlags(), QStringList(), std::move(devicePaths));
                } else {
                    m_status = State::Ended;
                    m_currentPath = QString();
                    return false;
                }
                break;

            case State::IteratingDevices:
                if (m_deviceIterator && m_deviceIterator->advance()) {
                    m_currentPath = m_deviceIterator->currentFilePath();
                    return true;
                }
                m_status = State::Ended;
                m_currentPath = QString();
                return false;

            case State::Ended:
            default:
                 m_currentPath = QString();
                 return false;
            }
        }
    }

    QString currentFilePath() const override
    {
        return m_currentPath;
    }

    QString currentFileName() const override
    {
        if (m_status == State::IteratingBase && m_baseIterator) {
            return m_baseIterator->currentFileName();
        } else if (m_status == State::IteratingDevices && m_deviceIterator) {
            return m_deviceIterator->currentFileName();
        }
        return FilePath::fromString(m_currentPath).fileName();
    }

    QFileInfo currentFileInfo() const override
    {
         if (m_status == State::IteratingBase && m_baseIterator) {
            return m_baseIterator->currentFileInfo();
        } else if (m_status == State::IteratingDevices && m_deviceIterator) {
            return m_deviceIterator->currentFileInfo();
        }
        return QFileInfo(m_currentPath);
    }

private:
    std::unique_ptr<QAbstractFileEngineIterator> m_baseIterator;
    std::unique_ptr<DirIterator> m_deviceIterator;
    QString m_rootPath;
    QString m_currentPath;
    mutable State m_status{State::IteratingBase};
};

} // namespace Internal
} // namespace Utils
