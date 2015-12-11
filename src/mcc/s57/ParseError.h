/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

namespace mcc {
namespace s57 {

enum class ParseError {
    Ok = 0,
    OnlyOneBaseCellFileAllowed,
    CannotUpdateWithoutBaseCellFile,
    UnexpectedEof,
    InvalidFirstRecord,
    InvalidTextualValue,
    InvalidRecordSize,
    InvalidDirectorySize,
    InvalidDirectorySeparator,
    InvalidReservedFieldInEntryMap,
    InvalidStampInRecordLeader,
    InvalidIndicatorInRecordLeader,
    UnsupportedRecordFieldLengthLength,
    UnsupportedRecordFieldPositionLength,
    UnsupportedRecordFieldTagLength,
    InvalidRecordIdentifier,
    InvalidS57VersionString,
    InvalidField,
    InvalidFieldSize,
    InvalidDateTimeFormat,
    InvalidRecordIdNumber,
    InvalidRecordName,
    InvalidUpdateInstruction,
    InvalidOrientation,
    InvalidExchangePurpose,
    InvalidUsageIndicator,
    InvalidMaskingIndicator,
    InvalidTopologyIndicator,
    InvalidProductSpecification,
    InvalidApplicationProfileIdentification,
    InvalidDataStructure,
    InvalidCoordinateUnits,
    InvalidObjectGeometricPrimitive,
    InvalidRelationshipIndicator,
};
}
}
