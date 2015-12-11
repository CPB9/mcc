/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include <string>
#include <QString>
#include <QMetaType>
#include "mcc/misc/NetVariant.h"
#include "mcc/messages/Tm.h"


namespace mcc {
namespace misc {

class TmParam
{
public:
    TmParam(){}
    TmParam(const QString& device, const QString& trait, const QString& status, const NetVariant& value = NetVariant())
        :_device(device), _trait(trait), _status(status), _value(value)
    {
    }

    const QString& device() const { return _device; };
    const QString& trait()  const { return _trait; };
    const QString& status() const { return _status; };
    const NetVariant& value()  const { return _value; }

    void setValue(const NetVariant& value) { _value = value; }

    inline const QString& name() const
    {
        if (_name.isEmpty())
            _name = QString("%1.%2.%3").arg(_device).arg(_trait).arg(_status);
        return _name;
    }

private:
    mutable QString _name;
    QString _device;
    QString _trait;
    QString _status;
    NetVariant _value;
};

class ParamValueList {
public:
    template<typename T>
    void insert(const std::string &trait, const std::string &status, T value)
    {
        _params.emplace_back(trait, status, value);
    }

    template<typename T>
    void insert(const std::string &trait, const std::initializer_list<const std::string> names,
                std::initializer_list<T> values)
    {
        BMCL_ASSERT_MSG(names.size() == values.size(), "programming error");
        auto it2(values.begin());
        for (auto it(names.begin()); it != names.end(); ++it, ++it2)
        {
            insert<T>(trait, *it, *it2);
        }
    }

    std::vector<mcc::messages::TmParam> _params;
};

}
}

Q_DECLARE_METATYPE(mcc::misc::TmParam)
