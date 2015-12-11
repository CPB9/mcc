/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include <QString>

namespace mcc {

    class Names
    {
    public:
        static const char* multicast()      { return "~"; }
        static const char* coreDb()         { return "mcc.core.db";        }
        static const char* coreCmd()        { return "mcc.core.cmd"; }
        static const char* coreTm()         { return "mcc.core.tm"; }
        static const char* coreManager()    { return "mcc.core.manager"; }
        static const char* coreRouter()     { return "mcc.core.router"; }
        static const char* ui()             { return "mcc.ui"; }
        static const char* model()          { return "mcc.model"; }
        static const char* model1()         { return "mcc.model1"; }
        static const char* model2()         { return "mcc.model2"; }
        static const char* model3()         { return "mcc.model3"; }
        static const char* model4()         { return "mcc.model4"; }
        static const char* encoderInternal(){ return "mcc.encoder.internal"; }
        static const char* encoderMavlink() { return "mcc.encoder.mavlink"; }
        static const char* encoderPhoton()  { return "mcc.encoder.photon"; }
        static const char* java()           { return "mcc.java"; }
        static const char* python()         { return "mcc.python"; }
    };
}
