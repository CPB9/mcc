/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <QGroupBox>

#include <kml/base/color32.h>
#include <kml/dom/kml_ptr.h>

#include <utility>

class QColor;
class QColorDialog;
class QComboBox;
class QDoubleSpinBox;
class QHBoxLayout;
class QLabel;
class QSlider;
class QSpinBox;
class QString;
class QToolButton;

namespace mcc {
namespace ui {
namespace map {

class StyleWidget : public QGroupBox {
    Q_OBJECT
public:
    enum PolygonStyle { Fill, Outline, FillOutline };

    static StyleWidget* createFillWidget();
    static StyleWidget* createLineWidget();
    static StyleWidget* createLabelWidget();
    static StyleWidget* createIconWidget();

    void setPolygonStyle(PolygonStyle style);
    PolygonStyle polygonStyle() const;

    void setFillAndOutline(bool hasFill, bool hasOutline);
    std::pair<bool, bool> fillAndOutline() const;

    void setColor(const QColor& color);
    void setColor(const kmlbase::Color32& color);
    kmlbase::Color32 color() const;

    void setOpacity(int opacity);
    int opacity() const;

    void setWidth(double width);
    double width() const;

    void resetChangedState(bool isChanged = false);
    bool hasChanged() const;

    void initFromSubStyle(const kmldom::SubStylePtr& style);
    kmldom::PolyStylePtr createPolyStyle(kmldom::PolyStylePtr style = 0) const;
    kmldom::LineStylePtr createLineStyle(kmldom::LineStylePtr style = 0) const;
    kmldom::IconStylePtr createIconStyle(kmldom::IconStylePtr style = 0) const;
    kmldom::LabelStylePtr createLabelStyle(kmldom::LabelStylePtr style = 0) const;

private:
    explicit StyleWidget(const QString& title, const QString& labelTitle = QString(), QWidget* parent = 0);
    QToolButton* _colorButton;
    QDoubleSpinBox* _widthBox;
    QSlider* _opacitySlider;
    QSpinBox* _opacityBox;
    QComboBox* _fillBox;
    QLabel* _widthLabel;
    QColor _currentColor;
    bool _hasChanged;
};

inline bool StyleWidget::hasChanged() const
{
    return _hasChanged;
}
}
}
}
