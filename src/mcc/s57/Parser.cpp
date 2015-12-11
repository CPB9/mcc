/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mcc/s57/Parser.h"
#include "mcc/misc/Helpers.h"
#include "bmcl/MemReader.h"

#include <algorithm>
#include <cstring>
#include <cstdint>
#include <ctime>
#include <iostream>

#ifdef _WIN32
#include <iomanip>
#include <sstream>
#include <malloc.h>
#define s57_alloca _alloca
#else
#include <time.h>
#include <alloca.h>
#define s57_alloca alloca
#endif

#define EXPECT_TERMINATOR(reader, terminator)                                                                          \
    do {                                                                                                               \
        if (reader->readUint8() != terminator) {                                                                       \
            return ParseError::InvalidDirectorySeparator;                                                              \
        }                                                                                                              \
    } while (0);

#define EXPECT_NEXT_FIELD(name)                                                                                        \
    do {                                                                                                               \
        if (_currentDir.isEmpty()) {                                                                                   \
            return ParseError::InvalidField;                                                                           \
        }                                                                                                              \
        if (_currentDir.currentField().nameIs(name)) {                                                                 \
            _currentDir.popFront();                                                                                    \
        } else {                                                                                                       \
            return ParseError::InvalidField;                                                                           \
        }                                                                                                              \
    } while (0);

#define EXPECT_EMPTY_FIELD()                                                                                           \
    do {                                                                                                               \
        if (!_currentDir.isEmpty()) {                                                                                  \
            return ParseError::InvalidField;                                                                           \
        }                                                                                                              \
    } while (0);

#define EXPECT_FIELD_TERMINATOR(reader) EXPECT_TERMINATOR(reader, fieldTerminator)

#define EXPECT_UNIT_TERMINATOR(reader) EXPECT_TERMINATOR(reader, unitTerminator)

#define EXPECT_MINUMIM_SIZE(reader, size)                                                                              \
    do {                                                                                                               \
        if (reader->readableSize() < size) {                                                                           \
            return ParseError::UnexpectedEof;                                                                          \
        }                                                                                                              \
    } while (0);

// TODO: check EOF

