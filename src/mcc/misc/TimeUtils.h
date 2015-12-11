/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include <QString>
#include <QDateTime>


namespace mcc {
namespace misc {

static inline QString dateSerializeFormat()
{
    return QString("yyyy-MM-ddTHH:mm:ss.zzz");
}

static inline std::string currentDateTime()
{
    return QDateTime::currentDateTime().toString(dateSerializeFormat()).toStdString();
}


}}
