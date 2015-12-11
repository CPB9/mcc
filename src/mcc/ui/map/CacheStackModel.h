/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "mcc/ui/map/MercatorProjection.h"
#include "mcc/ui/map/StaticMapType.h"
#include "mcc/ui/map/StackCache.h"
#include "mcc/ui/map/OnlineCache.h"

#include <QAbstractTableModel>

namespace mcc {
namespace ui {
namespace map {

class CacheStackModel : public QAbstractTableModel {
    Q_OBJECT
public:
    CacheStackModel(QObject* parent = 0);
    ~CacheStackModel();

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    void selectOnlineMap(StaticMapType type);
    void setOnlineCachePath(const QString& path);

    void addOmcfCache(const QString& path);
    void removeAt(int index);
    bool canRemoveAt(int index);
    void moveUp(int index);
    void moveDown(int index);

    const OnlineCachePtr& onlineCache(StaticMapType type) const;
    const StackCachePtr& stack() const;

    StaticMapType currentStaticMap() const;
    MercatorProjection currentProjection() const;

signals:
    void stackChanged();

private:
    void updateEnabled();

    StackCachePtr _stack;
    MercatorProjection::ProjectionType _currentProj;
    StaticMapType _currentStaticMap;
    std::vector<OnlineCachePtr> _onlineCaches;
};

inline MercatorProjection CacheStackModel::currentProjection() const
{
    return _currentProj;
}

inline const StackCachePtr& CacheStackModel::stack() const
{
    return _stack;
}

inline const OnlineCachePtr& CacheStackModel::onlineCache(StaticMapType type) const
{
    std::size_t index = (std::size_t)type;
    return _onlineCaches[index];
}

inline StaticMapType CacheStackModel::currentStaticMap() const
{
    return _currentStaticMap;
}
}
}
}
