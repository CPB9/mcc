/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mcc/ui/map/CacheStackView.h"
#include "mcc/ui/map/CacheStackModel.h"
#include "mcc/ui/core/Settings.h"

#include <QTableView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QToolButton>
#include <QHeaderView>
#include <QFileDialog>
#include <QMouseEvent>
#include <QApplication>
#include <QStyle>
#include <QDebug>

namespace mcc {
namespace ui {
namespace map {

CacheStackView::CacheStackView(CacheStackModel* model, QWidget* parent)
    : QWidget(parent)
    , _model(model)
{
    _view = new QTableView;
    _view->setModel(_model);
    _view->horizontalHeader()->setStretchLastSection(true);
    _view->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    _view->verticalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    _view->setContextMenuPolicy(Qt::CustomContextMenu);
    _view->setSelectionMode(QAbstractItemView::SingleSelection);
    _view->setSelectionBehavior(QAbstractItemView::SelectRows);
    _view->setWordWrap(false);

    _addButton = new QPushButton("Добавить");
    _removeButton = new QPushButton("Удалить");
    _removeButton->setEnabled(false);
    _moveUpButton = new QToolButton;
    _moveUpButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_ArrowUp));
    _moveUpButton->setEnabled(false);
    _moveDownButton = new QToolButton;
    _moveDownButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_ArrowDown));
    _moveDownButton->setEnabled(false);

    QVBoxLayout* upDownlayout = new QVBoxLayout;
    upDownlayout->addWidget(_moveUpButton);
    upDownlayout->addWidget(_moveDownButton);
    upDownlayout->addStretch();

    QHBoxLayout* viewLayout = new QHBoxLayout;
    viewLayout->addWidget(_view);
    viewLayout->addLayout(upDownlayout);

    QHBoxLayout* buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(_addButton);
    buttonLayout->addWidget(_removeButton);
    buttonLayout->addStretch();
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    // buttonLayout->addWidget(_okButton);

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addLayout(viewLayout);
    mainLayout->addLayout(buttonLayout);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(3);

    setLayout(mainLayout);

    connect(_view->selectionModel(), &QItemSelectionModel::selectionChanged, this,
            [this](const QItemSelection& selected, const QItemSelection& deselected) {
                (void)deselected;
                if (selected.isEmpty()) {
                    return;
                }
                auto idxs = selected.at(0);
                _removeButton->setEnabled(_model->canRemoveAt(idxs.topLeft().row()));
                _moveUpButton->setEnabled(idxs.topLeft().row() != 0);
                _moveDownButton->setEnabled(idxs.bottomRight().row() != _model->rowCount() - 1);
            });

    connect(_moveUpButton, &QPushButton::clicked, this, [this]() {
        auto idxs = _view->selectionModel()->selectedRows();
        std::sort(idxs.begin(), idxs.end());
        if (idxs.isEmpty()) {
            return;
        }
        auto idx = idxs[0];
        _model->moveUp(idx.row());
        _moveUpButton->setEnabled(idx.row() != 1);
        _moveDownButton->setEnabled(true);
        emit cacheUpdated();
    });

    connect(_moveDownButton, &QPushButton::clicked, this, [this]() {
        auto idxs = _view->selectionModel()->selectedRows();
        std::sort(idxs.begin(), idxs.end());
        if (idxs.isEmpty()) {
            return;
        }
        auto idx = idxs[0];
        _model->moveDown(idx.row());
        _moveDownButton->setEnabled(idx.row() != _model->rowCount() - 2);
        _moveUpButton->setEnabled(true);
        emit cacheUpdated();
    });

    connect(_addButton, &QPushButton::clicked, this, [this]() {
        QString last = core::Settings::instance()->lastStackPath();
        QString path = QFileDialog::getOpenFileName(this, "Добавить кеш", last, "Кеш карт (*.omcf)");
        if (path.isEmpty()) {
            return;
        }
        core::Settings::instance()->setLastStackPath(path);
        _model->addOmcfCache(path);
        emit cacheUpdated();
    });

    connect(_removeButton, &QPushButton::clicked, this, [this]() {
        auto idxs = _view->selectionModel()->selectedRows();
        std::sort(idxs.begin(), idxs.end());
        if (idxs.isEmpty()) {
            return;
        }
        auto idx = idxs[0];
        _model->removeAt(idx.row());
        emit cacheUpdated();
    });
    connect(_model, &CacheStackModel::stackChanged, this, &CacheStackView::cacheUpdated);
}

bool CacheStackView::event(QEvent* event)
{
    if (!isEnabled() && (event->type() == QEvent::MouseButtonPress)) {
        QMouseEvent* mevent = static_cast<QMouseEvent*>(event);
        if (mevent->button() == Qt::LeftButton) {
            event->accept();
            emit clicked();
            return true;
        }
    }
    return QWidget::event(event);
}
}
}
}
