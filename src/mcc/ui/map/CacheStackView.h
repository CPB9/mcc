/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <QWidget>

class QTableView;
class QPushButton;
class QToolButton;

namespace mcc {
namespace ui {
namespace map {

class CacheStackModel;

class CacheStackView : public QWidget {
    Q_OBJECT
public:
    explicit CacheStackView(CacheStackModel* model, QWidget* parent = 0);

    bool event(QEvent*) override;

signals:
    void cacheUpdated();
    void clicked();

private:
    CacheStackModel* _model;
    QTableView* _view;

    QPushButton* _addButton;
    QPushButton* _removeButton;
    QToolButton* _moveUpButton;
    QToolButton* _moveDownButton;
};
}
}
}
