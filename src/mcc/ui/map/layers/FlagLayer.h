/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "mcc/ui/core/GlobalCoordinatePrinter.h"
#include "mcc/ui/core/LatLonEdit.h"
#include "mcc/ui/map/layers/SimpleFlagLayer.h"
#include "mcc/ui/map/drawables/Flag.h"

#include <QLineEdit>
#include <QLabel>
#include <QDialog>

#include <deque>

class QWidget;
class QDialog;

namespace mcc {
namespace ui {
namespace map {

class FlagLayer : public SimpleFlagLayer {
    Q_OBJECT
public:
    FlagLayer(const MapRectConstPtr& rect);
    ~FlagLayer() override;
    void draw(QPainter* p) const override;
    bool zoomEvent(const QPoint& pos, int fromZoom, int toZoom) override;
    void changeProjection(const MercatorProjection& from, const MercatorProjection& to) override;
    bool viewportResetEvent(int oldZoom, int newZoom, const QRect& oldViewpiort, const QRect& newViewport) override;

protected:
    void addFlagAt(const QPointF& pos) override;
    void insertFlagAt(std::size_t index, const QPointF& pos) override;
    misc::Option<std::size_t> flagAt(const QPointF& pos) override;
    misc::Option<LineIndexAndPos> lineAt(const QPointF& pos) override;
    void moveCurrentBy(const QPointF& delta) override;
    void setCurrentActive(bool isActive) override;
    bool showContextMenuForCurrent(const QPoint& pos) override;
    bool showContextMenuForNone(const QPoint& pos) override;
    void showFlagEditor(const QPoint& pos) override;
    void finishMovingCurrent() override;

private:
    void removeActive();
    Flag* currentFlag();
    QString genNextFlagName();

    std::deque<Flag> _flags;
    QPixmap _activePixmap;
    QPixmap _inactivePixmap;
    std::unique_ptr<QDialog> _editor;
    QLineEdit* _lineEdit;
    QLabel* _coordsLabel;
    QMenu* _contextMenu;
    QAction* _copyAction;
    QAction* _removeAction;
    QAction* _addAction;
    QAction* _clearAction;
    core::LatLonEdit* _latEditor;
    core::LatLonEdit* _lonEditor;
    int _flagCounter;
};

inline Flag* FlagLayer::currentFlag()
{
    return &_flags[_activeFlag.unwrap()];
}
}
}
}
