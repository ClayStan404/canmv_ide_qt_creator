// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include "fileiteratordevicesappender.h"

#include <QtCore/private/qfsfileengine_p.h>
#include <memory>

namespace Utils {
namespace Internal {

class RootInjectFSEngine : public QFSFileEngine
{
public:
    using QFSFileEngine::QFSFileEngine;

public:
    IteratorUniquePtr beginEntryList(const QString &path, QDirListing::IteratorFlags flags, const QStringList &filterNames) override
    {
        IteratorUniquePtr baseIterator = QFSFileEngine::beginEntryList(path, flags, filterNames);
        return std::make_unique<FileIteratorWrapper>(path, flags, filterNames, std::move(baseIterator));
    }
};

} // namespace Internal
} // namespace Utils
