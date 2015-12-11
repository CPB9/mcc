/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "mcc/s57/Ptr.h"
#include "mcc/s57/ParseError.h"
#include "mcc/s57/ParseState.h"
#include "mcc/s57/Fields.h"
#include "mcc/s57/File.h"
#include "mcc/s57/Record.h"
#include "mcc/s57/RecordDescriptor.h"
#include "mcc/misc/Result.h"
#include "bmcl/Reader.h"

#include <array>
#include <stack>

namespace bmcl {
class MemReader;
}

namespace mcc {
namespace s57 {

template <typename T>
using ParseResult = bmcl::Result<T, ParseError>;

class Parser {
public:
    ParseResult<BaseCellFilePtr> parseBaseCellFile(bmcl::MemReader* reader);
    const std::stack<ParseState>& state() const;
    const std::vector<ParseState>& history() const;

private:
    // record header
    ParseResult<Directory> parseDirectory(bmcl::MemReader* reader);
    ParseResult<Leader> parseDdrLeader(bmcl::MemReader* reader);
    ParseResult<Leader> parseDrLeader(bmcl::MemReader* reader);
    ParseError parseNextDdrHeader(bmcl::MemReader* reader);
    ParseError parseNextDrHeader(bmcl::MemReader* reader);
    ParseResult<Leader> parseLeader(bmcl::MemReader* reader, const std::array<char, 7>& expectedStamp,
                                    const std::array<char, 3>& expectedIndicator);

    // records
    ParseResult<DataSetGeneralInfoPtr> parseDataSetGeneralInfo(bmcl::MemReader* reader);
    ParseResult<DataSetGeographicReferencePtr> parseDataSetGeographicReference(bmcl::MemReader* reader);
    ParseResult<VectorRecordPtr> parseVectorRecord(bmcl::MemReader* reader);
    ParseResult<FeatureRecordPtr> parseFeatureRecord(bmcl::MemReader* reader);

    // fields
    ParseResult<DataSetIdentification> parseDsid(bmcl::MemReader* reader);
    ParseResult<DataSetStructureInformation> parseDssi(bmcl::MemReader* reader);
    ParseResult<DataSetParameter> parseDspm(bmcl::MemReader* reader);
    ParseResult<VectorRecordIdentifier> parseVrid(bmcl::MemReader* reader);
    ParseResult<std::vector<VectorRecordAttribute>> parseAttv(bmcl::MemReader* reader);
    ParseResult<std::vector<VectorRecordPointer>> parseVrpt(bmcl::MemReader* reader);
    ParseResult<std::vector<Coordinate2D>> parseSg2d(bmcl::MemReader* reader);
    ParseResult<std::vector<Coordinate3D>> parseSg3d(bmcl::MemReader* reader);
    ParseResult<FeatureRecordIdentifier> parseFrid(bmcl::MemReader* reader);
    ParseResult<FeatureObjectIdentifier> parseFoid(bmcl::MemReader* reader);
    ParseResult<std::vector<FeatureRecordAttribute>> parseAttf(bmcl::MemReader* reader);
    ParseResult<std::vector<FeatureRecordNationalAttribute>> parseNatf(bmcl::MemReader* reader);
    ParseResult<std::vector<FeatureRecordToFeatureObjectPointer>> parseFfpt(bmcl::MemReader* reader);
    ParseResult<std::vector<FeatureRecordToSpatialObjectPointer>> parseFspt(bmcl::MemReader* reader);

    // enums
    ParseResult<RecordName> skipExpectedRecordName(bmcl::MemReader* reader, RecordName name) const;
    ParseResult<RecordName> parseRecordName(bmcl::MemReader* reader) const;
    ParseResult<UpdateInstruction> parseUpdateInstruction(bmcl::MemReader* reader) const;
    ParseResult<Orientation> parseOrientation(bmcl::MemReader* reader) const;
    ParseResult<UsageIndicator> parseUsageIndicator(bmcl::MemReader* reader) const;
    ParseResult<TopologyIndicator> parseTopologyIndicator(bmcl::MemReader* reader) const;
    ParseResult<MaskingIndicator> parseMaskingIndicator(bmcl::MemReader* reader) const;
    ParseResult<ExchangePurpose> parseExchangePurpose(bmcl::MemReader* reader) const;
    ParseResult<ProductSpecification> parseProductSpecification(bmcl::MemReader* reader) const;
    ParseResult<ApplicationProfileIdentification> parseApplicationProfileId(bmcl::MemReader* reader) const;
    ParseResult<DataStructure> parseDataStructure(bmcl::MemReader* reader) const;
    ParseResult<CoordinateUnits> parseCoordinateUnits(bmcl::MemReader* reader) const;
    ParseResult<ObjectGeometricPrimitive> parseObjectGeometricPrimitive(bmcl::MemReader* reader) const;
    ParseResult<RelationshipIndicator> parseRelationshipIndicator(bmcl::MemReader* reader) const;

    // helpers
    ParseResult<unsigned long> parseTextualValue(bmcl::MemReader* reader, std::size_t size, int base = 10) const;
    ParseResult<std::uint16_t> parseRecordId(bmcl::MemReader* reader);
    ParseResult<std::uint16_t> parseRecordIdNumber16(bmcl::MemReader* reader) const;
    ParseResult<std::uint32_t> parseRecordIdNumber32(bmcl::MemReader* reader) const;
    ParseResult<std::string> parseString(bmcl::MemReader* reader) const;
    ParseResult<std::tm> parseDateTime(bmcl::MemReader* reader) const;
    ParseResult<TopLevelIdentifier> parseTopLevelIdentifier(bmcl::MemReader* reader) const;

    bool currentFieldDescAvailable(const char* name) const;
    template <typename T, typename C>
    ParseResult<std::vector<T>> parseFixedSizeArray(bmcl::MemReader* reader, const char* name, std::size_t chunkSize,
                                                    ParseState state, C parseFunc);
    template <typename T, typename C>
    ParseResult<std::vector<T>> parseVariableSizeArray(bmcl::MemReader* reader, const char* name, ParseState state, C parseFunc);
    template <typename T>
    ParseResult<std::vector<T>> parseAttributes(bmcl::MemReader* reader, const char* name, ParseState state);
    template <typename T, typename C>
    ParseResult<std::vector<T>> parseRepeatingRecord(bmcl::MemReader* reader, const char* name, C parseFunc);

    void pushState(ParseState state);
    void popState();

    Leader _currentLeader;
    Directory _currentDir;
    std::stack<ParseState> _state;
    std::vector<ParseState> _history;
    std::size_t _currentId;
};
}
}
