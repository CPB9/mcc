/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "mcc/ui/core/CoordinatePrinter.h"

#include <QObject>

#include <utility>

namespace mcc {
namespace ui {
namespace core {

class CoordinatePrinter;

class GlobalCoordinatePrinter : public QObject {
    Q_OBJECT
public:
    static GlobalCoordinatePrinter* instance();

    const CoordinatePrinter& printer() const;

    GlobalCoordinatePrinter(const GlobalCoordinatePrinter& other) = delete;
    GlobalCoordinatePrinter(GlobalCoordinatePrinter&& other) = delete;
    GlobalCoordinatePrinter& operator=(const GlobalCoordinatePrinter& other) = delete;
    GlobalCoordinatePrinter& operator=(GlobalCoordinatePrinter&& other) = delete;

    void setTargetSystem(CoordinateSystem system);
    void setFormat(CoordinateFormat format);

    CoordinateSystem targetSystem() const;
    CoordinateFormat format() const;
signals:
    void printerChanged();

private:
    GlobalCoordinatePrinter();

    CoordinatePrinter _printer;
};

}
}
}
