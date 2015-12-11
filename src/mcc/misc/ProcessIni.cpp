/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <memory>
#include <mutex>

#include <QCoreApplication>
#include <QStringList>
#include <QThread>
#include <QDateTime>
#include "bmcl/Logging.h"
#include "bmcl/ColorStream.h"
#include "mcc/misc/Net.h"
#include "mcc/misc/ProcessIni.h"
#include "mcc/Settings.h"


namespace mcc {
namespace misc {

void bmclHandler(bmcl::LogLevel level, const char* msg)
{
    if ((int)level > (int)bmcl::logLevel()) {
        return;
    }
    QByteArray threadName = QThread::currentThread()->objectName().toLocal8Bit();
    if (threadName.size() < 20) {
        QByteArray filler;
        filler.fill(' ', 20 - threadName.size());
        threadName.append(filler);
    }
    QByteArray time = QTime::currentTime().toString("mm:ss.zzz").toLocal8Bit();
    const char* prefix;
    bmcl::ColorAttr attr;
    switch (level) {
    case bmcl::LogLevel::Debug:
        prefix = "DEBUG:   ";
        attr = bmcl::ColorAttr::FgBlack;
        break;
    case bmcl::LogLevel::Info:
        prefix = "INFO:    ";
        attr = bmcl::ColorAttr::FgCyan;
        break;
    case bmcl::LogLevel::Warning:
        prefix = "WARNING: ";
        attr = bmcl::ColorAttr::FgYellow;
        break;
    case bmcl::LogLevel::Critical:
        prefix = "CRITICAL:";
        attr = bmcl::ColorAttr::FgRed;
        break;
    case bmcl::LogLevel::Panic:
        prefix = "PANIC:   ";
        attr = bmcl::ColorAttr::FgRed;
        break;
    default:
        prefix = "????:    ";
    }
    static std::mutex mtx;
    std::lock_guard<std::mutex> lock(mtx);
    bmcl::ColorStdError out;
    out << bmcl::ColorAttr::Bright << time.constData();
    out << " [" << threadName.constData() << "] ";
    out << attr << prefix << bmcl::ColorAttr::Reset << ' ';
    out << msg << std::endl;
}

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context);
    QByteArray local = msg.toLocal8Bit();
    const char* cmsg = local.constData();
    switch (type) {
    case QtDebugMsg:
        bmclHandler(bmcl::LogLevel::Debug, cmsg);
        break;
    case QtWarningMsg:
        bmclHandler(bmcl::LogLevel::Warning, cmsg);
        break;
    case QtCriticalMsg:
        bmclHandler(bmcl::LogLevel::Critical, cmsg);
        break;
    case QtFatalMsg:
        bmclHandler(bmcl::LogLevel::Panic, cmsg);
        abort();
    }
}

static inline QString handshakeClientAddress()
{
    return QString("tcp://127.0.0.1:%1").arg(mcc::DefaultBaseTcpPort);
}

std::unique_ptr<QCommandLineParser> iniProcess(const QString& name)
{
    setlocale(LC_ALL, "Rus");
    bmcl::setLogHandler(bmclHandler);
    qInstallMessageHandler(myMessageOutput);

    std::unique_ptr<QCommandLineParser> parser(new QCommandLineParser);
    parser->setApplicationDescription(name);
    parser->addHelpOption();
    parser->addVersionOption();
    parser->addOption(QCommandLineOption({ "r", CmdLine::router() }, "specifies router address (default is " + handshakeClientAddress() + ")", "router address", handshakeClientAddress()));

    QString text = "specifies logging level, which can be: none, panic, critical, warning, info, debug (default)";
    parser->addOption(QCommandLineOption({ "l", CmdLine::log() }, text, "logging level", "debug"));
    return parser;
}

void process(QCommandLineParser* parser, const QCoreApplication& app)
{
    parser->process(app);
    QString l = parser->value(mcc::misc::CmdLine::log());
    bmcl::LogLevel log = bmcl::LogLevel::Debug;

    if      (l == "none")     log = bmcl::LogLevel::None;
    else if (l == "panic")    log = bmcl::LogLevel::Panic;
    else if (l == "critical") log = bmcl::LogLevel::Critical;
    else if (l == "warning")  log = bmcl::LogLevel::Warning;
    else if (l == "info")     log = bmcl::LogLevel::Info;
    else if (l == "debug")    log = bmcl::LogLevel::Debug;

    BMCL_INFO() << "warning level set to " << l;
    bmcl::setLogLevel(log);
}
}
}
