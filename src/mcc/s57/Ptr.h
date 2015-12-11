/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <memory>

namespace mcc {
namespace s57 {

class BaseCellFile;
typedef std::unique_ptr<BaseCellFile> BaseCellFilePtr;

class ParseTree;
typedef std::shared_ptr<ParseTree> ParseTreePtr;

class DataSetGeneralInfo;
typedef std::unique_ptr<DataSetGeneralInfo> DataSetGeneralInfoPtr;

class DataSetGeographicReference;
typedef std::unique_ptr<DataSetGeographicReference> DataSetGeographicReferencePtr;

class VectorRecord;
typedef std::unique_ptr<VectorRecord> VectorRecordPtr;

class FeatureRecord;
typedef std::unique_ptr<FeatureRecord> FeatureRecordPtr;
}
}
