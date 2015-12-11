/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include <QString>
#include <QVector>
#include <QMetaType>
#include <memory>
#include "bmcl/Option.h"


namespace mcc {
namespace misc {

struct CmdParamDescription
{
    std::size_t _id;
    std::size_t _number;
    QString     _name;
    QString     _type;
    QString     _unit;
    QString     _info;
    bmcl::Option<double> _min;
    bmcl::Option<double> _max;
};
typedef QVector<CmdParamDescription> CmdParamDescriptionList;

struct CmdDescription
{
    std::size_t _id;
    std::size_t _number;
    QString     _name;
    QString     _info;
    CmdParamDescriptionList  _args;
};
typedef QVector<CmdDescription> CmdDescriptionList;

struct TmParamDescription
{
    std::size_t _id;
    std::size_t _number;
    QString     _name;
    QString     _info;
    QString     _type;
    QString     _unit;
    QString     _properties;
    bmcl::Option<double> _min;
    bmcl::Option<double> _max;
};
typedef QVector<TmParamDescription> TmParamDescriptionList;

struct TraitDescription
{
    std::size_t _id;
    std::size_t _parentId;
    QString _name;
    QString _unique_name;
    QString _info;
    TmParamDescriptionList  _tmParams;
    CmdDescriptionList      _cmds;
};
typedef QVector<TraitDescription> TraitDescriptionList;

struct FirmwareDescription
{
    std::size_t _id;
    QString _name;
    QString _info;
    QString _registered;
    QString _source;
    TraitDescriptionList _traits;
};
typedef std::shared_ptr<FirmwareDescription> FirmwareDescriptionPtr;
}
}


Q_DECLARE_METATYPE(mcc::misc::CmdParamDescription);
Q_DECLARE_METATYPE(mcc::misc::CmdDescription);
Q_DECLARE_METATYPE(mcc::misc::TmParamDescription);
Q_DECLARE_METATYPE(mcc::misc::TraitDescription);
Q_DECLARE_METATYPE(mcc::misc::FirmwareDescription);
