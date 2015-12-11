/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mcc/ui/map/KmlView.h"
#include "mcc/ui/map/KmlModel.h"

#include <QTreeView>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QPushButton>
#include <QDebug>

namespace mcc {
namespace ui {
namespace map {

KmlView::KmlView(KmlModel* model, QWidget* parent)
    : QWidget(parent)
    , _model(model)
{
    _view = new QTreeView;
    //_model->open("test.kml");
    _view->setModel(_model);
    _view->header()->setStretchLastSection(true);
    _view->header()->setSectionResizeMode(0, QHeaderView::Interactive);
    _view->setContextMenuPolicy(Qt::CustomContextMenu);
    _view->setSelectionMode(QAbstractItemView::SingleSelection);
    _view->setSelectionBehavior(QAbstractItemView::SelectRows);
    _view->setDragEnabled(true);
    _view->viewport()->setAcceptDrops(true);
    _view->setDropIndicatorShown(true);
    _view->setAlternatingRowColors(true);
    _view->header()->setStretchLastSection(true);

    QHBoxLayout* buttonsLayout = new QHBoxLayout;
    _addFolder   = new QPushButton(QIcon(":/resources/folder.png"), "", this);
    _addFlag     = new QPushButton(QIcon(":/resources/flag.png"), "", this);
    _addPolyline = new QPushButton(QIcon(":/resources/polyline.png"), "", this);
    _addPolygon  = new QPushButton(QIcon(":/resources/polygon.png"), "", this);

    buttonsLayout->addWidget(_addFolder);
    buttonsLayout->addWidget(_addFlag);
    buttonsLayout->addWidget(_addPolyline);
    buttonsLayout->addWidget(_addPolygon);

    connect(_addFolder,     &QPushButton::pressed, _model, &KmlModel::onFolderAdd);
    connect(_addFlag,       &QPushButton::pressed, _model, &KmlModel::onPointAdd);
    connect(_addPolyline,   &QPushButton::pressed, _model, &KmlModel::onPolylineAdd);
    connect(_addPolygon,    &QPushButton::pressed, _model, &KmlModel::onPolygonAdd);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addLayout(buttonsLayout);
    layout->addWidget(_view);
    setLayout(layout);

    connect(_view, &QTreeView::customContextMenuRequested, this, &KmlView::onCustomContextMenu);
    connect(_view, &QTreeView::doubleClicked, this, [this](const QModelIndex& index) { _model->centerOn(index); });
    connect(_view->selectionModel(), &QItemSelectionModel::selectionChanged, this, &KmlView::onSelectionChanged);
    connect(_view, &QTreeView::expanded, this, [&](const QModelIndex& index) { Q_UNUSED(index);  _view->resizeColumnToContents(0); });
    connect(_view, &QTreeView::collapsed, this, [&](const QModelIndex& index) { Q_UNUSED(index); _view->resizeColumnToContents(0); });

}

void KmlView::onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    Q_UNUSED(deselected);
    if (!selected.isEmpty()) {
        _model->selectIndex(selected.at(0).topLeft());
    } else {
        _model->selectIndex(QModelIndex());
    }
}

void KmlView::onCustomContextMenu(const QPoint& point)
{
    QModelIndex index = _view->indexAt(point);
    if (!index.isValid()) {
        _view->clearSelection();
    }
    _model->showContextMenu(index, _view->viewport()->mapToGlobal(point));
}
}
}
}
