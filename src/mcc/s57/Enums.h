/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <cstdint>

namespace mcc {
namespace s57 {

enum class RecordName : std::uint8_t {
    DataSetGeneralInformation = 10, //
    DataSetGeographicReference = 20,
    DataSetHistory = 30,
    DataSetAccuracy = 40,
    CatalogueDirectory = 50,
    CatalogueCrossReference = 60,
    DataDictionaryDefinition = 70,
    DataDictionaryDomain = 80,
    DataDictionarySchema = 90,
    Feature = 100,
    IsolatedNode = 110,
    ConnectedNode = 120,
    Edge = 130,
    Face = 140
};

enum class UpdateInstruction : std::uint8_t {
    Insert = 1, //
    Delete = 2,
    Modify = 3
};

enum class Orientation : std::uint8_t {
    Forward = 1, //
    Reverse = 2,
    Null = 255
};

enum class UsageIndicator : std::uint8_t {
    Exterior = 1, //
    Interior = 2,
    ExteriorTruncated = 3,
    Null = 255
};

enum class TopologyIndicator : std::uint8_t {
    BeginningNode = 1,
    EndNode = 2,
    LeftFace = 3,
    RightFace = 4,
    ContainingFace = 5,
    Null = 255
};

enum class MaskingIndicator : std::uint8_t {
    Mask = 1, //
    Show = 2,
    Null = 255
};

enum class ExchangePurpose : std::uint8_t {
    New = 1, //
    Revision = 2,
};

enum class ProductSpecification : std::uint8_t {
    ElectronicNavigationalChart = 1, //
    IhoObjectCatalogueDataDictionary = 2,
};

enum class ApplicationProfileIdentification : std::uint8_t {
    EncNew = 1, //
    EncUpdate = 2,
    IhoDataDictionary = 3,
};

enum class DataStructure : std::uint8_t {
    CartographicSpaghetti = 1,
    ChainNode = 2,
    PlanarGraph = 3,
    FullTopology = 4,
    TopologyIsNotRelevant = 255,
};

enum class CoordinateUnits : std::uint8_t {
    LatitudeLongitude = 1, //
    EastingNorthing = 2,
    UnitsOfChartMap = 3,
};

enum class ObjectGeometricPrimitive : std::uint8_t {
    Point = 1, //
    Line = 2,
    Area = 3,
    NoReference = 255,
};

enum class RelationshipIndicator : std::uint8_t {
    Master = 1, //
    Slave = 2,
    Peer = 3,
};
}
}
