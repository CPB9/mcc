/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mcc/ui/map/KmlStyleWidget.h"

#include <QColorDialog>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QSpinBox>
#include <QToolButton>
#include <QDebug>

#include <kml/dom/colorstyle.h>
#include <kml/dom/iconstyle.h>
#include <kml/dom/kml_cast.h>
#include <kml/dom/kml_factory.h>
#include <kml/dom/labelstyle.h>
#include <kml/dom/linestyle.h>
#include <kml/dom/polystyle.h>

namespace mcc {
namespace ui {
namespace map {

StyleWidget* StyleWidget::createFillWidget()
{
    StyleWidget* w = new StyleWidget("Заливка");
    w->_widthBox->hide();
    w->_widthLabel->hide();
    return w;
}

StyleWidget* StyleWidget::createLineWidget()
{
    StyleWidget* w = new StyleWidget("Линия", "Толщина");
    w->_fillBox->hide();
    return w;
}

StyleWidget* StyleWidget::createIconWidget()
{
    StyleWidget* w = new StyleWidget("Метка", "Увеличение");
    w->_fillBox->hide();
    return w;
}

StyleWidget* StyleWidget::createLabelWidget()
{
    StyleWidget* w = new StyleWidget("Надпись", "Увеличение");
    w->_fillBox->hide();
    return w;
}

StyleWidget::StyleWidget(const QString& title, const QString& labelTitle, QWidget* parent)
    : QGroupBox(title, parent)
    , _colorButton(new QToolButton)
    , _widthBox(new QDoubleSpinBox)
    , _opacitySlider(new QSlider)
    , _opacityBox(new QSpinBox)
    , _fillBox(new QComboBox)
    , _widthLabel(new QLabel(labelTitle))
    , _hasChanged(false)
{
    _widthBox->setMinimum(0);
    _widthBox->setMaximum(100);
    _widthBox->setValue(1);
    _opacitySlider->setOrientation(Qt::Horizontal);
    _opacitySlider->setMinimum(0);
    _opacitySlider->setMaximum(100);
    _opacitySlider->setValue(100);
    _opacityBox->setSuffix("%");
    _opacityBox->setMinimum(0);
    _opacityBox->setMaximum(100);
    _opacityBox->setValue(100);
    _fillBox->addItems({"Заливка", "Границы", "Заливка+Границы"});
    QHBoxLayout* layout = new QHBoxLayout;
    layout->addWidget(new QLabel("Цвет"));
    layout->addWidget(_colorButton);
    layout->addStretch();
    layout->addWidget(_fillBox);
    layout->addWidget(_widthLabel);
    layout->addWidget(_widthBox);
    layout->addStretch();
    layout->addWidget(new QLabel("Непрозрачность"));
    layout->addWidget(_opacitySlider);
    layout->addWidget(_opacityBox);
    setColor(Qt::white);
    connect<void (QSpinBox::*)(int)>(_opacityBox, &QSpinBox::valueChanged, _opacitySlider, [this](int value) {
        _opacitySlider->setValue(value);
        resetChangedState(true);
    });
    connect<void (QSlider::*)(int)>(_opacitySlider, &QSlider::valueChanged, _opacityBox, [this](int value) {
        _opacityBox->setValue(value);
        resetChangedState(true);
    });
    connect<void (QDoubleSpinBox::*)(double)>(_widthBox, &QDoubleSpinBox::valueChanged, this, [this](double value) {
        (void)value;
        resetChangedState(true);
    });
    connect(_colorButton, &QToolButton::clicked, this, [this]() {
        QColor color = QColorDialog::getColor(_currentColor, this, "Цвет");
        if (color.isValid()) {
            setColor(color);
        }
    });
    connect<void (QComboBox::*)(int)>(_fillBox, &QComboBox::activated, this, [this](){
        resetChangedState(true);
    });
    setLayout(layout);
}

void StyleWidget::resetChangedState(bool isChanged)
{
    if (isChanged == _hasChanged) {
        return;
    }

    if (isChanged) {
        setTitle(title() + "*");
    } else {
        QString title = this->title();
        setTitle(title.mid(0, title.length() - 1));
    }

    _hasChanged = isChanged;
}

void StyleWidget::setPolygonStyle(StyleWidget::PolygonStyle style)
{
    switch (style) {
    case Fill:
        _fillBox->setCurrentIndex(0);
        break;
    case Outline:
        _fillBox->setCurrentIndex(1);
        break;
    case FillOutline:
        _fillBox->setCurrentIndex(2);
        break;
    }
}

StyleWidget::PolygonStyle StyleWidget::polygonStyle() const
{
    switch (_fillBox->currentIndex()) {
    case 0:
        return Fill;
    case 1:
        return Outline;
    default:
        return FillOutline;
    }
}

void StyleWidget::setFillAndOutline(bool hasFill, bool hasOutline)
{
    if (hasFill) {
        if (hasOutline) {
            setPolygonStyle(FillOutline);
        } else {
            setPolygonStyle(Fill);
        }
    } else {
        if (hasOutline) {
            setPolygonStyle(Outline);
        } else {
            setPolygonStyle(Fill); // default value
        }
    }
}

std::pair<bool, bool> StyleWidget::fillAndOutline() const
{
    bool fill = true;
    bool outline = true;
    if (polygonStyle() == Fill) {
        outline = false;
    } else if (polygonStyle() == Outline) {
        fill = false;
    }
    return std::make_pair(fill, outline);
}

void StyleWidget::setColor(const QColor& color)
{
    _colorButton->setStyleSheet(QString("background-color: %1;"
                                        "border: 1px;"
                                        "border-color: black;"
                                        "border-style: outset;").arg(color.name()));
    _currentColor = color;
    resetChangedState(true);
}

void StyleWidget::setColor(const kmlbase::Color32& color)
{
    QColor qcolor;
    qcolor.setRed(color.get_red());
    qcolor.setBlue(color.get_blue());
    qcolor.setGreen(color.get_green());
    setOpacity(double(color.get_alpha()) / 255.0 * 100.0);
    setColor(qcolor);
}

kmlbase::Color32 StyleWidget::color() const
{
    kmlbase::Color32 color;
    color.set_red(_currentColor.red());
    color.set_green(_currentColor.green());
    color.set_blue(_currentColor.blue());
    color.set_alpha(double(opacity()) / 100.0 * 255.0);
    return color;
}

void StyleWidget::setOpacity(int opacity)
{
    _opacityBox->setValue(opacity);
}

int StyleWidget::opacity() const
{
    return _opacityBox->value();
}

void StyleWidget::setWidth(double width)
{
    _widthBox->setValue(width);
}

double StyleWidget::width() const
{
    return _widthBox->value();
}

void StyleWidget::initFromSubStyle(const kmldom::SubStylePtr& style)
{
    if (!style) {
        resetChangedState(false);
        return;
    }
    if (style->IsA(kmldom::Type_ColorStyle)) {
        auto colorStyle = kmldom::AsColorStyle(style);
        setColor(colorStyle->get_color());
    }
    if (style->IsA(kmldom::Type_PolyStyle)) {
        auto polyStyle = kmldom::AsPolyStyle(style);
        setFillAndOutline(polyStyle->get_fill(), polyStyle->get_outline());
    } else if (style->IsA(kmldom::Type_IconStyle)) {
        auto iconStyle = kmldom::AsIconStyle(style);
        setWidth(iconStyle->get_scale());
    } else if (style->IsA(kmldom::Type_LabelStyle)) {
        auto labelStyle = kmldom::AsLabelStyle(style);
        setWidth(labelStyle->get_scale());
    } else if (style->IsA(kmldom::Type_LineStyle)) {
        auto lineStyle = kmldom::AsLineStyle(style);
        setWidth(lineStyle->get_width());
    }
    resetChangedState(false);
}

kmldom::PolyStylePtr StyleWidget::createPolyStyle(kmldom::PolyStylePtr style) const
{
    if (!style) {
        style = kmldom::KmlFactory::GetFactory()->CreatePolyStyle();
    }
    std::pair<bool, bool> fillOutline = fillAndOutline();
    qDebug() << fillOutline.first;
    qDebug() << fillOutline.second;
    style->set_fill(fillOutline.first);
    style->set_outline(fillOutline.second);
    style->set_color(color());
    return style;
}

kmldom::LineStylePtr StyleWidget::createLineStyle(kmldom::LineStylePtr style) const
{
    if (!style) {
        style = kmldom::KmlFactory::GetFactory()->CreateLineStyle();
    }
    style->set_color(color());
    style->set_width(width());
    return style;
}

kmldom::IconStylePtr StyleWidget::createIconStyle(kmldom::IconStylePtr style) const
{
    if (!style) {
        style = kmldom::KmlFactory::GetFactory()->CreateIconStyle();
    }
    style->set_color(color());
    style->set_scale(width());
    return style;
}

kmldom::LabelStylePtr StyleWidget::createLabelStyle(kmldom::LabelStylePtr style) const
{
    if (!style) {
        style = kmldom::KmlFactory::GetFactory()->CreateLabelStyle();
    }
    style->set_color(color());
    style->set_scale(width());
    return style;
}
}
}
}
