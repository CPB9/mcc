/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mcc/ui/map/KmlSettings.h"
#include "mcc/ui/map/KmlStyleWidget.h"
#include "mcc/ui/map/KmlUtils.h"

#include <kml/dom.h>
#include <kml/engine.h>

#include <QPushButton>
#include <QTabWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QDebug>

namespace mcc {
namespace ui {
namespace map {

KmlSettings::KmlSettings(QWidget* parent)
    : QWidget(parent)
{
    setWindowTitle("Параметры элемента Kml");
    _okButton = new QPushButton("Ok");
    _cancelButton = new QPushButton("Отмена");
    _descriptionEdit = new QTextEdit;
    _nameEdit = new QLineEdit;

    QGroupBox* nameWidget = new QGroupBox("Название");
    QHBoxLayout* nameLayout = new QHBoxLayout;
    nameLayout->addWidget(_nameEdit);
    nameWidget->setLayout(nameLayout);

    QGroupBox* descriptionWidget = new QGroupBox("Описание");
    QVBoxLayout* descriptionLayout = new QVBoxLayout;
    descriptionLayout->addWidget(_descriptionEdit);
    descriptionWidget->setLayout(descriptionLayout);

    QVBoxLayout* styleLayout = new QVBoxLayout;
    _lineStyleWidget = StyleWidget::createLineWidget();
    _areaStyleWidget = StyleWidget::createFillWidget();
    _labelStyleWidget = StyleWidget::createLabelWidget();
    _iconStyleWidget = StyleWidget::createIconWidget();
    styleLayout->addWidget(_lineStyleWidget);
    styleLayout->addWidget(_areaStyleWidget);
    styleLayout->addWidget(_labelStyleWidget);
    styleLayout->addWidget(_iconStyleWidget);
    styleLayout->addStretch();

    QHBoxLayout* buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    buttonLayout->addWidget(_okButton);
    buttonLayout->addWidget(_cancelButton);

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(nameWidget);
    mainLayout->addWidget(descriptionWidget);
    mainLayout->addLayout(styleLayout);
    mainLayout->addLayout(buttonLayout);
    setLayout(mainLayout);
    connect(_okButton, &QPushButton::clicked, this, [this]() {
        apply();
        applyStyle();
        close();
    });
    connect(_cancelButton, &QPushButton::clicked, this, [this]() { close(); });
}

void KmlSettings::apply()
{
    bool changed = false;
    std::string name = _nameEdit->text().toStdString();
    if (_lastFeature->get_name() != name) {
        _lastFeature->set_name(name);
        changed = true;
    }
    std::string description = _descriptionEdit->toPlainText().toStdString();
    if (_lastFeature->get_description() != description) {
        _lastFeature->set_description(description);
        changed = true;
    }
    kmldom::StylePtr style = KmlUtils::resolveStyle(_lastDoc, _lastFeature);
    if (!style) {
        // style = kmldom::KmlFactory::GetFactory()->CreateStyle();
        // TODO: создать стиль
        changed = true;
    }
    if (_lineStyleWidget->hasChanged()) {
        style->set_linestyle(_lineStyleWidget->createLineStyle(style->get_linestyle()));
        changed = true;
    }
    if (_areaStyleWidget->hasChanged()) {
        style->set_polystyle(_areaStyleWidget->createPolyStyle(style->get_polystyle()));
        changed = true;
    }
    if (_iconStyleWidget->hasChanged()) {
        style->set_iconstyle(_iconStyleWidget->createIconStyle(style->get_iconstyle()));
        changed = true;
    }
    if (_labelStyleWidget->hasChanged()) {
        style->set_labelstyle(_labelStyleWidget->createLabelStyle(style->get_labelstyle()));
        changed = true;
    }
    if (changed) {
        emit elementChanged(_lastDoc, _lastFeature);
    }
}

void KmlSettings::showStyleWidgets(bool line, bool area, bool label, bool icon)
{
    _lineStyleWidget->setVisible(line);
    _areaStyleWidget->setVisible(area);
    _labelStyleWidget->setVisible(label);
    _iconStyleWidget->setVisible(icon);
}

void KmlSettings::applyStyle()
{

}

void KmlSettings::showSettings(kmldom::Document* parent, kmldom::Feature* feature)
{
    _nameEdit->setText(QString::fromStdString(feature->get_name()));
    _descriptionEdit->setText(QString::fromStdString(feature->get_description()));
    _lastDoc = parent;
    _lastFeature = feature;

    _lineStyleWidget->resetChangedState();
    _areaStyleWidget->resetChangedState();
    _iconStyleWidget->resetChangedState();
    _labelStyleWidget->resetChangedState();

    kmldom::StylePtr style = KmlUtils::resolveStyle(parent, feature);
    if (feature->IsA(kmldom::Type_Placemark)) {
        if (style) {
            _lineStyleWidget->initFromSubStyle(style->get_linestyle());
            _areaStyleWidget->initFromSubStyle(style->get_polystyle());
            _iconStyleWidget->initFromSubStyle(style->get_iconstyle());
            _labelStyleWidget->initFromSubStyle(style->get_labelstyle());
        }
        kmldom::Placemark* placemark = static_cast<kmldom::Placemark*>(feature);
        if (!placemark->has_geometry()) {
            kmldom::PointPtr point = kmldom::KmlFactory::GetFactory()->CreatePoint();
            placemark->set_geometry(point); // HACK
        }
        const kmldom::GeometryPtr& geometry = placemark->get_geometry();
        if (geometry->IsA(kmldom::Type_Polygon)) {
            showStyleWidgets(true, true, false, false);
        } else if (geometry->IsA(kmldom::Type_Point)) {
            showStyleWidgets(false, false, true, true);
        } else if (geometry->IsA(kmldom::Type_LineString)) {
            showStyleWidgets(true, false, false, false);
        }
    } else if (feature->IsA(kmldom::Type_Container)) {
        showStyleWidgets(false, false, false, false); // TODO: слияние стилей
    }

    show();
}
}
}
}
