/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mcc/ui/core/Structs.h"
#include "mcc/ui/map/layers/FlagLayer.h"
#include "mcc/ui/core/GlobalCoordinatePrinter.h"

#include <algorithm>

#include <QPainter>
#include <QDebug>
#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QApplication>
#include <QClipboard>
#include <QDialog>
#include <QMenu>
#include <QAction>

namespace mcc {
namespace ui {
namespace map {

FlagLayer::FlagLayer(const MapRectConstPtr& rect)
    : SimpleFlagLayer(rect)
    , _editor(new QDialog(rect->parent()))
    , _lineEdit(new QLineEdit)
    , _coordsLabel(new QLabel("Координаты: "))
    , _flagCounter(1)
{
    QPushButton* flagCopyCoordsButton = new QPushButton("В буфер обмена");
    QHBoxLayout* flagCoordsLayout = new QHBoxLayout;
    flagCoordsLayout->addWidget(_coordsLabel);
    _latEditor = core::GlobalCoordinatePrinter::instance()->printer().latitudeEditor(_editor.get());
    flagCoordsLayout->addWidget(_latEditor);
    _lonEditor = core::GlobalCoordinatePrinter::instance()->printer().longitudeEditor(_editor.get());
    flagCoordsLayout->addWidget(_lonEditor);
    flagCoordsLayout->addWidget(flagCopyCoordsButton);
    QPushButton* flagOkButton = new QPushButton("Ок");
    QPushButton* flagCancelButton = new QPushButton("Отмена");
    QPushButton* flagDeleteButton = new QPushButton("Удалить");
    QHBoxLayout* flagLabelLayout = new QHBoxLayout;
    flagLabelLayout->addWidget(new QLabel("Метка"));
    flagLabelLayout->addWidget(_lineEdit);
    QHBoxLayout* flagButtonsLayout = new QHBoxLayout;
    flagButtonsLayout->addStretch();
    flagButtonsLayout->addWidget(flagOkButton);
    flagButtonsLayout->addWidget(flagCancelButton);
    flagButtonsLayout->addWidget(flagDeleteButton);
    QVBoxLayout* flagLayout = new QVBoxLayout;
    flagLayout->addLayout(flagCoordsLayout);
    flagLayout->addLayout(flagLabelLayout);
    flagLayout->addLayout(flagButtonsLayout);
    _editor->setLayout(flagLayout);
    _editor->setWindowTitle("Настройки флага");

    _activePixmap = QPixmap::fromImage(Flag::drawFlag(26, 39, Qt::yellow));
    _inactivePixmap = QPixmap::fromImage(Flag::drawFlag(26, 39, Qt::blue));

    connect(flagCopyCoordsButton, &QPushButton::clicked, this,
            [this]() { currentFlag()->printCoordinatesToClipboard(mapRect()); });

    connect(flagOkButton, &QPushButton::clicked, this, [&]() {
        currentFlag()->setName(_lineEdit->text());
        currentFlag()->setLatLon(core::LatLon(_latEditor->latLon().latitude, _lonEditor->latLon().longitude), mapRect());
        _editor->close();
        emit sceneUpdated();
    });

    connect(flagCancelButton, &QPushButton::clicked, this, [this]() { _editor->close(); });

    connect(flagDeleteButton, &QPushButton::clicked, this, [this]() {
        removeActive();
        _editor->close();
        emit sceneUpdated();
    });

    _contextMenu = new QMenu(mapRect()->parent());
    _addAction = _contextMenu->addAction("Добавить флаг");
    _removeAction = _contextMenu->addAction("Удалить флаг");
    _clearAction = _contextMenu->addAction("Очистить флаги");
    _contextMenu->addSeparator();
    _copyAction = _contextMenu->addAction("Скопировать координаты");

    connect(_removeAction, &QAction::triggered, this, [this]() {
        removeActive();
        emit sceneUpdated();
    });
    connect(_copyAction, &QAction::triggered, this,
            [this]() { currentFlag()->printCoordinatesToClipboard(mapRect()); });
    connect(_addAction, &QAction::triggered, this, [this]() {
        addFlagAt(mapRect()->fromWindowSystemCoordinates(_contextMenu->pos()));
        emit sceneUpdated();
    });
    connect(_clearAction, &QAction::triggered, this, [this]() {
        _flags.clear();
        _flagCounter = 1;
        _activeFlag = misc::None;
        emit sceneUpdated();
    });
}

void FlagLayer::removeActive()
{
    _flags.erase(_flags.begin() + _activeFlag.unwrap());
    if (_flags.empty()) {
        _flagCounter = 1;
    }
    _activeFlag = misc::None;
}

QString FlagLayer::genNextFlagName()
{
    QString name = "Флаг " + QString::number(_flagCounter);
    _flagCounter++;
    return name;
}

void FlagLayer::changeProjection(const MercatorProjection& from, const MercatorProjection& to)
{
    for (Flag& flag : _flags) {
        flag.changeProjection(mapRect(), from, to);
    }
}

FlagLayer::~FlagLayer()
{
}

misc::Option<std::size_t> FlagLayer::flagAt(const QPointF& pos)
{
    for (std::size_t i = 0; i < _flags.size(); i++) {
        if (_flags[i].rect().contains(pos)) {
            return i;
        }
    }
    return misc::None;
}

misc::Option<LineIndexAndPos> FlagLayer::lineAt(const QPointF& pos)
{
    (void)pos;
    return misc::None;
}

void FlagLayer::draw(QPainter* p) const
{
    for (const Flag& flag : _flags) {
        flag.drawWithoutLabel(p, mapRect());
    }
    for (const Flag& flag : _flags) {
        flag.drawLabel(p, mapRect());
    }
}

bool FlagLayer::showContextMenuForCurrent(const QPoint& pos)
{
    _removeAction->setEnabled(true);
    _copyAction->setEnabled(true);
    _addAction->setEnabled(false);
    _contextMenu->exec(mapRect()->toWindowSystemCoordinates(pos));
    return true;
}

bool FlagLayer::showContextMenuForNone(const QPoint& pos)
{
    _removeAction->setEnabled(false);
    _copyAction->setEnabled(false);
    _addAction->setEnabled(true);
    _contextMenu->exec(mapRect()->toWindowSystemCoordinates(pos));
    return true;
}

void FlagLayer::addFlagAt(const QPointF& pos)
{
    _activeFlag = _flags.size();
    _flags.emplace_back(pos, _activePixmap, _inactivePixmap, genNextFlagName(), mapRect());
    _flags.back().setActive(true);
}

void FlagLayer::insertFlagAt(std::size_t index, const QPointF& pos)
{
    _activeFlag = index;
    _flags.emplace(_flags.begin() + index, pos, _activePixmap, _inactivePixmap, genNextFlagName(), mapRect());
    _flags[index].setActive(true);
}

void FlagLayer::moveCurrentBy(const QPointF& delta)
{
    currentFlag()->moveBy(delta);
}

void FlagLayer::setCurrentActive(bool isActive)
{
    currentFlag()->setActive(isActive);
}

void FlagLayer::showFlagEditor(const QPoint& pos)
{
    Flag* flag = currentFlag();
    _lineEdit->setText(flag->name());
    core::LatLon latLon(flag->toLatLon(mapRect()));
    _latEditor->setLatLon(latLon);
    _lonEditor->setLatLon(latLon);
    _editor->move(mapRect()->toWindowSystemCoordinates(pos));
    _editor->exec();
}

bool FlagLayer::zoomEvent(const QPoint& pos, int from, int to)
{
    (void)pos;
    for (Flag& flag : _flags) {
        flag.changeZoomLevel(from, to);
    }
    return true;
}

bool FlagLayer::viewportResetEvent(int oldZoom, int newZoom, const QRect& oldViewpiort, const QRect& newViewport)
{
    (void)oldViewpiort;
    (void)newViewport;
    for (Flag& flag : _flags) {
        flag.changeZoomLevel(oldZoom, newZoom);
    }
    return true;
}

void FlagLayer::finishMovingCurrent()
{
}
}
}
}
