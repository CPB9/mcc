/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "mcc/ui/core/Structs.h"

#include <QWidget>
#include <QComboBox>
#include <QLineEdit>
#include <QAbstractItemView>


namespace mcc {
namespace ui {
namespace core {

class CheckableComboBox : public QComboBox {
    Q_OBJECT
public:
    CheckableComboBox(QWidget* parent = 0)
        : QComboBox(parent)
    {
        setEditable(true);
        lineEdit()->setReadOnly(true);
    }

    void hidePopup() override
    {
        QComboBox::hidePopup();
        updateText();
    }

    void updateText()
    {
        QString str;
        for (int i = 0; i < count(); ++i) {
            if (itemData(i, Qt::CheckStateRole) == Qt::Checked)
                str += itemText(i) + "; ";
        }

        lineEdit()->setText(str);
    }
};

class WaypointFlagEdit : public QWidget {
    Q_OBJECT
public:
    explicit WaypointFlagEdit(QWidget* parent = 0);
    explicit WaypointFlagEdit(WaypointFlags flags, QWidget* parent = 0);

    void setFlags(WaypointFlags flags);
    WaypointFlags flags() const;

private:
    std::vector<std::pair<QString, WaypointType>> _waypointTypes;
    CheckableComboBox* _editor;
};
}
}
}
