/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <QWidget>

class QPushButton;
class QSlider;

namespace mcc {
namespace ui {
namespace map {

class MapSlider : public QWidget {
    Q_OBJECT
public:
    MapSlider(QWidget* parent);

    void setMaximum(int value);
    void setMinimum(int value);
    void setValue(int value);
    int value() const;

signals:
    void valueChanged(int value);

private:
    void onSliderValueChanged(int value);

    QSlider* _slider;
    QPushButton* _zoomInButton;
    QPushButton* _zoomOutButton;
    bool _isChangingValue;
};
}
}
}
