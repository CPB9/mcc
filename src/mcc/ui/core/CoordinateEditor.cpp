/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mcc/ui/core/CoordinateEditor.h"
#include "mcc/ui/core/CoordinatePrinter.h"
#include "mcc/ui/core/GlobalCoordinatePrinter.h"

#include <QValidator>

namespace mcc {
namespace ui {
namespace core {

class CoordinateValidator : public QValidator {
public:
    explicit CoordinateValidator(const CoordinatePrinter& printer)
    {
        _printer = printer;
    }

    virtual State validate(QString& str, int& pos) const
    {
        (void)str;
        (void)pos;
        return QValidator::Acceptable;
    }
private:
    CoordinatePrinter _printer;
};

CoordinateEditor::CoordinateEditor(QWidget* parent)
    : QWidget(parent)
{
}

QString CoordinateEditor::asText() const
{
    return QString();
}

double CoordinateEditor::asValue() const
{
    return 0;
}
}
}
}
