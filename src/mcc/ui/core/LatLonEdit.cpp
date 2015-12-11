/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mcc/ui/core/LatLonEdit.h"
#include "mcc/ui/core/GlobalCoordinatePrinter.h"

#include <QDebug>
#include <QApplication>

namespace mcc { namespace ui { namespace core {

LatLonEdit::LatLonEdit(QWidget* parent)
    : QLineEdit(parent)
    , _validator(0)
{
    setPrinter(GlobalCoordinatePrinter::instance()->printer());
    connect(GlobalCoordinatePrinter::instance(), &GlobalCoordinatePrinter::printerChanged, this,
            [this]() { setPrinter(GlobalCoordinatePrinter::instance()->printer()); });
}

LatLonEdit::~LatLonEdit()
{
    if (_validator) {
        delete _validator;
    }
}

void LatLonEdit::setPrinter(const CoordinatePrinter &printer)
{
    _printer = printer;
}

void LatLonEdit::setLatLon(const LatLon& latLon)
{
    _latLon = latLon;
    emit latLonChanged(_latLon);
}

const LatLon& LatLonEdit::latLon() const
{
    return _latLon;
}

LatEdit::LatEdit(QWidget* parent)
    : LatLonEdit(parent)
{
    connect(this, &QLineEdit::editingFinished, this, [this]() {
        misc::Option<double> lat(_printer.parse(text()));
        if (lat.isSome())
        {
            setLatLon(LatLon(_printer.toWgs84Lat(lat.unwrap(), _latLon.longitude), _latLon.longitude));
        }
    });
    connect(this, &QLineEdit::textChanged, this, [this]() {
        if (!hasAcceptableInput())
        {
            QPalette *palette = new QPalette(Qt::red);
            palette->setColor(QPalette::Highlight, Qt::red);
            setPalette(*palette);
        }
        else
        {
            setPalette(QApplication::palette());
        }
    });
}

void LatEdit::setPrinter(const CoordinatePrinter &printer)
{
    LatLonEdit::setPrinter(printer);
    if (_validator) {
        delete _validator;
    }
    _validator = _printer.latitudeValidator();
    setValidator(_validator);
    //setInputMask(_printer.latitudeInputMask());
    updateValue();
}

void LatEdit::setLatLon(const LatLon &latLon)
{
    LatLonEdit::setLatLon(latLon);
    updateValue();
}

void LatEdit::updateValue()
{
    setText(_printer.printLat(_latLon.latitude, _latLon.longitude));
}

LonEdit::LonEdit(QWidget* parent)
    : LatLonEdit(parent)
{
    connect(this, &QLineEdit::editingFinished, this, [this]() {
        misc::Option<double> lon(_printer.parse(text()));
        if (lon.isSome())
        {
            setLatLon(LatLon(_latLon.latitude, _printer.toWgs84Lon(_latLon.latitude, lon.unwrap())));
        }
    });
    connect(this, &QLineEdit::textChanged, this, [this]() {
        if (!hasAcceptableInput())
        {
            QPalette *palette = new QPalette(Qt::red);
            palette->setColor(QPalette::Highlight, Qt::red);
            setPalette(*palette);
        }
        else
        {
            setPalette(QApplication::palette());
        }
    });
}

void LonEdit::setPrinter(const CoordinatePrinter &printer)
{
    LatLonEdit::setPrinter(printer);
    if (_validator) {
        delete _validator;
    }
    _validator = _printer.longitudeValidator();
    setValidator(_validator);
    //setInputMask(_printer.longitudeInputMask());
    updateValue();
}

void LonEdit::updateValue()
{
    setText(_printer.printLon(_latLon.latitude, _latLon.longitude));
}

void LonEdit::setLatLon(const LatLon &latLon)
{
    LatLonEdit::setLatLon(latLon);
    updateValue();
}

}}}
