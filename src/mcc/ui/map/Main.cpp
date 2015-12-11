/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mcc/ui/map/OmcfCacheWidget.h"

#include <QApplication>
#include <QTreeView>

using namespace mcc::ui::map;

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    OmcfCacheWidget view;
    view.show();
    return app.exec();
}
