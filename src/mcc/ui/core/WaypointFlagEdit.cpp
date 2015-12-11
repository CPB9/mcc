/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mcc/ui/core/WaypointFlagEdit.h"

#include <QStandardItemModel>

namespace mcc {
namespace ui {
namespace core {

WaypointFlagEdit::WaypointFlagEdit(QWidget* parent)
    : QWidget(parent)
{
    _waypointTypes.emplace_back("Нет", WaypointType::Normal);
    _waypointTypes.emplace_back("Дом", WaypointType::Home);
    _waypointTypes.emplace_back("Разворот", WaypointType::Turn);
    _waypointTypes.emplace_back("По высоте", WaypointType::ByAltitude);
    _waypointTypes.emplace_back("Цель", WaypointType::Target);
    _waypointTypes.emplace_back("Переход", WaypointType::SwitchRoute);
    _waypointTypes.emplace_back("Возврат", WaypointType::ReturnToRoute);
    _waypointTypes.emplace_back("Ожидание", WaypointType::Waiting);
    _waypointTypes.emplace_back("Интерполяция", WaypointType::Interpolation);
    _waypointTypes.emplace_back("По курсу", WaypointType::ByHeading);
    _editor = new CheckableComboBox(this);
    QStandardItemModel* model = new QStandardItemModel();
    for (const auto& pair : _waypointTypes) {
        QStandardItem* item = new QStandardItem();
        item->setText(pair.first);
        //item->setData(static_cast<uint>(pair.second), Qt::UserRole);
        item->setCheckable(true);
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
        model->appendRow(item);
    }
    _editor->setModel(model);
}

WaypointFlagEdit::WaypointFlagEdit(WaypointFlags flags, QWidget* parent)
    : WaypointFlagEdit(parent)
{
    setFlags(flags);
}

WaypointFlags WaypointFlagEdit::flags() const
{
    WaypointFlags flags;
    for (int i = 0; i < _editor->count(); ++i) {
        if (_editor->itemData(i, Qt::CheckStateRole) == Qt::Checked) {
            flags |= _waypointTypes[i].second;
        }
    }
    return flags;
}

void WaypointFlagEdit::setFlags(WaypointFlags flags)
{
    for (std::size_t i = 0; i < _waypointTypes.size(); i++) {
        WaypointType type = _waypointTypes[i].second;
        Qt::CheckState state = Qt::Unchecked;

        if (flags & type || (flags == 0 && type == WaypointType::Normal)) {
            state = Qt::Checked;
        }

        _editor->setItemData(i, state, Qt::CheckStateRole);
    }

    _editor->updateText();
}
}
}
}
