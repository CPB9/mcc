/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "mcc/ui/core/Structs.h"
#include "mcc/ui/core/CoordinatePrinter.h"

#include <QLineEdit>
#include <QValidator>

namespace mcc {
namespace ui {
namespace core {

class LatLonEdit : public QLineEdit {
    Q_OBJECT
public:
    LatLonEdit(QWidget* parent);
    virtual ~LatLonEdit();

    virtual void setLatLon(const LatLon& latLon);
    const LatLon& latLon() const;

signals:
    void latLonChanged(const LatLon& latLon);

protected:
    CoordinatePrinter _printer;
    LatLon _latLon;
    QValidator* _validator;

    virtual void setPrinter(const CoordinatePrinter& printer);
};

class LatEdit : public LatLonEdit {
public:
    LatEdit(QWidget* parent = 0);

protected:
    virtual void setPrinter(const CoordinatePrinter& printer) override;
    virtual void setLatLon(const LatLon& latLon) override;
    void updateValue();
};

class LonEdit : public LatLonEdit {
public:
    LonEdit(QWidget* parent = 0);

protected:
    virtual void setPrinter(const CoordinatePrinter& printer) override;
    virtual void setLatLon(const LatLon& latLon) override;
    void updateValue();
};
}
}
}
