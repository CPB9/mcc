/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <QCoreApplication>
#include <QFile>

#include "bmcl/Assert.h"

#include "mcc/core/decode/Sqlite3RegistryProvider.h"

namespace mcc {

namespace decode {

class Sqlite3RegistryProviderSingleton {

public:
    static Registry* getInstance()
    {
        static std::unique_ptr<Registry> instance(provide().release());
        return instance.get();
    }

private:
    Sqlite3RegistryProviderSingleton() {}

    Sqlite3RegistryProviderSingleton(Sqlite3RegistryProviderSingleton const& o) = delete;
    Sqlite3RegistryProviderSingleton& operator=(Sqlite3RegistryProviderSingleton const& o) = delete;

    static std::unique_ptr<Registry> provide()
    {
        QString localDbPath = QCoreApplication::applicationDirPath() + "/local.sqlite";
        if (!QFile(localDbPath).exists())
        {
            BMCL_ASSERT_MSG(false, "local.sqlite not found");
        }
        Sqlite3RegistryProvider registryProvider(localDbPath.toStdString());
        return registryProvider.provide();
    }

};

std::shared_ptr<Component> findDecodeComponentOrFail(std::string componentFqn);

}
}

