/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QCoreApplication>
#include "mcc/misc/ProcessIni.h"
#include "mcc/Names.h"
#include "mcc/core/router/Service.h"


int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    auto parser = mcc::misc::iniProcess(mcc::Names::coreRouter());
    mcc::misc::process(parser.get(), app);

    //auto thread = mcc::misc::Runnable::startInThread<mcc::core::router::Service>(parser->value(mcc::misc::CmdLine::router()).toStdString(), false);
    return app.exec();
}
