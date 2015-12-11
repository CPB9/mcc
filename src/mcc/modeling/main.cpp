/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QString>
#include <QDebug>
#include "mcc/Settings.h"
#include "mcc/misc/ProcessIni.h"
#include "mcc/modeling/SimpleModel.h"

using namespace mcc::modeling;

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(mcc::Names::model());
    QCoreApplication::setApplicationVersion("1.0");

    auto parser = mcc::misc::iniProcess(mcc::Names::model());
    parser->addOption(QCommandLineOption({ "m", "model" }, "specifies model id", "model id"));
    parser->addOption(QCommandLineOption({ "p", "port" }, "specifies udp command port (default is " + QString::number(DefaultModelControlPort) + " + model id)", "command port", QString::number(DefaultModelControlPort)));
    mcc::misc::process(parser.get(), app);

    if (!parser->isSet("model"))
    {
        qDebug() << "model id is no set";
        return 3;
    }
    std::string router = parser->value("router").toStdString();
    std::size_t id = parser->value("model").toUInt();
    int port = parser->value("port").toInt() + id;

    std::unique_ptr<SimpleModel> service;
    switch(id)
    {
    case 1: service = mcc::messages::ServiceAbstract::startInThread<mcc::modeling::Model1>(mcc::messages::LocalRouterPtr(), port); break;
    case 2: service = mcc::messages::ServiceAbstract::startInThread<mcc::modeling::Model2>(mcc::messages::LocalRouterPtr(), port); break;
    case 3: service = mcc::messages::ServiceAbstract::startInThread<mcc::modeling::Model3>(mcc::messages::LocalRouterPtr(), port); break;
    case 4: service = mcc::messages::ServiceAbstract::startInThread<mcc::modeling::Model4>(mcc::messages::LocalRouterPtr(), port); break;
    default:
        qDebug() << "Incorrect model number";
        return 2;
    }

    app.exec();
}
