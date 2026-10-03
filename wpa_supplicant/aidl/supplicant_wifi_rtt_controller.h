/*
 * Copyright (C) 2025 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef WPA_SUPPLICANT_AIDL_VENDOR_SUPPLICANT_WIFI_RTT_CONTROLLER_H
#define WPA_SUPPLICANT_AIDL_VENDOR_SUPPLICANT_WIFI_RTT_CONTROLLER_H

#include <aidl/android/hardware/wifi/supplicant/BnSupplicantWifiRttController.h>

extern "C" {
#include "utils/common.h"
#include "utils/includes.h"
#include "wpa_supplicant_i.h"
}

namespace aidl {
namespace android {
namespace hardware {
namespace wifi {
namespace supplicant {

class SupplicantWifiRttController : public BnSupplicantWifiRttController {
public:
	static std::shared_ptr<SupplicantWifiRttController> create(
		struct wpa_global* wpa_global, const char* ifname);
	static int getCurrentCmdId();
	static void setCurrentCmdId(int cmdId);
	~SupplicantWifiRttController() override = default;
	SupplicantWifiRttController(
		struct wpa_global* wpa_global, const char* ifname);

	void invalidate();
	bool isValid();
	void setWeakPtr(std::weak_ptr<SupplicantWifiRttController> ptr);

	::ndk::ScopedAStatus getName(std::string* _aidl_return) override;
	::ndk::ScopedAStatus getCapabilities(RttCapabilities* _aidl_return) override;
	::ndk::ScopedAStatus setProximityRangingDeviceName(
		const std::string& in_name) override;
	::ndk::ScopedAStatus setProximityRangingMacAddress(
		const std::array<uint8_t, 6>& in_macAddress) override;
	::ndk::ScopedAStatus getProximityRangingMacAddress(
		std::array<uint8_t, 6>* _aidl_return) override;
	::ndk::ScopedAStatus rangeRequest(
		int32_t in_cmdId,
		const std::vector<RttConfig>& in_rttConfigs) override;
	::ndk::ScopedAStatus rangeCancel(
		int32_t in_cmdId, const std::vector<MacAddress>& in_addrs) override;
	::ndk::ScopedAStatus registerEventCallback(
		const std::shared_ptr<ISupplicantWifiRttControllerEventCallback>&
			in_callback) override;

private:
	// Corresponding worker functions for the AIDL methods.
	std::pair<RttCapabilities, ndk::ScopedAStatus> getCapabilitiesInternal();
	ndk::ScopedAStatus setProximityRangingDeviceNameInternal(
		const std::string& name);
	ndk::ScopedAStatus setProximityRangingMacAddressInternal(
		const std::array<uint8_t, 6>& macAddress);
	std::pair<std::array<uint8_t, 6>, ndk::ScopedAStatus>
	getProximityRangingMacAddressInternal();
	ndk::ScopedAStatus rangeRequestInternal(
		int32_t cmdId, const std::vector<RttConfig>& rttConfigs);
	ndk::ScopedAStatus rangeCancelInternal(
		int32_t cmdId, const std::vector<MacAddress>& addrs);
	ndk::ScopedAStatus registerEventCallbackInternal(
		const std::shared_ptr<ISupplicantWifiRttControllerEventCallback>&
			callback);

	struct wpa_supplicant* retrieveIfacePtr();

	struct wpa_global* wpa_global_;
	const std::string ifname_;
	bool is_valid_;
	std::weak_ptr<SupplicantWifiRttController> weak_ptr_this_;
	std::string proximity_ranging_name_;
	std::array<uint8_t, 6> proximity_ranging_mac_address_{};
	static int current_cmd_id_;
};

}  // namespace supplicant
}  // namespace wifi
}  // namespace hardware
}  // namespace android
}  // namespace aidl

#endif  // WPA_SUPPLICANT_AIDL_VENDOR_SUPPLICANT_WIFI_RTT_CONTROLLER_H
