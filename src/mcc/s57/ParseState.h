/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

namespace mcc {
namespace s57 {

enum class ParseState {
    DrRecord,
    DdrRecord,
    Leader,
    Directory,

    DsgiRecordData,
    DsgrRecordData,
    VectorRecordData,
    FeatureRecordData,

    Dsid,
    Dssi,
    Dspm,
    Vrid,
    Attv,
    Vrpt,
    Sg2d,
    Sg3d,
    Frid,
    Foid,
    Attf,
    Natf,
    Ffpt,
    Fspt
};

}
}
