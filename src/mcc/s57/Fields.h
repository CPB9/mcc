/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "mcc/s57/Directory.h"
#include "mcc/s57/Enums.h"

#include <cstdint>
#include <string>
#include <ctime>

namespace mcc {
namespace s57 {

struct TopLevelIdentifier {
    RecordName recordName;
    std::uint32_t recordId;
};

struct DataSetIdentification {
    TopLevelIdentifier id;
    ExchangePurpose exchangePurpose;
    std::uint8_t indendedUsage;
    std::string dataSetName;
    std::string editionNumber;
    std::string updateNumber;
    std::tm updateApplicationDate;
    std::tm issueDate;
    std::uint8_t s57editionMajor;
    std::uint8_t s57editionMinor;
    ProductSpecification productSpec;
    std::string productSpecDescription;
    std::string productSpecEditionNumber;
    ApplicationProfileIdentification apid;
    std::uint16_t agencyCode;
    std::string comment;
};

struct DataSetStructureInformation {
    DataStructure dataStructure;
    std::uint8_t attfLexicalLevel;
    std::uint8_t natfLexicalLevel;
    std::uint32_t metaRecordNumber;
    std::uint32_t catrographicRecordNumber;
    std::uint32_t geoRecordNumber;
    std::uint32_t collectionRecordNumber;
    std::uint32_t isolatedNodeRecordNumber;
    std::uint32_t connectedNodeRecordNumber;
    std::uint32_t edgeRecordNumber;
    std::uint32_t faceRecordNumber;
};

struct DataSetParameter {
    TopLevelIdentifier id;
    std::uint8_t horizontalGeodeticDatum;
    std::uint8_t verticalDatum;
    std::uint8_t soundingDatum;
    std::uint32_t dataCompilationScale;
    std::uint8_t unitsOfDepth;
    std::uint8_t unitsOfHeigth;
    std::uint8_t positionalAccuracy;
    CoordinateUnits coordinateUnits;
    std::uint32_t coordinateMultFactor;
    std::uint32_t soundingMultFactor;
    std::string comment;
};

struct VectorRecordIdentifier {
    TopLevelIdentifier id;
    std::uint16_t version;
    UpdateInstruction updateInstruction;
};

struct VectorRecordAttribute {
    std::uint16_t code;
    std::string value;
};

struct VectorRecordPointerControl {
    UpdateInstruction updateInstruction;
    std::uint32_t pointerIndex;
    std::uint32_t pointerNum;
};

struct VectorRecordPointer {
    TopLevelIdentifier name;
    Orientation orientation;
    UsageIndicator usageIndicator;
    TopologyIndicator topologyIndicator;
    MaskingIndicator maskingIndicator;
};

struct CoordinateControl {
    UpdateInstruction updateInstruction;
    std::uint32_t coordinateIndex;
    std::uint32_t coordinateNum;
};

struct Coordinate2D {
    std::int32_t y;
    std::int32_t x;
};

struct Coordinate3D {
    std::int32_t y;
    std::int32_t x;
    std::int32_t sounding;
};

struct FeatureRecordIdentifier {
    TopLevelIdentifier id;
    ObjectGeometricPrimitive primitive;
    std::uint8_t group;
    std::uint16_t objectLabel;
    std::uint16_t recordVersion;
    UpdateInstruction updateInstruction;
};

struct FeatureObjectIdentifier {
    std::uint16_t producingAgency;
    std::uint32_t featureId;
    std::uint16_t featureIdSubdivision;
};

struct FeatureRecordAttribute {
    std::uint32_t code;
    std::string value;
};

struct FeatureRecordNationalAttribute {
    std::uint32_t code;
    std::string value;
};

struct FeatureRecordToFeatureObjectPointer {
    FeatureObjectIdentifier longName;
    RelationshipIndicator relationshipIndicator;
    std::string comment;
};

struct FeatureRecordToSpatialObjectPointer {
    TopLevelIdentifier name;
    Orientation orientation;
    UsageIndicator usageIndicator;
    MaskingIndicator maskingIndicator;
};
}
}
