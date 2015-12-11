/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mcc/ui/core/GlobalCoordinatePrinter.h"

namespace mcc {
namespace ui {
namespace core {

GlobalCoordinatePrinter::GlobalCoordinatePrinter()
    : QObject()
{
    _printer.setFormat(CoordinateFormat::DegreesMinutesSeconds);
}

GlobalCoordinatePrinter* GlobalCoordinatePrinter::instance()
{
    static GlobalCoordinatePrinter printer;
    return &printer;
}

const CoordinatePrinter& GlobalCoordinatePrinter::printer() const
{
    return _printer;
}

void GlobalCoordinatePrinter::setFormat(CoordinateFormat format)
{
    _printer.setFormat(format);
    emit printerChanged();
}

void GlobalCoordinatePrinter::setTargetSystem(CoordinateSystem system)
{
    _printer.setTargetSystem(system);
    emit printerChanged();
}

CoordinateSystem GlobalCoordinatePrinter::targetSystem() const
{
    return _printer.system();
}

CoordinateFormat GlobalCoordinatePrinter::format() const
{
    return _printer.format();
}

}
}
}
