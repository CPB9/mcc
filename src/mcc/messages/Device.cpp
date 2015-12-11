/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mcc/messages/Device.h"

namespace mcc {
namespace messages {

MESSAGE_REQUEREMENT_DEFINITIONS(DeviceActivate_Request);
MESSAGE_REQUEREMENT_DEFINITIONS(DeviceActivate_Response);
MESSAGE_REQUEREMENT_DEFINITIONS(DeviceConnect_Request);
MESSAGE_REQUEREMENT_DEFINITIONS(DeviceConnect_Response);
MESSAGE_REQUEREMENT_DEFINITIONS(DeviceDisconnect_Request);
MESSAGE_REQUEREMENT_DEFINITIONS(DeviceDisconnect_Response);
MESSAGE_REQUEREMENT_DEFINITIONS(DeviceRegister_Request);
MESSAGE_REQUEREMENT_DEFINITIONS(DeviceRegister_Response);
MESSAGE_REQUEREMENT_DEFINITIONS(DeviceRegistered);
MESSAGE_REQUEREMENT_DEFINITIONS(DeviceUnRegistered);
MESSAGE_REQUEREMENT_DEFINITIONS(DeviceUpdate_Request);
MESSAGE_REQUEREMENT_DEFINITIONS(DeviceUpdate_Response);
MESSAGE_REQUEREMENT_DEFINITIONS(DeviceUpdated);
MESSAGE_REQUEREMENT_DEFINITIONS(DeviceUnRegister_Request);
MESSAGE_REQUEREMENT_DEFINITIONS(DeviceUnRegister_Response);
MESSAGE_REQUEREMENT_DEFINITIONS(DeviceFileLoad_Request);
MESSAGE_REQUEREMENT_DEFINITIONS(DeviceFileLoadCancel_Request);
MESSAGE_REQUEREMENT_DEFINITIONS(DeviceFileLoad_Response);
MESSAGE_REQUEREMENT_DEFINITIONS(DeviceList_Request);
MESSAGE_REQUEREMENT_DEFINITIONS(DeviceList_Response);
MESSAGE_REQUEREMENT_DEFINITIONS(DeviceDescription_Request);
MESSAGE_REQUEREMENT_DEFINITIONS(DeviceDescription_Response);
MESSAGE_REQUEREMENT_DEFINITIONS(DeviceState_Response);
MESSAGE_REQUEREMENT_DEFINITIONS(DeviceActionLog);

}
}

