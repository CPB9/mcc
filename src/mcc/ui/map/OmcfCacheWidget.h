/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "mcc/ui/map/OmcfCache.h"

#include <QWidget>
#include <QDir>

class QProgressBar;

namespace mcc {
namespace ui {
namespace map {

class MercatorProjection;

class OmcfCacheWidget : public QWidget {
    Q_OBJECT
public:
    OmcfCacheWidget(QWidget* parent = 0);

signals:
    void progressChanged(int value);
    void finished();

private:
    void createCache(const QString& dirPath, const QString& outputPath, const QString& name, const QString& description,
                     const QByteArray& format, const MercatorProjection& proj);
    void getFiles(int baseLen, QDir& startDir, OrderedTilePosCache<QString>& tilePaths, const char* format);

    QProgressBar* _progressBar;
};
}
}
}
