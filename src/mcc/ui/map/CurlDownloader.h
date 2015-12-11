/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <string>

typedef void CURL;

namespace bmcl {
class Buffer;
}

namespace mcc {
namespace ui {
namespace map {

class CurlDownloader {
public:
    CurlDownloader();
    ~CurlDownloader();

    bmcl::Buffer download(const char* url);
    bmcl::Buffer download(const std::string& url);

private:
    CURL* _handle;
};
}
}
}