namespace mcc {
namespace s57 {

using bmcl::MemReader;

static const int leaderSize = 24;
static const std::uint8_t fieldTerminator = 0x1e;
static const std::uint8_t unitTerminator = 0x1f;

static const std::array<char, 7> ddrRecordStamp{{'3', 'L', 'E', '1', ' ', '0', '9'}};
static const std::array<char, 3> ddrRecordIndicator{{' ', '!', ' '}};
static const std::array<char, 7> drRecordStamp{{' ', 'D', ' ', ' ', ' ', ' ', ' '}};
static const std::array<char, 3> drRecordIndicator{{' ', ' ', ' '}};

void Parser::pushState(ParseState state)
{
    _state.push(state);
    _history.push_back(state);
}

void Parser::popState()
{
    _state.pop();
}

ParseResult<std::string> Parser::parseString(MemReader* reader) const
{
    std::string str;
    std::size_t sizeLeft = reader->readableSize();
    std::uint8_t value;
    while (true) {
        if (sizeLeft == 0) {
            return ParseError::UnexpectedEof;
        }
        value = reader->readUint8();
        if (value == unitTerminator) {
            break;
        }
        str.push_back(value);
        sizeLeft--;
    }
    return std::move(str);
}

template <typename T>
inline bool hasValidRange(T value)
{
    static_assert(!std::numeric_limits<T>::is_signed, "");
    return value != 0 && value != std::numeric_limits<T>::max();
}

ParseResult<std::uint16_t> Parser::parseRecordIdNumber16(MemReader* reader) const
{
    std::uint16_t id = reader->readUint16Le();
    if (!hasValidRange(id)) {
        return ParseError::InvalidRecordIdNumber;
    }
    return id;
}

ParseResult<std::uint32_t> Parser::parseRecordIdNumber32(MemReader* reader) const
{
    std::uint32_t id = reader->readUint32Le();
    if (!hasValidRange(id)) {
        return ParseError::InvalidRecordIdNumber;
    }
    return id;
}

ParseResult<unsigned long> Parser::parseTextualValue(MemReader* reader, std::size_t size, int base) const
{
    EXPECT_MINUMIM_SIZE(reader, size);
    char* tmp = (char*)s57_alloca(size + 1);
    reader->read(tmp, size);
    tmp[size] = '\0';
    char* end;
    unsigned long value = std::strtoul(tmp, &end, base);
    if (end != &tmp[size]) {
        return ParseError::InvalidTextualValue;
    }
    return value;
}

ParseResult<std::uint16_t> Parser::parseRecordId(MemReader* reader)
{
    EXPECT_NEXT_FIELD("0001");
    EXPECT_MINUMIM_SIZE(reader, 2);
    std::uint16_t id = reader->readUint16Le();
    //     if (id != expectedId) {
    //         return ParseError::InvalidRecordIdentifier;
    //     }
    EXPECT_FIELD_TERMINATOR(reader);
    return id;
}

ParseResult<std::tm> Parser::parseDateTime(MemReader* reader) const
{
    EXPECT_MINUMIM_SIZE(reader, 8);
    char tmp[9];
    tmp[8] = '\0';
    reader->read(tmp, 8);
    std::tm time;
    std::memset(&time, 0, sizeof(time));
    const char* format = "%Y%m%d";
#ifdef _WIN32
    std::stringstream ss(tmp);
    ss >> std::get_time(&time, format);
    if (ss.fail() || !ss.eof()) {
        return ParseError::InvalidDateTimeFormat;
    }
#else
    char* end = strptime(tmp, format, &time);
    if (end != &tmp[8]) {
        return ParseError::InvalidDateTimeFormat;
    }
#endif
    return time;
}

ParseResult<TopLevelIdentifier> Parser::parseTopLevelIdentifier(MemReader* reader) const
{
    EXPECT_MINUMIM_SIZE(reader, 5);
    TopLevelIdentifier fp;
    MCC_SET_IF_RESULT(fp.recordName, parseRecordName(reader));
    MCC_SET_IF_RESULT(fp.recordId, parseRecordIdNumber32(reader));
    return fp;
}

ParseResult<RecordName> Parser::skipExpectedRecordName(MemReader* reader, RecordName name) const
{
    EXPECT_MINUMIM_SIZE(reader, 1);
    RecordName recordName = (RecordName)reader->readUint8();
    if (recordName != name) {
        return ParseError::InvalidRecordName;
    }
    return name;
}

ParseResult<RecordName> Parser::parseRecordName(MemReader* reader) const
{
    EXPECT_MINUMIM_SIZE(reader, 1);
    std::uint8_t name = reader->readUint8();
    switch (name) {
    case 10:
        return RecordName::DataSetGeneralInformation;
    case 20:
        return RecordName::DataSetGeographicReference;
    case 30:
        return RecordName::DataSetHistory;
    case 40:
        return RecordName::DataSetAccuracy;
    case 50:
        return RecordName::CatalogueDirectory;
    case 60:
        return RecordName::CatalogueCrossReference;
    case 70:
        return RecordName::DataDictionaryDefinition;
    case 80:
        return RecordName::DataDictionaryDomain;
    case 90:
        return RecordName::DataDictionarySchema;
    case 100:
        return RecordName::Feature;
    case 110:
        return RecordName::IsolatedNode;
    case 120:
        return RecordName::ConnectedNode;
    case 130:
        return RecordName::Edge;
    case 140:
        return RecordName::Face;
    }
    return ParseError::InvalidRecordName;
}

ParseResult<UpdateInstruction> Parser::parseUpdateInstruction(MemReader* reader) const
{
    EXPECT_MINUMIM_SIZE(reader, 1);
    std::uint8_t instr = reader->readUint8();
    switch (instr) {
    case 1:
        return UpdateInstruction::Insert;
    case 2:
        return UpdateInstruction::Delete;
    case 3:
        return UpdateInstruction::Modify;
    }
    return ParseError::InvalidUpdateInstruction;
}

ParseResult<DataStructure> Parser::parseDataStructure(MemReader* reader) const
{
    EXPECT_MINUMIM_SIZE(reader, 1);
    std::uint8_t instr = reader->readUint8();
    switch (instr) {
    case 1:
        return DataStructure::CartographicSpaghetti;
    case 2:
        return DataStructure::ChainNode;
    case 3:
        return DataStructure::PlanarGraph;
    case 4:
        return DataStructure::FullTopology;
    case 255:
        return DataStructure::TopologyIsNotRelevant;
    }
    return ParseError::InvalidDataStructure;
}

ParseResult<Orientation> Parser::parseOrientation(MemReader* reader) const
{
    EXPECT_MINUMIM_SIZE(reader, 1);
    std::uint8_t orientation = reader->readUint8();
    switch (orientation) {
    case 1:
        return Orientation::Forward;
    case 2:
        return Orientation::Reverse;
    case 255:
        return Orientation::Null;
    }
    return ParseError::InvalidOrientation;
}

ParseResult<UsageIndicator> Parser::parseUsageIndicator(MemReader* reader) const
{
    EXPECT_MINUMIM_SIZE(reader, 1);
    std::uint8_t indicator = reader->readUint8();
    switch (indicator) {
    case 1:
        return UsageIndicator::Exterior;
    case 2:
        return UsageIndicator::Interior;
    case 3:
        return UsageIndicator::ExteriorTruncated;
    case 255:
        return UsageIndicator::Null;
    }
    return ParseError::InvalidUsageIndicator;
}

ParseResult<MaskingIndicator> Parser::parseMaskingIndicator(MemReader* reader) const
{
    EXPECT_MINUMIM_SIZE(reader, 1);
    std::uint8_t indicator = reader->readUint8();
    switch (indicator) {
    case 1:
        return MaskingIndicator::Mask;
    case 2:
        return MaskingIndicator::Show;
    case 255:
        return MaskingIndicator::Null;
    }
    return ParseError::InvalidMaskingIndicator;
}

ParseResult<TopologyIndicator> Parser::parseTopologyIndicator(MemReader* reader) const
{
    EXPECT_MINUMIM_SIZE(reader, 1);
    std::uint8_t indicator = reader->readUint8();
    switch (indicator) {
    case 1:
        return TopologyIndicator::BeginningNode;
    case 2:
        return TopologyIndicator::EndNode;
    case 3:
        return TopologyIndicator::LeftFace;
    case 4:
        return TopologyIndicator::RightFace;
    case 5:
        return TopologyIndicator::ContainingFace;
    case 255:
        return TopologyIndicator::Null;
    }
    return ParseError::InvalidTopologyIndicator;
}

ParseResult<ExchangePurpose> Parser::parseExchangePurpose(MemReader* reader) const
{
    EXPECT_MINUMIM_SIZE(reader, 1);
    std::uint8_t ep = reader->readUint8();
    switch (ep) {
    case 1:
        return ExchangePurpose::New;
    case 2:
        return ExchangePurpose::Revision;
    }
    return ParseError::InvalidExchangePurpose;
}

ParseResult<ProductSpecification> Parser::parseProductSpecification(MemReader* reader) const
{
    EXPECT_MINUMIM_SIZE(reader, 1);
    std::uint8_t spec = reader->readUint8();
    switch (spec) {
    case 1:
        return ProductSpecification::ElectronicNavigationalChart;
    case 2:
        return ProductSpecification::IhoObjectCatalogueDataDictionary;
    }
    return ParseError::InvalidProductSpecification;
}

ParseResult<ApplicationProfileIdentification> Parser::parseApplicationProfileId(MemReader* reader) const
{
    EXPECT_MINUMIM_SIZE(reader, 1);
    std::uint8_t spec = reader->readUint8();
    switch (spec) {
    case 1:
        return ApplicationProfileIdentification::EncNew;
    case 2:
        return ApplicationProfileIdentification::EncUpdate;
    case 3:
        return ApplicationProfileIdentification::IhoDataDictionary;
    }
    return ParseError::InvalidApplicationProfileIdentification;
}

ParseResult<CoordinateUnits> Parser::parseCoordinateUnits(MemReader* reader) const
{
    EXPECT_MINUMIM_SIZE(reader, 1);
    std::uint8_t units = reader->readUint8();
    switch (units) {
    case 1:
        return CoordinateUnits::LatitudeLongitude;
    case 2:
        return CoordinateUnits::EastingNorthing;
    case 3:
        return CoordinateUnits::UnitsOfChartMap;
    }
    return ParseError::InvalidCoordinateUnits;
}

ParseResult<ObjectGeometricPrimitive> Parser::parseObjectGeometricPrimitive(MemReader* reader) const
{
    EXPECT_MINUMIM_SIZE(reader, 1);
    std::uint8_t ogp = reader->readUint8();
    switch (ogp) {
    case 1:
        return ObjectGeometricPrimitive::Point;
    case 2:
        return ObjectGeometricPrimitive::Line;
    case 3:
        return ObjectGeometricPrimitive::Area;
    case 255:
        return ObjectGeometricPrimitive::NoReference;
    }
    return ParseError::InvalidObjectGeometricPrimitive;
}

ParseResult<RelationshipIndicator> Parser::parseRelationshipIndicator(MemReader* reader) const
{
    EXPECT_MINUMIM_SIZE(reader, 1);
    std::uint8_t indicator = reader->readUint8();
    switch (indicator) {
    case 1:
        return RelationshipIndicator::Master;
    case 2:
        return RelationshipIndicator::Slave;
    case 3:
        return RelationshipIndicator::Peer;
    }
    return ParseError::InvalidRelationshipIndicator;
}

ParseResult<Directory> Parser::parseDirectory(MemReader* reader)
{
    pushState(ParseState::Directory);
    int directorySize = _currentLeader.fieldOffset - leaderSize - 1;
    int fieldDescSize = _currentLeader.fieldDescriptionSize();
    if (directorySize < fieldDescSize || directorySize % fieldDescSize) {
        return ParseError::InvalidDirectorySize;
    }
    EXPECT_MINUMIM_SIZE(reader, std::size_t(directorySize));

    Directory dir;
    dir.reserveFields(directorySize / fieldDescSize);
    while (directorySize > 0) {
        FieldDescription desc;
        reader->read(desc.name, 4);
        desc.name[4] = '\0';
        MCC_SET_IF_RESULT(desc.length, parseTextualValue(reader, _currentLeader.fieldLengthSize));
        MCC_SET_IF_RESULT(desc.offset, parseTextualValue(reader, _currentLeader.fieldPositionSize));
        dir.addField(desc);
        directorySize -= fieldDescSize;
    }

    popState();
    return std::move(dir);
}

ParseResult<Leader> Parser::parseLeader(MemReader* reader, const std::array<char, 7>& expectedStamp,
                                        const std::array<char, 3>& expectedIndicator)
{
    pushState(ParseState::Leader);
    EXPECT_MINUMIM_SIZE(reader, leaderSize);
    Leader leader;
    MCC_IF_RESULT (length, parseTextualValue(reader, 5)) {
        leader.length = length.take();
    }
    reader->read(leader.stamp, 7);
    leader.stamp[7] = '\0';
    if (std::memcmp(leader.stamp, expectedStamp.data(), 7) != 0) {
        return ParseError::InvalidStampInRecordLeader;
    }
    MCC_SET_IF_RESULT(leader.fieldOffset, parseTextualValue(reader, 5));

    std::array<char, 3> indicator;
    reader->read(indicator.data(), 3);
    if (indicator != expectedIndicator) {
        return ParseError::InvalidIndicatorInRecordLeader;
    }

    char fieldLengthSize = reader->readUint8();
    if (fieldLengthSize < '1' || fieldLengthSize > '9') {
        return ParseError::UnsupportedRecordFieldLengthLength;
    }
    leader.fieldLengthSize = fieldLengthSize - '0';

    char fieldPositionSize = reader->readUint8();
    if (fieldPositionSize < '1' || fieldPositionSize > '9') {
        return ParseError::UnsupportedRecordFieldPositionLength;
    }
    leader.fieldPositionSize = fieldPositionSize - '0';

    std::uint8_t reserved = reader->readUint8();
    if (reserved != '0') {
        return ParseError::InvalidReservedFieldInEntryMap;
    }

    leader.fieldTagSize = reader->readUint8() - '0';
    if (leader.fieldTagSize != 4) {
        return ParseError::UnsupportedRecordFieldTagLength;
    }

    popState();
    return std::move(leader);
}

ParseResult<Leader> Parser::parseDdrLeader(MemReader* reader)
{
    return parseLeader(reader, ddrRecordStamp, ddrRecordIndicator);
}

ParseResult<Leader> Parser::parseDrLeader(MemReader* reader)
{
    return parseLeader(reader, drRecordStamp, drRecordIndicator);
}

ParseError Parser::parseNextDdrHeader(MemReader* reader)
{
    MCC_SET_IF_RESULT(_currentLeader, parseDdrLeader(reader));
    MCC_SET_IF_RESULT(_currentDir, parseDirectory(reader));
    EXPECT_FIELD_TERMINATOR(reader);
    return ParseError::Ok;
}

ParseError Parser::parseNextDrHeader(MemReader* reader)
{
    MCC_SET_IF_RESULT(_currentLeader, parseDrLeader(reader));
    MCC_SET_IF_RESULT(_currentDir, parseDirectory(reader));
    EXPECT_FIELD_TERMINATOR(reader);
    return ParseError::Ok;
}

template <typename T, typename C>
ParseResult<std::vector<T>> Parser::parseRepeatingRecord(MemReader* reader, const char* name, C parseFunc)
{
    std::vector<T> values;
    if (_currentDir.fieldNum() < 2) {
        return ParseError::InvalidField;
    }

    if (!_currentDir.fieldAt(1).nameIs(name)) {
        return ParseError::InvalidField;
    }

    MCC_IF_RESULT (value, parseFunc(reader)) {
        values.push_back(value.take());
    }
    popState();
    while (reader->readableSize() != 0) {
        pushState(ParseState::DrRecord);
        ParseError rv = parseNextDrHeader(reader);
        if (rv != ParseError::Ok) {
            return rv;
        }
        if (_currentDir.fieldNum() < 2) {
            return ParseError::InvalidField;
        }

        if (!_currentDir.fieldAt(1).nameIs(name)) {
            break;
        }

        MCC_IF_RESULT (value, parseFunc(reader)) {
            values.push_back(value.take());
        }
        popState();
    }

    return std::move(values);
}

ParseResult<BaseCellFilePtr> Parser::parseBaseCellFile(MemReader* reader)
{
    _history.clear();
    _history.reserve(reader->readableSize() / 20); // история занимает примерно 5% от размера файла
    pushState(ParseState::DdrRecord);
    ParseError rv = parseNextDdrHeader(reader);
    if (rv != ParseError::Ok) {
        return rv;
    }

    std::size_t recordAreaSize = _currentLeader.length - _currentLeader.fieldOffset;
    EXPECT_MINUMIM_SIZE(reader, recordAreaSize);
    reader->skip(recordAreaSize);
    popState();

    rv = parseNextDrHeader(reader);
    if (rv != ParseError::Ok) {
        return rv;
    }

    BaseCellFilePtr file = BaseCellFilePtr(new BaseCellFile);
    MCC_IF_RESULT (info, parseDataSetGeneralInfo(reader)) {
        file->setDataSetGeneralInfo(info.take());
    }

    pushState(ParseState::DrRecord);
    rv = parseNextDrHeader(reader);
    if (rv != ParseError::Ok) {
        return rv;
    }

    MCC_IF_RESULT (ref, parseDataSetGeographicReference(reader)) {
        file->setDataSetGeographicReference(ref.take());
    }

    pushState(ParseState::DrRecord);
    rv = parseNextDrHeader(reader);
    if (rv != ParseError::Ok) {
        return rv;
    }

    MCC_IF_RESULT (vrecords, parseRepeatingRecord<VectorRecordPtr>(
                                 reader, "VRID", [this](MemReader* reader) { return parseVectorRecord(reader); })) {
        file->setVectorRecords(vrecords.take());
    }
    popState();

    MCC_IF_RESULT (frecords, parseRepeatingRecord<FeatureRecordPtr>(
                                 reader, "FRID", [this](MemReader* reader) { return parseFeatureRecord(reader); })) {
        file->setFeatureRecords(frecords.take());
    }

    assert(_state.empty());
    return std::move(file);
}

const std::stack<ParseState>& Parser::state() const
{
    return _state;
}

const std::vector<ParseState>& Parser::history() const
{
    return _history;
}

ParseResult<DataSetGeneralInfoPtr> Parser::parseDataSetGeneralInfo(MemReader* reader)
{
    pushState(ParseState::DsgiRecordData);
    std::uint16_t id;
    MCC_SET_IF_RESULT(id, parseRecordId(reader));

    ParseResult<DataSetIdentification> dsid = parseDsid(reader);
    if (dsid.isErr()) {
        return dsid.unwrapErr();
    }

    ParseResult<DataSetStructureInformation> dssi = parseDssi(reader);
    if (dssi.isErr()) {
        return dssi.unwrapErr();
    }
    EXPECT_EMPTY_FIELD();

    popState();
    return DataSetGeneralInfo::create(dsid.take(), dssi.take(), id);
}

ParseResult<DataSetIdentification> Parser::parseDsid(MemReader* reader)
{
    EXPECT_NEXT_FIELD("DSID");
    pushState(ParseState::Dsid);
    DataSetIdentification dsid;
    MCC_SET_IF_RESULT(dsid.id.recordName, skipExpectedRecordName(reader, RecordName::DataSetGeneralInformation));
    MCC_SET_IF_RESULT(dsid.id.recordId, parseRecordIdNumber32(reader));
    MCC_SET_IF_RESULT(dsid.exchangePurpose, parseExchangePurpose(reader));

    dsid.indendedUsage = reader->readUint8(); // TODO: check

    MCC_SET_IF_RESULT(dsid.dataSetName, parseString(reader));
    MCC_SET_IF_RESULT(dsid.editionNumber, parseString(reader));
    MCC_SET_IF_RESULT(dsid.updateNumber, parseString(reader));
    MCC_SET_IF_RESULT(dsid.updateApplicationDate, parseDateTime(reader));
    MCC_SET_IF_RESULT(dsid.issueDate, parseDateTime(reader));

    EXPECT_MINUMIM_SIZE(reader, 4);
    char version[4];
    reader->read(version, 4);
    if (std::memcmp(version, "03.1", 4) != 0) {
        return ParseError::InvalidS57VersionString;
    }
    dsid.s57editionMajor = 3;
    dsid.s57editionMinor = 1;

    MCC_SET_IF_RESULT(dsid.productSpec, parseProductSpecification(reader));
    MCC_SET_IF_RESULT(dsid.productSpecDescription, parseString(reader));
    MCC_SET_IF_RESULT(dsid.productSpecEditionNumber, parseString(reader));
    MCC_SET_IF_RESULT(dsid.apid, parseApplicationProfileId(reader));
    EXPECT_MINUMIM_SIZE(reader, 2);
    dsid.agencyCode = reader->readUint16Le();
    MCC_SET_IF_RESULT(dsid.comment, parseString(reader));
    EXPECT_FIELD_TERMINATOR(reader);

    popState();
    return std::move(dsid);
}

ParseResult<DataSetStructureInformation> Parser::parseDssi(MemReader* reader)
{
    EXPECT_NEXT_FIELD("DSSI");
    pushState(ParseState::Dssi);
    EXPECT_MINUMIM_SIZE(reader, 35);
    DataSetStructureInformation dssi;
    MCC_SET_IF_RESULT(dssi.dataStructure, parseDataStructure(reader));
    // TODO: check attf, natf
    dssi.attfLexicalLevel = reader->readUint8();
    dssi.natfLexicalLevel = reader->readUint8();
    dssi.metaRecordNumber = reader->readUint32Le();
    dssi.catrographicRecordNumber = reader->readUint32Le();
    dssi.geoRecordNumber = reader->readUint32Le();
    dssi.collectionRecordNumber = reader->readUint32Le();
    dssi.isolatedNodeRecordNumber = reader->readUint32Le();
    dssi.connectedNodeRecordNumber = reader->readUint32Le();
    dssi.edgeRecordNumber = reader->readUint32Le();
    dssi.faceRecordNumber = reader->readUint32Le();
    EXPECT_FIELD_TERMINATOR(reader);

    popState();
    return std::move(dssi);
}

ParseResult<DataSetGeographicReferencePtr> Parser::parseDataSetGeographicReference(MemReader* reader)
{
    pushState(ParseState::DsgrRecordData);

    std::uint16_t id;
    MCC_SET_IF_RESULT(id, parseRecordId(reader));

    ParseResult<DataSetParameter> dspm = parseDspm(reader);
    if (dspm.isErr()) {
        return dspm.unwrapErr();
    }
    EXPECT_EMPTY_FIELD();

    popState();
    return DataSetGeographicReference::create(dspm.take(), id);
}

ParseResult<DataSetParameter> Parser::parseDspm(MemReader* reader)
{
    EXPECT_NEXT_FIELD("DSPM");
    pushState(ParseState::Dspm);
    DataSetParameter dspm;
    MCC_SET_IF_RESULT(dspm.id.recordName, skipExpectedRecordName(reader, RecordName::DataSetGeographicReference));
    MCC_SET_IF_RESULT(dspm.id.recordId, parseRecordIdNumber32(reader));
    EXPECT_MINUMIM_SIZE(reader, 10);
    dspm.horizontalGeodeticDatum = reader->readUint8();
    dspm.verticalDatum = reader->readUint8();
    dspm.soundingDatum = reader->readUint8();
    dspm.dataCompilationScale = reader->readUint32Le();
    dspm.unitsOfDepth = reader->readUint8();
    dspm.unitsOfHeigth = reader->readUint8();
    dspm.positionalAccuracy = reader->readUint8();
    MCC_SET_IF_RESULT(dspm.coordinateUnits, parseCoordinateUnits(reader));
    EXPECT_MINUMIM_SIZE(reader, 8);
    dspm.coordinateMultFactor = reader->readUint32Le();
    dspm.soundingMultFactor = reader->readUint32Le();
    MCC_SET_IF_RESULT(dspm.comment, parseString(reader));
    EXPECT_FIELD_TERMINATOR(reader);

    popState();
    return std::move(dspm);
}

bool Parser::currentFieldDescAvailable(const char* name) const
{
    return !_currentDir.isEmpty() && _currentDir.currentField().nameIs(name);
}

ParseResult<VectorRecordPtr> Parser::parseVectorRecord(MemReader* reader)
{
    pushState(ParseState::VectorRecordData);

    std::uint16_t id;
    MCC_SET_IF_RESULT(id, parseRecordId(reader));

    VectorRecordPtr record = VectorRecord::create(id);
    MCC_IF_RESULT (vrid, parseVrid(reader)) {
        record->setVectorRecordIdentifier(vrid.take());
    }

    if (_currentDir.isEmpty()) {
        return ParseError::InvalidField;
    }

    if (currentFieldDescAvailable("ATTV")) {
        MCC_IF_RESULT (attributes, parseAttv(reader)) {
            record->setAttributes(attributes.take());
        }
    }

    if (currentFieldDescAvailable("VRPT")) {
        MCC_IF_RESULT (pointers, parseVrpt(reader)) {
            record->setPointers(pointers.take());
        }
    }

    if (currentFieldDescAvailable("SG2D")) {
        MCC_IF_RESULT (coords, parseSg2d(reader)) {
            record->setCoordinates(coords.take());
        }
    } else if (currentFieldDescAvailable("SG3D")) {
        MCC_IF_RESULT (coords, parseSg3d(reader)) {
            record->setCoordinates(coords.take());
        }
    }
    EXPECT_EMPTY_FIELD();

    popState();
    return std::move(record);
}

ParseResult<VectorRecordIdentifier> Parser::parseVrid(MemReader* reader)
{
    EXPECT_NEXT_FIELD("VRID");
    pushState(ParseState::Vrid);
    EXPECT_MINUMIM_SIZE(reader, 8);
    VectorRecordIdentifier vrid;
    std::uint8_t recordName = reader->readUint8();
    switch (recordName) {
    case 110:
        vrid.id.recordName = RecordName::IsolatedNode;
        break;
    case 120:
        vrid.id.recordName = RecordName::ConnectedNode;
        break;
    case 130:
        vrid.id.recordName = RecordName::Edge;
        break;
    case 140:
        vrid.id.recordName = RecordName::Face;
        break;
    default:
        return ParseError::InvalidRecordName;
    }

    MCC_SET_IF_RESULT(vrid.id.recordId, parseRecordIdNumber32(reader));
    vrid.version = reader->readUint16Le();
    MCC_SET_IF_RESULT(vrid.updateInstruction, parseUpdateInstruction(reader));
    EXPECT_FIELD_TERMINATOR(reader);

    popState();
    return std::move(vrid);
}

template <typename T, typename C>
ParseResult<std::vector<T>> Parser::parseVariableSizeArray(MemReader* reader, const char name[4], ParseState state,
                                                           C parseFunc)
{
    if (_currentDir.isEmpty()) {
        return ParseError::InvalidField;
    }
    if (!_currentDir.currentField().nameIs(name)) {
        return ParseError::InvalidField;
    }
    pushState(state);
    std::size_t len = _currentDir.currentField().length - 1;
    EXPECT_MINUMIM_SIZE(reader, len);
    _currentDir.popFront();

    std::vector<T> values;
    std::size_t readableSize = reader->readableSize();
    std::size_t sizeRead = 0;
    while (true) {
        T value;
        ParseError rv = parseFunc(&value);
        if (rv != ParseError::Ok) {
            return rv;
        }
        values.push_back(std::move(value));
        sizeRead = readableSize - reader->readableSize();
        if (sizeRead == len) {
            break;
        }
        if (sizeRead > len) {
            return ParseError::InvalidFieldSize;
        }
    }
    EXPECT_FIELD_TERMINATOR(reader);
    popState();
    return std::move(values);
}

template <typename T>
ParseResult<std::vector<T>> Parser::parseAttributes(MemReader* reader, const char* name, ParseState state)
{
    return parseVariableSizeArray<T>(reader, name, state, [this, reader](T* value) {
        EXPECT_MINUMIM_SIZE(reader, 2);
        value->code = reader->readUint16Le();
        MCC_SET_IF_RESULT(value->value, parseString(reader));
        return ParseError::Ok;
    });
}

ParseResult<std::vector<VectorRecordAttribute>> Parser::parseAttv(MemReader* reader)
{
    return parseAttributes<VectorRecordAttribute>(reader, "ATTV", ParseState::Attv);
}

template <typename T, typename C>
ParseResult<std::vector<T>> Parser::parseFixedSizeArray(MemReader* reader, const char* name, std::size_t chunkSize,
                                                        ParseState state, C parseFunc)
{
    if (_currentDir.isEmpty()) {
        return ParseError::InvalidField;
    }
    if (!_currentDir.currentField().nameIs(name)) {
        return ParseError::InvalidField;
    }
    pushState(state);
    std::size_t len = _currentDir.currentField().length - 1;
    if ((len < chunkSize) || ((len % chunkSize) != 0)) {
        return ParseError::InvalidFieldSize;
    }
    _currentDir.popFront();

    std::size_t count = len / chunkSize;
    std::vector<T> values;
    values.reserve(count);
    for (std::size_t i = 0; i < count; i++) {
        T value;
        ParseError rv = parseFunc(&value);
        if (rv != ParseError::Ok) {
            return rv;
        }
        values.push_back(std::move(value));
    }
    EXPECT_FIELD_TERMINATOR(reader);
    popState();
    return std::move(values);
}

ParseResult<std::vector<VectorRecordPointer>> Parser::parseVrpt(MemReader* reader)
{
    return parseFixedSizeArray<VectorRecordPointer>(
        reader, "VRPT", 9, ParseState::Vrpt, [this, reader](VectorRecordPointer* vrpt) {
            MCC_SET_IF_RESULT(vrpt->name, parseTopLevelIdentifier(reader));
            MCC_SET_IF_RESULT(vrpt->orientation, parseOrientation(reader));
            MCC_SET_IF_RESULT(vrpt->usageIndicator, parseUsageIndicator(reader));
            MCC_SET_IF_RESULT(vrpt->maskingIndicator, parseMaskingIndicator(reader));
            MCC_SET_IF_RESULT(vrpt->topologyIndicator, parseTopologyIndicator(reader));
            return ParseError::Ok;
        });
}

ParseResult<std::vector<Coordinate2D>> Parser::parseSg2d(MemReader* reader)
{
    return parseFixedSizeArray<Coordinate2D>(reader, "SG2D", 8, ParseState::Sg2d, [reader](Coordinate2D* coord) {
        coord->y = reader->readUint32Le();
        coord->x = reader->readUint32Le();
        return ParseError::Ok;
    });
}

ParseResult<std::vector<Coordinate3D>> Parser::parseSg3d(MemReader* reader)
{
    return parseFixedSizeArray<Coordinate3D>(reader, "SG3D", 12, ParseState::Sg3d, [reader](Coordinate3D* coord) {
        coord->y = reader->readUint32Le();
        coord->x = reader->readUint32Le();
        coord->sounding = reader->readUint32Le();
        return ParseError::Ok;
    });
}

ParseResult<FeatureRecordPtr> Parser::parseFeatureRecord(MemReader* reader)
{
    pushState(ParseState::FeatureRecordData);
    std::uint16_t id;
    MCC_SET_IF_RESULT(id, parseRecordId(reader));

    FeatureRecordPtr record = FeatureRecord::create(id);
    MCC_IF_RESULT (frid, parseFrid(reader)) {
        record->setFeatureRecordIdentifier(frid.take());
    }

    MCC_IF_RESULT (foid, parseFoid(reader)) {
        record->setFeatureObjectIdentifier(foid.take());
    }

    if (_currentDir.isEmpty()) {
        return ParseError::InvalidField;
    }

    if (currentFieldDescAvailable("ATTF")) {
        MCC_IF_RESULT (attributes, parseAttf(reader)) {
            record->setAttributes(attributes.take());
        }
    }

    if (currentFieldDescAvailable("NATF")) {
        MCC_IF_RESULT (attributes, parseNatf(reader)) {
            record->setNationalAttributes(attributes.take());
        }
    }

    if (currentFieldDescAvailable("FFPT")) {
        MCC_IF_RESULT (pointers, parseFfpt(reader)) {
            record->setFeatureObjectPointers(pointers.take());
        }
    }

    if (currentFieldDescAvailable("FSPT")) {
        MCC_IF_RESULT (pointers, parseFspt(reader)) {
            record->setSpatialRecordPointers(pointers.take());
        }
    }

    EXPECT_EMPTY_FIELD();
    popState();
    return std::move(record);
}

ParseResult<FeatureRecordIdentifier> Parser::parseFrid(MemReader* reader)
{
    EXPECT_NEXT_FIELD("FRID");
    pushState(ParseState::Frid);
    EXPECT_MINUMIM_SIZE(reader, 12);
    FeatureRecordIdentifier frid;
    MCC_SET_IF_RESULT(frid.id.recordName, skipExpectedRecordName(reader, RecordName::Feature));
    MCC_SET_IF_RESULT(frid.id.recordId, parseRecordIdNumber32(reader));
    MCC_SET_IF_RESULT(frid.primitive, parseObjectGeometricPrimitive(reader));
    frid.group = reader->readUint8();
    frid.objectLabel = reader->readUint16Le();
    frid.recordVersion = reader->readUint16Le();
    MCC_SET_IF_RESULT(frid.updateInstruction, parseUpdateInstruction(reader));
    EXPECT_FIELD_TERMINATOR(reader);
    popState();
    return std::move(frid);
}

ParseResult<FeatureObjectIdentifier> Parser::parseFoid(MemReader* reader)
{
    EXPECT_NEXT_FIELD("FOID");
    pushState(ParseState::Foid);
    EXPECT_MINUMIM_SIZE(reader, 8);
    FeatureObjectIdentifier foid;
    foid.producingAgency = reader->readUint16Le();
    MCC_SET_IF_RESULT(foid.featureId, parseRecordIdNumber32(reader));
    MCC_SET_IF_RESULT(foid.featureIdSubdivision, parseRecordIdNumber16(reader));
    EXPECT_FIELD_TERMINATOR(reader);
    popState();
    return std::move(foid);
}

ParseResult<std::vector<FeatureRecordAttribute>> Parser::parseAttf(MemReader* reader)
{
    return parseAttributes<FeatureRecordAttribute>(reader, "ATTF", ParseState::Attf);
}

ParseResult<std::vector<FeatureRecordNationalAttribute>> Parser::parseNatf(MemReader* reader)
{
    return parseAttributes<FeatureRecordNationalAttribute>(reader, "NATF", ParseState::Natf);
}

ParseResult<std::vector<FeatureRecordToFeatureObjectPointer>> Parser::parseFfpt(MemReader* reader)
{
    return parseVariableSizeArray<FeatureRecordToFeatureObjectPointer>(
        reader, "FFPT", ParseState::Ffpt, [this, reader](FeatureRecordToFeatureObjectPointer* ffpt) {
            EXPECT_MINUMIM_SIZE(reader, 8);
            ffpt->longName.producingAgency = reader->readUint16Le();
            MCC_SET_IF_RESULT(ffpt->longName.featureId, parseRecordIdNumber32(reader));
            MCC_SET_IF_RESULT(ffpt->longName.featureIdSubdivision, parseRecordIdNumber16(reader));
            MCC_SET_IF_RESULT(ffpt->relationshipIndicator, parseRelationshipIndicator(reader));
            MCC_SET_IF_RESULT(ffpt->comment, parseString(reader));
            return ParseError::Ok;
        });
}

ParseResult<std::vector<FeatureRecordToSpatialObjectPointer>> Parser::parseFspt(MemReader* reader)
{
    return parseFixedSizeArray<FeatureRecordToSpatialObjectPointer>(
        reader, "FSPT", 8, ParseState::Fspt, [this, reader](FeatureRecordToSpatialObjectPointer* fspt) {
            MCC_SET_IF_RESULT(fspt->name, parseTopLevelIdentifier(reader));
            MCC_SET_IF_RESULT(fspt->orientation, parseOrientation(reader));
            MCC_SET_IF_RESULT(fspt->usageIndicator, parseUsageIndicator(reader));
            MCC_SET_IF_RESULT(fspt->maskingIndicator, parseMaskingIndicator(reader));
            return ParseError::Ok;
        });
}
}
}
