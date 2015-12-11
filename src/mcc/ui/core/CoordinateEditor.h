/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <QWidget>

class QLineEdit;

namespace mcc {
namespace ui {
namespace core {

class CoordinateValidator;

class CoordinateEditor : public QWidget {
    Q_OBJECT
public:
    CoordinateEditor(QWidget* parent = 0);

    double asValue() const;
    QString asText() const;

private:
    QLineEdit* _lineEdit;

};

}
}
}
