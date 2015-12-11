/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "mcc/s57/Record.h"
#include "mcc/s57/Ptr.h"

#include <cstdint>
#include <string>
#include <bitset>
#include <chrono>

namespace mcc {
namespace s57 {

class Parser;

class BaseCellFile {
private:
    friend class Parser;
    void setDataSetGeneralInfo(DataSetGeneralInfoPtr&& info);
    void setDataSetGeographicReference(DataSetGeographicReferencePtr&& info);
    void setVectorRecords(std::vector<VectorRecordPtr>&& records);
    void setFeatureRecords(std::vector<FeatureRecordPtr>&& records);

    DataSetGeneralInfoPtr _info;
    DataSetGeographicReferencePtr _reference;
    std::vector<VectorRecordPtr> _vectorRecords;
    std::vector<FeatureRecordPtr> _featureRecords;
};

inline void BaseCellFile::setVectorRecords(std::vector<VectorRecordPtr>&& records)
{
    _vectorRecords = std::move(records);
}

inline void BaseCellFile::setFeatureRecords(std::vector<FeatureRecordPtr>&& records)
{
    _featureRecords = std::move(records);
}

inline void BaseCellFile::setDataSetGeneralInfo(DataSetGeneralInfoPtr&& info)
{
    _info = std::move(info);
}

inline void BaseCellFile::setDataSetGeographicReference(DataSetGeographicReferencePtr&& ref)
{
    _reference = std::move(ref);
}
}
}
