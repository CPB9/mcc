/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "mcc/s57/RecordDescriptor.h"

#include <cstdint>
#include <vector>

namespace mcc {
namespace s57 {

class ParseTree {
public:
    void addRecord(const Record& record);
    void addRecord(Record&& record);
private:
    std::vector<Record> _records;
};

inline void ParseTree::addRecord(const Record& record)
{
    _records.push_back(record);
}

inline void ParseTree::addRecord(Record&& record)
{
    _records.push_back(std::move(record));
}
}
}
