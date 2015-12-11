/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mcc/ui/map/CacheStackModel.h"
#include "mcc/ui/map/OmcfCache.h"
#include "mcc/ui/map/OnlineCache.h"
#include "mcc/ui/core/Settings.h"

#include "bmcl/Buffer.h"
#include "bmcl/MemReader.h"
#include "bmcl/Logging.h"

#include <QColor>

namespace mcc {
namespace ui {
namespace map {

CacheStackModel::CacheStackModel(QObject* parent)
    : QAbstractTableModel(parent)
    , _currentProj(MercatorProjection::SphericalMercator)
{
    _stack = StackCache::create();
    _onlineCaches.push_back(OnlineCache::create(StaticMapType::GoogleMap));
    _onlineCaches.push_back(OnlineCache::create(StaticMapType::GoogleLandscape));
    _onlineCaches.push_back(OnlineCache::create(StaticMapType::GoogleSatellite));
    _onlineCaches.push_back(OnlineCache::create(StaticMapType::YandexMap));
    _onlineCaches.push_back(OnlineCache::create(StaticMapType::YandexSatellite));
    _onlineCaches.push_back(OnlineCache::create(StaticMapType::YandexNarodnaya));
    _onlineCaches.push_back(OnlineCache::create(StaticMapType::OsmBasic));
    _onlineCaches.push_back(OnlineCache::create(StaticMapType::OsmMapQuest));
    _onlineCaches.push_back(OnlineCache::create(StaticMapType::OsmMapQuestSat));
    _onlineCaches.push_back(OnlineCache::create(StaticMapType::OsmThunderforestLandscape));

    QByteArray state = core::Settings::instance()->mapStackState();
    if (state.isEmpty() || state.size() < 3) {
        _stack->setProjection(_currentProj);
        for (const OnlineCachePtr& cache : _onlineCaches) {
            _stack->append(cache);
        }
    } else {
        bmcl::MemReader reader(state.data(), state.size());
        _currentProj = (MercatorProjection::ProjectionType)reader.readUint8();
        _currentStaticMap = (StaticMapType)reader.readUint8();
        while (!reader.isEmpty()) {
            if (reader.readableSize() < 3) {
                break;
            }
            uint type = reader.readUint8();
            bool isEnabled = reader.readUint8();
            if (type == 0) {
                StaticMapType type = (StaticMapType)reader.readUint8();
                auto cache = OnlineCache::create(type);
                _stack->append(cache);
                _stack->setEnabled(_stack->size() - 1, isEnabled);
            } else {
                if (reader.readableSize() < 8) {
                    break;
                }

                uint64_t size = reader.readUint64Le();
                if (reader.readableSize() < size) {
                    break;
                }

                QString path = QString::fromUtf8((char*)reader.current(), size);
                reader.skip(size);
                misc::Result<OmcfCachePtr, OmcfCache::Result> rv = OmcfCache::create(path);
                if (rv.isOk()) {
                    _stack->append(rv.unwrap());
                }
            }
        }
        for (const OnlineCachePtr& onlineCache : _onlineCaches) {
            if (!_stack->hasOnlineCache(onlineCache->type())) {
                _stack->append(onlineCache);
            }
        }
    }
    updateEnabled();
    emit stackChanged();
}

CacheStackModel::~CacheStackModel()
{
    bmcl::Buffer state;
    state.writeUint8((uint8_t)_currentProj);
    state.writeUint8((uint8_t)_currentStaticMap);
    for (std::size_t i = 0; i < _stack->size(); i++) {
        const FileCache* ptr = _stack->at(i).get();
        if (const OnlineCache* onlineCache = dynamic_cast<const OnlineCache*>(ptr)) {
            state.writeUint8(0);
            state.writeUint8(_stack->isEnabled(i));
            state.writeUint8((uint8_t)onlineCache->type());
        } else if (const OmcfCache* omcfCache = dynamic_cast<const OmcfCache*>(ptr)) {
            state.writeUint8(1);
            state.writeUint8(_stack->isEnabled(i));
            QByteArray path = omcfCache->path().toUtf8();
            state.writeUint64Le(path.size());
            state.write(path.data(), path.size());
        } else {
            assert(false);
        }
    }
    core::Settings::instance()->setMapStackState(QByteArray((char*)state.start(), state.size()));
}

void CacheStackModel::setOnlineCachePath(const QString& path)
{
    for (const OnlineCachePtr& cache : _onlineCaches) {
        cache->setBasePath(path);
    }
}

void CacheStackModel::selectOnlineMap(StaticMapType type)
{
    beginResetModel();
    _currentStaticMap = type;
    endResetModel();
}

void CacheStackModel::addOmcfCache(const QString& path)
{
    auto newCache = OmcfCache::create(path);
    if (newCache.isErr()) {
        return;
    }

    beginInsertRows(QModelIndex(), _stack->size(), _stack->size());
    _stack->append(newCache.unwrap());
    bool isEnabled = newCache.unwrap()->projection().isA(_currentProj);
    _stack->setEnabled(_stack->size() - 1, isEnabled);
    endInsertRows();
}

void CacheStackModel::removeAt(int index)
{
    beginRemoveRows(QModelIndex(), index, index);
    _stack->removeAt(index);
    endRemoveRows();
}

bool CacheStackModel::canRemoveAt(int index)
{
    return !_stack->at(index)->isBuiltIn();
}

void CacheStackModel::moveUp(int index)
{
    beginMoveRows(QModelIndex(), index, index, QModelIndex(), index - 1);
    _stack->swap(index, index - 1);
    endMoveRows();
}

void CacheStackModel::moveDown(int index)
{
    beginMoveRows(QModelIndex(), index + 1, index + 1, QModelIndex(), index);
    _stack->swap(index, index + 1);
    endMoveRows();
}

int CacheStackModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return _stack->size();
}

int CacheStackModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 4;
}

QVariant CacheStackModel::data(const QModelIndex& index, int role) const
{
    if (role == Qt::CheckStateRole && index.column() == 0) {
        if (_stack->isEnabled(index.row())) {
            return Qt::Checked;
        } else {
            return Qt::Unchecked;
        }
    }

    auto cache = _stack->at(index.row());
    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case 0:
            return cache->name();
        case 1:
            return cache->description();
        case 2:
            if (cache->isBuiltIn()) {
                return "Да";
            } else {
                return "Нет";
            }
        case 3:
            if (cache->projection().isA(MercatorProjection::SphericalMercator)) {
                return "Сферический меркатор";
            } else if (cache->projection().isA(MercatorProjection::EllipticalMercator)) {
                return "Эллиптический меркатор";
            } else {
            }
        }
    } else if (role == Qt::BackgroundColorRole) {
        if (index.column() == 0 && cache->isBuiltIn()) {
            auto onlineCache = static_cast<const OnlineCache*>(cache.get());
            if (onlineCache->type() == _currentStaticMap) {
                return QColor(qRgb(251, 128, 114));
            }
        }
        if (index.column() == 2) {
            if (cache->isBuiltIn()) {
                return QColor(qRgb(230, 245, 201));
            } else {
                return QColor(qRgb(253, 205, 172));
            }
        }
        if (index.column() == 3) {
            if (cache->projection().isA(MercatorProjection::SphericalMercator)) {
                return QColor(qRgb(179, 226, 205));
            } else if (cache->projection().isA(MercatorProjection::EllipticalMercator)) {
                return QColor(qRgb(203, 213, 232));
            }
        }
    }

    return QVariant();
}

