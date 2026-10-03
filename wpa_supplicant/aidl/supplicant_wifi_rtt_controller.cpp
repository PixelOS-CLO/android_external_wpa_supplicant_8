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

#include "supplicant_wifi_rtt_controller.h"
#include "aidl_manager.h"
#include "aidl_return_util.h"
#include "misc_utils.h"
#include "sta_iface.h"
#include "src/common/proximity_ranging.h"
#include "src/drivers/driver.h"

extern "C" {
#include "pr_supplicant.h"
}

namespace aidl {
namespace android {
namespace hardware {
namespace wifi {
namespace supplicant {
using aidl_return_util::validateAndCall;
using misc_utils::createStatus;
using PasnConfig::PASN_SAE_PMK_LEN_IN_BYTES;

constexpr int32_t kMaxNumContinuousRangingSeekerSessions = 1;
constexpr int32_t kMaxNumContinuousRangingAdvertiserSessions = 1;
constexpr int32_t kMaxNumPeersPerRangingSession = 1;
constexpr u8 kDefaultAvailabilityWindowMs = 160;

int SupplicantWifiRttController::current_cmd_id_ = -1;

std::shared_ptr<SupplicantWifiRttController> SupplicantWifiRttController::create(
	struct wpa_global* wpa_global, const char* ifname)
{
	std::shared_ptr<SupplicantWifiRttController> ptr =
		ndk::SharedRefBase::make<SupplicantWifiRttController>(
			wpa_global, ifname);
	std::weak_ptr<SupplicantWifiRttController> weak_ptr_this(ptr);
	ptr->setWeakPtr(weak_ptr_this);
	return ptr;
}

SupplicantWifiRttController::SupplicantWifiRttController(
	struct wpa_global* wpa_global, const char* ifname)
	: wpa_global_(wpa_global), ifname_(ifname), is_valid_(true)
{
}

void SupplicantWifiRttController::invalidate()
{
	wpa_global_ = nullptr;
	is_valid_ = false;
};

bool SupplicantWifiRttController::isValid() { return is_valid_; }

void SupplicantWifiRttController::setWeakPtr(
	std::weak_ptr<SupplicantWifiRttController> ptr)
{
	weak_ptr_this_ = ptr;
}

::ndk::ScopedAStatus SupplicantWifiRttController::getName(
	std::string* _aidl_return)
{
	*_aidl_return = ifname_;
	return ndk::ScopedAStatus::ok();
}

struct wpa_supplicant* SupplicantWifiRttController::retrieveIfacePtr()
{
	return wpa_supplicant_get_iface(wpa_global_, ifname_.c_str());
}

::ndk::ScopedAStatus SupplicantWifiRttController::getCapabilities(
	RttCapabilities* _aidl_return)
{
	return validateAndCall(
		this, SupplicantStatusCode::FAILURE_UNKNOWN,
		&SupplicantWifiRttController::getCapabilitiesInternal, _aidl_return);
}

::ndk::ScopedAStatus SupplicantWifiRttController::setProximityRangingDeviceName(
	const std::string& in_name)
{
	return validateAndCall(
		this, SupplicantStatusCode::FAILURE_UNKNOWN,
		&SupplicantWifiRttController::setProximityRangingDeviceNameInternal,
		in_name);
}

::ndk::ScopedAStatus SupplicantWifiRttController::setProximityRangingMacAddress(
	const std::array<uint8_t, 6>& in_macAddress)
{
	return validateAndCall(
		this, SupplicantStatusCode::FAILURE_UNKNOWN,
		&SupplicantWifiRttController::setProximityRangingMacAddressInternal,
		in_macAddress);
}

::ndk::ScopedAStatus SupplicantWifiRttController::getProximityRangingMacAddress(
	std::array<uint8_t, 6>* _aidl_return)
{
	return validateAndCall(
		this, SupplicantStatusCode::FAILURE_UNKNOWN,
		&SupplicantWifiRttController::getProximityRangingMacAddressInternal,
		_aidl_return);
}

::ndk::ScopedAStatus SupplicantWifiRttController::rangeRequest(
	int32_t in_cmdId, const std::vector<RttConfig>& in_rttConfigs)
{
	return validateAndCall(
		this, SupplicantStatusCode::FAILURE_UNKNOWN,
		&SupplicantWifiRttController::rangeRequestInternal, in_cmdId,
		in_rttConfigs);
}

::ndk::ScopedAStatus SupplicantWifiRttController::rangeCancel(
	int32_t in_cmdId, const std::vector<MacAddress>& in_addrs)
{
	return validateAndCall(
		this, SupplicantStatusCode::FAILURE_UNKNOWN,
		&SupplicantWifiRttController::rangeCancelInternal, in_cmdId,
		in_addrs);
}

::ndk::ScopedAStatus SupplicantWifiRttController::registerEventCallback(
	const std::shared_ptr<ISupplicantWifiRttControllerEventCallback>&
		in_callback)
{
	return validateAndCall(
		this, SupplicantStatusCode::FAILURE_UNKNOWN,
		&SupplicantWifiRttController::registerEventCallbackInternal,
		in_callback);
}

static RttBw convertBwToRttBw(u32 bw_bitmap)
{
	if (bw_bitmap & (BIT(WPA_PR_CHAN_WIDTH_80P80) | BIT(WPA_PR_CHAN_WIDTH_160)))
		return RttBw::BW_160MHZ;
	if (bw_bitmap & BIT(WPA_PR_CHAN_WIDTH_80))
		return RttBw::BW_80MHZ;
	if (bw_bitmap & BIT(WPA_PR_CHAN_WIDTH_40))
		return RttBw::BW_40MHZ;
	if (bw_bitmap & BIT(WPA_PR_CHAN_WIDTH_20))
		return RttBw::BW_20MHZ;
	return RttBw::INVALID;
}

static RttPreamble convertPreambleToRttPreamble(u32 preamble_bitmap)
{
	if (preamble_bitmap & BIT(WPA_PR_PREAMBLE_HE))
		return RttPreamble::HE;
	if (preamble_bitmap & BIT(WPA_PR_PREAMBLE_VHT))
		return RttPreamble::VHT;
	if (preamble_bitmap & BIT(WPA_PR_PREAMBLE_HT))
		return RttPreamble::HT;
	return RttPreamble::INVALID;
}

int SupplicantWifiRttController::getCurrentCmdId()
{
	return current_cmd_id_;
}

void SupplicantWifiRttController::setCurrentCmdId(int cmdId)
{
	current_cmd_id_ = cmdId;
}

// Internal implementations
std::pair<RttCapabilities, ndk::ScopedAStatus>
SupplicantWifiRttController::getCapabilitiesInternal()
{
	if (wpa_global_->pr == nullptr || wpa_global_->pr->cfg == nullptr) {
		return {RttCapabilities(), ndk::ScopedAStatus::ok()};
	}

	struct wpa_supplicant* wpa_s = retrieveIfacePtr();
	if (wpa_s == nullptr) {
		wpa_printf(MSG_ERROR, "Failed to retrieve wpa_supplicant interface for %s",
			   ifname_.c_str());
		return {RttCapabilities(), createStatus(SupplicantStatusCode::FAILURE_UNKNOWN)};
	}

	struct wpa_driver_capa capa;
	if (wpa_drv_get_capa(wpa_s, &capa) < 0) {
		return {RttCapabilities(),
			createStatus(SupplicantStatusCode::FAILURE_UNKNOWN)};
	}

	RttCapabilities capabilities;
	capabilities.prDeviceInfo.emplace();
	capabilities.prDeviceInfo->protocolInfo.emplace();
	capabilities.prDeviceInfo->maxNumContinuousRangingSeekerSessions =
		kMaxNumContinuousRangingSeekerSessions;
	capabilities.prDeviceInfo->maxNumContinuousRangingAdvertiserSessions =
		kMaxNumContinuousRangingAdvertiserSessions;
	capabilities.prDeviceInfo->minAllowedRangingIntervalEdcaMs =
		wpa_global_->pr->cfg->edca_min_ranging_interval;
	capabilities.prDeviceInfo->minAllowedRangingIntervalNtbMs =
		wpa_global_->pr->cfg->ntb_min_ranging_interval;
	// MAC randomization is set to true by default, since mac_randomization flag is not advertised
	// to shim layer. If mac randomizationn on connected STA is not supported, the raning request
	// will fail at wdev creation.
	capabilities.prDeviceInfo->isConnectedMacRandomizationSupported = true;
	capabilities.prDeviceInfo->isConcurrentIStaRStaOperationSupported =
		wpa_global_->pr->cfg->concurrent_ista_rsta;
	capabilities.prDeviceInfo->protocolInfo->deviceName =
		wpa_global_->pr->cfg->dev_name ? wpa_global_->pr->cfg->dev_name : "";
	capabilities.prDeviceInfo->protocolInfo->isEdcaBasedRangingSupported =
		wpa_global_->pr->cfg->edca_ista_support || wpa_global_->pr->cfg->edca_rsta_support;
	capabilities.prDeviceInfo->protocolInfo->isNtbNonSecureLtfRangingSupported =
		wpa_global_->pr->cfg->ntb_ista_support || wpa_global_->pr->cfg->ntb_rsta_support;
	capabilities.prDeviceInfo->protocolInfo->isNtbSecureLtfRangingSupported =
		(wpa_global_->pr->cfg->ntb_ista_support ||
		wpa_global_->pr->cfg->ntb_rsta_support) && wpa_global_->pr->cfg->secure_he_ltf;
	capabilities.prDeviceInfo->protocolInfo->isUnauthenticatedPasnModeSupported =
		wpa_global_->pr->cfg->pasn_type & (PR_PASN_DH19_UNAUTH | PR_PASN_DH20_UNAUTH);
	capabilities.prDeviceInfo->protocolInfo->isAuthenticatedPasnModeSupported =
		wpa_global_->pr->cfg->pasn_type & (PR_PASN_DH19_AUTH | PR_PASN_DH20_AUTH);
	capabilities.prDeviceInfo->protocolInfo->isEdcaBasedIstaRoleSupported =
		wpa_global_->pr->cfg->edca_ista_support;
	capabilities.prDeviceInfo->protocolInfo->isEdcaBasedRstaRoleSupported =
		wpa_global_->pr->cfg->edca_rsta_support;
	capabilities.prDeviceInfo->protocolInfo->isNtbIstaRoleSupported =
		wpa_global_->pr->cfg->ntb_ista_support;
	capabilities.prDeviceInfo->protocolInfo->isNtbRstaRoleSupported =
		wpa_global_->pr->cfg->ntb_rsta_support;
	capabilities.prDeviceInfo->protocolInfo->maxSupportedPacketBandwidthEdcaBased =
		convertBwToRttBw(capa.pd_bandwidths);
	capabilities.prDeviceInfo->protocolInfo->maxSupportedPacketBandwidthNtb =
		convertBwToRttBw(capa.pd_bandwidths);
	capabilities.prDeviceInfo->protocolInfo->maxSupportedPreambleEdcaBased =
		convertPreambleToRttPreamble(capa.pd_preambles);
	capabilities.prDeviceInfo->protocolInfo->maxSupportedPreambleNtb =
		convertPreambleToRttPreamble(capa.pd_preambles);
	capabilities.prDeviceInfo->protocolInfo->is6GHzSupported =
		wpa_global_->pr->cfg->support_6ghz;
	capabilities.prDeviceInfo->protocolInfo->maxNumTxAntennas =
		wpa_global_->pr->cfg->max_tx_antenna;
	capabilities.prDeviceInfo->protocolInfo->maxNumRxAntennas =
		wpa_global_->pr->cfg->max_rx_antenna;
	return {capabilities, ndk::ScopedAStatus::ok()};
}

ndk::ScopedAStatus
SupplicantWifiRttController::setProximityRangingDeviceNameInternal(
	const std::string& name)
{
	if (wpa_global_->pr == nullptr || wpa_global_->pr->cfg == nullptr) {
		return createStatus(SupplicantStatusCode::FAILURE_UNKNOWN);
	}
	char* new_name = os_strdup(name.c_str());
	if (new_name == nullptr) {
		return createStatus(SupplicantStatusCode::FAILURE_UNKNOWN);
	}
	os_free(wpa_global_->pr->cfg->dev_name);
	wpa_global_->pr->cfg->dev_name = new_name;
	proximity_ranging_name_ = name;
	return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus
SupplicantWifiRttController::setProximityRangingMacAddressInternal(
	const std::array<uint8_t, 6>& macAddress)
{
	struct wpa_supplicant* wpa_s = retrieveIfacePtr();
	if (wpa_s == nullptr) {
		wpa_printf(MSG_ERROR, "Failed to retrieve wpa_supplicant interface for %s",
			   ifname_.c_str());
		return createStatus(SupplicantStatusCode::FAILURE_UNKNOWN);
	}

	// Only updates the local copy of MAC address, as pr_pasn_ranging_params->src_addr will
	// set the new MAC address to driver when a new PASN ranging starts.
	proximity_ranging_mac_address_ = macAddress;

	AidlManager* aidl_manager = AidlManager::getInstance();
	if (aidl_manager) {
		std::shared_ptr<ISupplicantStaIface> sta_iface_interface;
		if (aidl_manager->getStaIfaceAidlObjectByIfname(ifname_, &sta_iface_interface) == 0
		    && sta_iface_interface != nullptr) {
			std::shared_ptr<StaIface> sta_iface =
				std::static_pointer_cast<StaIface>(sta_iface_interface);
			sta_iface->setProximityRangingMacAddress(macAddress);
		} else {
			wpa_printf(MSG_WARNING, "RTT: Failed to get StaIface for %s to update MAC",
				   ifname_.c_str());
		}
	}

	return ndk::ScopedAStatus::ok();
}

std::pair<std::array<uint8_t, 6>, ndk::ScopedAStatus>
SupplicantWifiRttController::getProximityRangingMacAddressInternal()
{
	return {proximity_ranging_mac_address_, ndk::ScopedAStatus::ok()};
}

static u8 convertRttTypeToRangingType(RttType type, bool isSecureLtf) {
	switch (type) {
	case RttType::TWO_SIDED_11MC:
		return PR_EDCA_BASED_RANGING;
	case RttType::TWO_SIDED_11AZ_NTB_SECURE:
		return isSecureLtf ? PR_NTB_SECURE_LTF_BASED_RANGING : PR_NTB_OPEN_BASED_RANGING;
	default:
		return 0;
	}
}

static int convertKeyMgmtMaskToAuthMode(int baseAkm, bool isPmkValid, bool isPassphraseValid)
{
	if (baseAkm & (int)KeyMgmtMask::SAE) {
		if (isPmkValid) {
			return PR_PASN_AUTH_MODE_PMK;
		} else if (isPassphraseValid) {
			return PR_PASN_AUTH_MODE_SAE;
		} else {
			return -1;
		}
	} else if (baseAkm & (int)KeyMgmtMask::PASN) {
		return PR_PASN_AUTH_MODE_PASN;
	}

	return -1;
}

static int validateRttConfig(const std::vector<RttConfig>& rttConfigs) {
	if (rttConfigs.size() != kMaxNumPeersPerRangingSession) {
		wpa_printf(MSG_ERROR, "Only %d ranging config is supported currently, received %zu",
			   kMaxNumPeersPerRangingSession, rttConfigs.size());
		return -1;
	}

	if (rttConfigs[0].peer != RttConfig::RttPeerType::STA) {
		wpa_printf(MSG_ERROR, "Only STA peer type is supported currently");
		return -1;
	}

	if (!rttConfigs[0].pdConfig.has_value()) {
		wpa_printf(MSG_ERROR, "ProximityRangingConfig is null");
		return -1;
	}

	if (!rttConfigs[0].secureConfig.has_value()) {
		wpa_printf(MSG_ERROR, "SecureConfig is null");
		return -1;
	}

	return 0;
}

static bool setPasnSecureConfig(const RttSecureConfig& secureConfig, struct pr_pasn_ranging_params* params) {
	const PasnConfig& pasnConfig = secureConfig.pasnConfig;
	// TODO(b/514151731): Handle USD mode DevIk/Password/PMK setup.
	if (pasnConfig.pmk.has_value()) {
		if (pasnConfig.pmk.value().size() != PASN_SAE_PMK_LEN_IN_BYTES) {
			wpa_printf(MSG_ERROR, "PASN: PMK vector length mismatch (%zu != %u)",
				   pasnConfig.pmk.value().size(), PASN_SAE_PMK_LEN_IN_BYTES);
			return false;
		}
		os_memcpy(params->pmk, pasnConfig.pmk.value().data(), PASN_SAE_PMK_LEN_IN_BYTES);
		params->pmk_len = PASN_SAE_PMK_LEN_IN_BYTES;
	} else {
		params->pmk_len = 0;
	}

	if (pasnConfig.passphrase.has_value()) {
		size_t pass_len = pasnConfig.passphrase.value().size();
		if (pass_len == 0) {
			wpa_printf(MSG_ERROR, "PASN: Passphrase vector is empty");
			return false;
		}
		size_t copy_len = pass_len;
		// Note: the maximum length of password is not the same as defined in the PasnConfig
		// AIDL. Here we truncate the password if it's too long.
		if (copy_len >= sizeof(params->password)) {
			copy_len = sizeof(params->password) - 1;
		}
		os_memcpy(params->password, pasnConfig.passphrase.value().data(), copy_len);
		params->password[copy_len] = '\0';
		params->password_valid = true;
	} else {
		params->password_valid = false;
	}
	return true;
}

ndk::ScopedAStatus SupplicantWifiRttController::rangeRequestInternal(
	int32_t cmdId, const std::vector<RttConfig>& rttConfigs)
{
	if (current_cmd_id_ != -1) {
		wpa_printf(MSG_ERROR, "Ranging session already active (cmdId=%d)", current_cmd_id_);
		return createStatus(SupplicantStatusCode::FAILURE_UNKNOWN);
	}
	current_cmd_id_ = cmdId;

	struct wpa_supplicant* wpa_s = retrieveIfacePtr();
	if (wpa_s == nullptr) {
		wpa_printf(MSG_ERROR, "Failed to retrieve wpa_supplicant interface for %s",
			   ifname_.c_str());
		return createStatus(SupplicantStatusCode::FAILURE_UNKNOWN);
	}

	if (validateRttConfig(rttConfigs) < 0) {
		wpa_printf(MSG_ERROR, "Validation of RttConfig faiiled");
		return createStatus(SupplicantStatusCode::FAILURE_UNKNOWN);
	}

	struct pr_pasn_ranging_params params;
	const RttConfig& rttConfig = rttConfigs[0];
	const ProximityRangingConfig& pdConfig = rttConfig.pdConfig.value();
	const RttSecureConfig& secureConfig = rttConfig.secureConfig.value();
	std::copy(rttConfig.addr.begin(), rttConfig.addr.end(),
		params.peer_addr);
	params.ranging_type = convertRttTypeToRangingType(rttConfig.type,
		secureConfig.enableSecureHeLtf);
	if (params.ranging_type == 0) {
		wpa_printf(MSG_ERROR, "Invalid RttType in RttConfig");
		return createStatus(SupplicantStatusCode::FAILURE_ARGS_INVALID);
	}
	if (pdConfig.rangingServiceRole == ProximityRangingConfig::RangingServiceRole::SEEKER) {
		if (pdConfig.preferredRangingMeasurementRole ==
				ProximityRangingConfig::RangingMeasurementRole::UNKNOWN) {
			params.ranging_role = PR_ISTA_SUPPORT;
			// Default set ISTA role for seeker if measurement role is not specified
		} else {
			params.ranging_role = pdConfig.preferredRangingMeasurementRole ==
				ProximityRangingConfig::RangingMeasurementRole::INITIATOR_STA
			 ? PR_ISTA_SUPPORT : PR_RSTA_SUPPORT;
		}
	} else if (pdConfig.rangingServiceRole ==
			ProximityRangingConfig::RangingServiceRole::ADVERTISER) {
		if (pdConfig.preferredRangingMeasurementRole ==
				ProximityRangingConfig::RangingMeasurementRole::UNKNOWN) {
			params.ranging_role = PR_RSTA_SUPPORT;
		} else {
			params.ranging_role = pdConfig.preferredRangingMeasurementRole ==
				ProximityRangingConfig::RangingMeasurementRole::INITIATOR_STA
			 ? PR_RSTA_SUPPORT : PR_ISTA_SUPPORT;
		}
	} else {
		wpa_printf(MSG_ERROR, "Invalid RangingServiceRole in RttConfig");
		return createStatus(SupplicantStatusCode::FAILURE_ARGS_INVALID);
	}
	params.auth_mode = convertKeyMgmtMaskToAuthMode(secureConfig.pasnConfig.baseAkm,
		secureConfig.pasnConfig.pmk.has_value(), secureConfig.pasnConfig.passphrase.has_value());
	if (params.auth_mode < 0) {
		wpa_printf(MSG_ERROR, "Invalid KeyMgmtMask in SecureConfig");
		return createStatus(SupplicantStatusCode::FAILURE_ARGS_INVALID);
	}
	params.freq = pdConfig.discoveryChannelFrequencyMhz;
	params.ranging_timeout = pdConfig.continuousRangingSessionTimeMillis;
	params.pr_pasn_status = PASN_STATUS_SUCCESS;
	std::copy(proximity_ranging_mac_address_.begin(), proximity_ranging_mac_address_.end(),
		params.src_addr);
	params.pasn_role = params.ranging_role == PR_ISTA_SUPPORT ? PR_ROLE_PASN_INITIATOR
		 : PR_ROLE_PASN_RESPONDER;

	/* EDCA parameters */
	params.burst_period = pdConfig.continuousRangingIntervalMillis;
	params.ftmr_retries = rttConfig.numRetriesPerFtmr;
	params.burst_duration = rttConfig.burstDuration;

	/* NTB parameters */
	params.min_time_between_measurements = rttConfig.ntbMinMeasurementTimeIn100Us;
	params.max_time_between_measurements = rttConfig.ntbMaxMeasurementTimeIn10Millis;
	// TODO(b/496389975): availability_window, nominal_time and measurements_per_aw
	// need further discussion on whether set in core supplicant or here.
	params.availability_window = kDefaultAvailabilityWindowMs;
	params.nominal_time = pdConfig.continuousRangingIntervalMillis;

	/* continuous ranging and location parameters */
	params.request_lci = rttConfig.mustRequestLci;
	params.request_civicloc = rttConfig.mustRequestLcr;
	params.continuous_ranging_session_time = pdConfig.continuousRangingSessionTimeMillis;
	params.forced_pr_freq = pdConfig.preferredRangingChannelFrequencyMhz;

	/* PD-specific parameters */
	if (pdConfig.configRangingIndications &
			(int)ProximityRangingConfig::ProximityRangingIndication::INGRESS_MET_MASK) {
		params.ingress_threshold = pdConfig.distanceIngressCm * 10;
	}
	if (pdConfig.configRangingIndications &
			(int)ProximityRangingConfig::ProximityRangingIndication::EGRESS_MET_MASK) {
		params.egress_threshold = pdConfig.distanceEgressCm * 10;
	}
	// The lmr_feedback is set true by default for RSTA role.
	if (params.ranging_type == PR_NTB_SECURE_LTF_BASED_RANGING ||
			params.ranging_type == PR_NTB_OPEN_BASED_RANGING) {
		if (params.ranging_role == PR_RSTA_SUPPORT) {
			params.lmr_feedback = true;
		} else if (params.ranging_role == PR_ISTA_SUPPORT &&
				pdConfig.advertiserRequiresRangeReport == true) {
			params.lmr_feedback = true;
		}
		params.ftms_per_burst = rttConfig.numNtbRepetitionsPerMeasurement;
	} else {
		params.ftms_per_burst = rttConfig.numFramesPerBurst;
	}

	if (!setPasnSecureConfig(secureConfig, &params)) {
		wpa_printf(MSG_ERROR, "Failed to set PASN secure config");
		return createStatus(SupplicantStatusCode::FAILURE_ARGS_INVALID);
	}
	wpas_pr_pasn_trigger(wpa_s, &params);

	return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus SupplicantWifiRttController::rangeCancelInternal(
	int32_t cmdId, const std::vector<MacAddress>& addrs)
{
	if (current_cmd_id_ != cmdId) {
		return createStatus(SupplicantStatusCode::FAILURE_UNKNOWN);
	}

	struct wpa_supplicant* wpa_s = retrieveIfacePtr();
	if (wpa_s == nullptr) {
		wpa_printf(MSG_ERROR, "Failed to retrieve wpa_supplicant interface for %s",
			   ifname_.c_str());
		return createStatus(SupplicantStatusCode::FAILURE_UNKNOWN);
	}

	wpas_pr_abort_ranging(wpa_s);

	AidlManager *aidl_manager = AidlManager::getInstance();
	if (aidl_manager) {
		aidl_manager->notifyRttContinuousRangingTerminatedEvent(wpa_s, 
			static_cast<u32>(ISupplicantWifiRttControllerEventCallback::
			ContinuousRangingTerminateReasonCode::USER_REQUEST));
	}
	current_cmd_id_ = -1;
	return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus SupplicantWifiRttController::registerEventCallbackInternal(
	const std::shared_ptr<ISupplicantWifiRttControllerEventCallback>&
		callback)
{
	AidlManager *aidl_manager = AidlManager::getInstance();
	if (!aidl_manager ||
		    aidl_manager->addWifiRttControllerEventCallbackAidlObject(ifname_, callback)) {
		return createStatus(SupplicantStatusCode::FAILURE_UNKNOWN);
	}
	return ndk::ScopedAStatus::ok();
}

}  // namespace supplicant
}  // namespace wifi
}  // namespace hardware
}  // namespace android
}  // namespace aidl
