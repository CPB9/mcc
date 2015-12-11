/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include <memory>
#include <QCommandLineParser>

class QString;
class QCoreApplication;

namespace mcc {
namespace misc
{
class CmdLine
{
public:
    static inline QString router() { return "router"; }
    static inline QString log() { return "log"; }
};

extern std::unique_ptr<QCommandLineParser> iniProcess(const QString& name);
extern void process(QCommandLineParser* parser, const QCoreApplication& app);
}
}