QVariant CacheStackModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    if (orientation == Qt::Vertical) {
        return section + 1;
    } else {
        switch (section) {
        case 0:
            return "Название";
        case 1:
            return "Описание";
        case 2:
            return "Встроенная";
        case 3:
            return "Проекция";
        }
    }

    return QVariant();
}

Qt::ItemFlags CacheStackModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    if (index.column() == 0) {
        flags |= Qt::ItemIsUserCheckable;
    }

    return flags;
}

void CacheStackModel::updateEnabled()
{
    for (std::size_t i = 0; i < _stack->size(); i++) {
        if (!_stack->at(i)->projection().isA(_currentProj)) {
            _stack->setEnabled(i, false);
            QModelIndex newIndex = createIndex(i, 0);
            emit dataChanged(newIndex, newIndex);
        }
    }
}

bool CacheStackModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (role == Qt::CheckStateRole && index.column() == 0) {
        _stack->setEnabled(index.row(), value.toBool());
        if (_stack->at(index.row())->projection().isA(MercatorProjection::SphericalMercator)) {
            _currentProj = MercatorProjection::SphericalMercator;
        } else {
            _currentProj = MercatorProjection::EllipticalMercator;
        }
        _stack->setProjection(_currentProj);
        updateEnabled();
        emit stackChanged();
        return true;
    }

    return false;
}
}
}
}

