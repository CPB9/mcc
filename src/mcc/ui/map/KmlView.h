/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <QWidget>

class QTreeView;
class QItemSelection;
class QPoint;
class QPushButton;

namespace mcc {
namespace ui {
namespace map {

class KmlModel;

class KmlView : public QWidget {
public:
    KmlView(KmlModel* model, QWidget* parent = 0);

private slots:
    void onCustomContextMenu(const QPoint& point);
    void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

private:
    QTreeView* _view;
    KmlModel* _model;

    QPushButton* _addFolder;
    QPushButton* _addPolyline;
    QPushButton* _addFlag;
    QPushButton* _addPolygon;
};
}
}
}
