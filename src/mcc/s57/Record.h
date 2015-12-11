/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "mcc/s57/Fields.h"
#include "mcc/s57/Directory.h"
#include "mcc/s57/Ptr.h"
#include "mcc/misc/Either.h"
#include "mcc/misc/Option.h"

#include <cstdint>
#include <vector>
#include <array>

namespace mcc {
namespace s57 {

class Parser;

class Record {
public:
    std::uint16_t id() const;

protected:
    Record(std::uint16_t id);

private:
    std::uint16_t _id;
};

inline Record::Record(std::uint16_t id)
    : _id(id)
{
}

inline std::uint16_t Record::id() const
{
    return _id;
}

class DataSetGeneralInfo : public Record {
private:
    friend class Parser;
    DataSetGeneralInfo(DataSetIdentification&& dsid, DataSetStructureInformation&& dssi, std::uint16_t id);
    static DataSetGeneralInfoPtr create(DataSetIdentification&& dsid, DataSetStructureInformation&& dssi,
                                        std::uint16_t id);

    DataSetIdentification _dsid;
    DataSetStructureInformation _dssi;
};

inline DataSetGeneralInfo::DataSetGeneralInfo(DataSetIdentification&& dsid, DataSetStructureInformation&& dssi,
                                              std::uint16_t id)
    : Record(id)
    , _dsid(std::move(dsid))
    , _dssi(std::move(dssi))
{
}

inline DataSetGeneralInfoPtr DataSetGeneralInfo::create(DataSetIdentification&& dsid,
                                                        DataSetStructureInformation&& dssi, std::uint16_t id)
{
    return DataSetGeneralInfoPtr(new DataSetGeneralInfo(std::move(dsid), std::move(dssi), id));
}

class DataSetGeographicReference : public Record {
private:
    friend class Parser;
    DataSetGeographicReference(DataSetParameter&& dspm, std::uint16_t id);
    static DataSetGeographicReferencePtr create(DataSetParameter&& dspm, std::uint16_t id);

    DataSetParameter _dspm;
};

inline DataSetGeographicReference::DataSetGeographicReference(DataSetParameter&& dspm, std::uint16_t id)
    : Record(id)
    , _dspm(std::move(dspm))
{
}

inline DataSetGeographicReferencePtr DataSetGeographicReference::create(DataSetParameter&& dspm, std::uint16_t id)
{
    return DataSetGeographicReferencePtr(new DataSetGeographicReference(std::move(dspm), id));
}

class VectorRecord : public Record {
public:
    bool hasCoordinates() const;
    bool hasPointers() const;
    bool hasAttributes() const;
    bool is2d() const;
    bool is3d() const;

private:
    friend class Parser;
    VectorRecord(std::uint16_t id);
    static VectorRecordPtr create(std::uint16_t id);

    void setVectorRecordIdentifier(VectorRecordIdentifier&& vrid);
    void setAttributes(std::vector<VectorRecordAttribute>&& attributes);
    void setPointers(std::vector<VectorRecordPointer>&& pointers);

    template <typename T>
    void setCoordinates(std::vector<T>&& coords);

    VectorRecordIdentifier _vrid;
    std::vector<VectorRecordAttribute> _attributes;
    std::vector<VectorRecordPointer> _pointers;
    misc::Option<misc::Either<std::vector<Coordinate2D>, std::vector<Coordinate3D>>> _coordinates;
};

inline VectorRecord::VectorRecord(std::uint16_t id)
    : Record(id)
{
}

inline VectorRecordPtr VectorRecord::create(std::uint16_t id)
{
    return VectorRecordPtr(new VectorRecord(id));
}

inline void VectorRecord::setAttributes(std::vector<VectorRecordAttribute>&& attributes)
{
    _attributes = std::move(attributes);
}

inline void VectorRecord::setPointers(std::vector<VectorRecordPointer>&& pointers)
{
    _pointers = std::move(pointers);
}

inline void VectorRecord::setVectorRecordIdentifier(VectorRecordIdentifier&& vrid)
{
    _vrid = std::move(vrid);
}

template <typename T>
inline void VectorRecord::setCoordinates(std::vector<T>&& coords)
{
    _coordinates.emplace(std::move(coords));
}

inline bool VectorRecord::is2d() const
{
    return _coordinates.unwrap().isFirst();
}

inline bool VectorRecord::is3d() const
{
    return _coordinates.unwrap().isSecond();
}

class FeatureRecord : public Record {
private:
    friend class Parser;
    FeatureRecord(std::uint16_t id);
    static FeatureRecordPtr create(std::uint16_t id);

    void setFeatureRecordIdentifier(FeatureRecordIdentifier&& frid);
    void setFeatureObjectIdentifier(FeatureObjectIdentifier&& foid);
    void setAttributes(std::vector<FeatureRecordAttribute>&& attributes);
    void setNationalAttributes(std::vector<FeatureRecordNationalAttribute>&& attributes);
    void setFeatureObjectPointers(std::vector<FeatureRecordToFeatureObjectPointer>&& pointers);
    void setSpatialRecordPointers(std::vector<FeatureRecordToSpatialObjectPointer>&& pointers);

    FeatureRecordIdentifier _frid;
    FeatureObjectIdentifier _foid;
    std::vector<FeatureRecordAttribute> _attributes;
    std::vector<FeatureRecordNationalAttribute> _nationalAttributes;
    std::vector<FeatureRecordToFeatureObjectPointer> _featureObjectPointers;
    std::vector<FeatureRecordToSpatialObjectPointer> _spatialRecordPointers;
};

inline FeatureRecord::FeatureRecord(std::uint16_t id)
    : Record(id)
{
}

inline FeatureRecordPtr FeatureRecord::create(std::uint16_t id)
{
    return FeatureRecordPtr(new FeatureRecord(id));
}

inline void FeatureRecord::setFeatureRecordIdentifier(FeatureRecordIdentifier&& frid)
{
    _frid = std::move(frid);
}

inline void FeatureRecord::setFeatureObjectIdentifier(FeatureObjectIdentifier&& foid)
{
    _foid = std::move(foid);
}

inline void FeatureRecord::setAttributes(std::vector<FeatureRecordAttribute>&& attributes)
{
    _attributes = std::move(attributes);
}

inline void FeatureRecord::setNationalAttributes(std::vector<FeatureRecordNationalAttribute>&& attributes)
{
    _nationalAttributes = std::move(attributes);
}

inline void FeatureRecord::setFeatureObjectPointers(std::vector<FeatureRecordToFeatureObjectPointer>&& pointers)
{
    _featureObjectPointers = std::move(pointers);
}

inline void FeatureRecord::setSpatialRecordPointers(std::vector<FeatureRecordToSpatialObjectPointer>&& pointers)
{
    _spatialRecordPointers = std::move(pointers);
}
}
}
