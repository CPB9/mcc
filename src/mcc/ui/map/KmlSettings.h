/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <QWidget>

namespace kmldom {
class Feature;
class Placemark;
class Document;
class Element;
}

namespace kmlengine {
class KmlFile;
}

class QPushButton;
class QTextEdit;
class QLineEdit;

namespace mcc {
namespace ui {
namespace map {

class StyleWidget;

class KmlSettings : public QWidget {
    Q_OBJECT
public:
    explicit KmlSettings(QWidget* parent = 0);
    void showSettings(kmldom::Document* parent, kmldom::Feature* feature);

signals:
    void elementChanged(kmldom::Document* parent, kmldom::Element* element);

private:
    void apply();
    void applyStyle();
    void showStyleWidgets(bool line, bool area, bool label, bool icon);
    QPushButton* _okButton;
    QPushButton* _cancelButton;
    QLineEdit* _nameEdit;
    QTextEdit* _descriptionEdit;
    StyleWidget* _lineStyleWidget;
    StyleWidget* _areaStyleWidget;
    StyleWidget* _labelStyleWidget;
    StyleWidget* _iconStyleWidget;
    kmldom::Document* _lastDoc;
    kmldom::Feature* _lastFeature;
};
}
}
}
