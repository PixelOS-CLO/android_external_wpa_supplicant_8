/*
 * WPA Supplicant - Aidl entry point to wpa_supplicant core
 * Copyright (c) 2021, Google Inc. All rights reserved.
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef WPA_SUPPLICANT_AIDL_AIDL_H
#define WPA_SUPPLICANT_AIDL_AIDL_H

#ifdef __cplusplus
extern "C"
{
#endif  // __cplusplus

enum wpas_continuous_ranging_status_code {
	WPAS_CONTINUOUS_RANGING_STATUS_UNKNOWN = 0,
	WPAS_CONTINUOUS_RANGING_STATUS_PR_RANGE_NEGOTIATION_STARTED = 1,
	WPAS_CONTINUOUS_RANGING_STATUS_PR_RANGE_NEGOTIATION_SUCCEEDED = 2,
	WPAS_CONTINUOUS_RANGING_STATUS_PR_STARTED_RANGE_REQUESTS_ISTA_ROLE = 3,
	WPAS_CONTINUOUS_RANGING_STATUS_PR_STARTED_RANGE_REQUESTS_RSTA_ROLE = 4,
};

	/**
	 * This is the aidl RPC interface entry point to the wpa_supplicant
	 * core. This initializes the aidl driver & AidlManager instance and
	 * then forwards all the notifcations from the supplicant core to the
	 * AidlManager.
	 */
	struct wpas_aidl_priv;
	struct wpa_global;

	struct wpas_aidl_priv *wpas_aidl_init(struct wpa_global *global);
	void wpas_aidl_deinit(struct wpas_aidl_priv *priv);

#ifdef CONFIG_CTRL_IFACE_AIDL
	int wpas_aidl_register_interface(struct wpa_supplicant *wpa_s);
	int wpas_aidl_unregister_interface(struct wpa_supplicant *wpa_s);
	int wpas_aidl_register_network(
		struct wpa_supplicant *wpa_s, struct wpa_ssid *ssid);
	int wpas_aidl_unregister_network(
		struct wpa_supplicant *wpa_s, struct wpa_ssid *ssid);
	int wpas_aidl_notify_state_changed(struct wpa_supplicant *wpa_s);
	int wpas_aidl_notify_network_request(
		struct wpa_supplicant *wpa_s, struct wpa_ssid *ssid,
		enum wpa_ctrl_req_type rtype, const char *default_txt);
	void wpas_aidl_notify_permanent_id_req_denied(struct wpa_supplicant *wpa_s);
	void wpas_aidl_notify_anqp_query_done(
		struct wpa_supplicant *wpa_s, const u8 *bssid, const char *result,
		const struct wpa_bss_anqp *anqp);
	void wpas_aidl_notify_hs20_icon_query_done(
		struct wpa_supplicant *wpa_s, const u8 *bssid,
		const char *file_name, const u8 *image, u32 image_length);
	void wpas_aidl_notify_hs20_rx_subscription_remediation(
		struct wpa_supplicant *wpa_s, const char *url, u8 osu_method);
	void wpas_aidl_notify_hs20_rx_deauth_imminent_notice(
		struct wpa_supplicant *wpa_s, u8 code, u16 reauth_delay,
		const char *url);
	void wpas_aidl_notify_hs20_rx_terms_and_conditions_acceptance(
			struct wpa_supplicant *wpa_s, const char *url);
	void wpas_aidl_notify_disconnect_reason(struct wpa_supplicant *wpa_s);
	void wpas_aidl_notify_mlo_info_change_reason(
		struct wpa_supplicant *wpa_s,
		enum mlo_info_change_reason reason);

	void wpas_aidl_notify_assoc_reject(struct wpa_supplicant *wpa_s, const u8 *bssid,
		u8 timed_out, const u8 *assoc_resp_ie, size_t assoc_resp_ie_len);
	void wpas_aidl_notify_auth_timeout(struct wpa_supplicant *wpa_s);
	void wpas_aidl_notify_bssid_changed(struct wpa_supplicant *wpa_s);
	void wpas_aidl_notify_wps_event_fail(
		struct wpa_supplicant *wpa_s, uint8_t *peer_macaddr,
		uint16_t config_error, uint16_t error_indication);
	void wpas_aidl_notify_wps_event_success(struct wpa_supplicant *wpa_s);
	void wpas_aidl_notify_wps_event_pbc_overlap(
		struct wpa_supplicant *wpa_s);
	void wpas_aidl_notify_p2p_device_found(
		struct wpa_supplicant *wpa_s, const u8 *addr,
		const struct p2p_peer_info *info, const u8 *peer_wfd_device_info,
		u8 peer_wfd_device_info_len, const u8 *peer_wfd_r2_device_info,
		u8 peer_wfd_r2_device_info_len);
	void wpas_aidl_notify_p2p_device_lost(
		struct wpa_supplicant *wpa_s, const u8 *p2p_device_addr);
	void wpas_aidl_notify_p2p_find_stopped(struct wpa_supplicant *wpa_s);
	void wpas_aidl_notify_p2p_go_neg_req(
		struct wpa_supplicant *wpa_s, const u8 *src_addr, u16 dev_passwd_id,
		u8 go_intent);
	void wpas_aidl_notify_p2p_go_neg_completed(
		struct wpa_supplicant *wpa_s, const struct p2p_go_neg_results *res);
	void wpas_aidl_notify_p2p_group_formation_failure(
		struct wpa_supplicant *wpa_s, const char *reason);
	void wpas_aidl_notify_p2p_group_started(
		struct wpa_supplicant *wpa_s, const struct wpa_ssid *ssid,
		int persistent, int client, const u8 *ip);
	void wpas_aidl_notify_p2p_group_removed(
		struct wpa_supplicant *wpa_s, const struct wpa_ssid *ssid,
		const char *role);
	void wpas_aidl_notify_p2p_invitation_received(
		struct wpa_supplicant *wpa_s, const u8 *sa, const u8 *go_dev_addr,
		const u8 *bssid, int id, int op_freq);
	void wpas_aidl_notify_p2p_invitation_result(
		struct wpa_supplicant *wpa_s, int status, const u8 *bssid);
	void wpas_aidl_notify_p2p_provision_discovery(
		struct wpa_supplicant *wpa_s, const u8 *dev_addr, int request,
		enum p2p_prov_disc_status status, u16 config_methods,
		unsigned int generated_pin, const char *group_ifname);
	void wpas_aidl_notify_p2p_bootstrap_request(
		struct wpa_supplicant *wpa_s, const u8 *dev_addr,
		int status, u16 bootstrap_method, const char *group_ifname);
	void wpas_aidl_notify_p2p_bootstrap_response(
		struct wpa_supplicant *wpa_s, const u8 *dev_addr,
		int status, u16 bootstrap_method, const char *group_ifname);
	void wpas_aidl_notify_p2p_sd_response(
		struct wpa_supplicant *wpa_s, const u8 *sa, u16 update_indic,
		const u8 *tlvs, size_t tlvs_len);
	void wpas_aidl_notify_ap_sta_authorized(
		struct wpa_supplicant *wpa_s, const u8 *sta,
		const u8 *p2p_dev_addr, const u8 *ip);
	void wpas_aidl_notify_ap_sta_deauthorized(
		struct wpa_supplicant *wpa_s, const u8 *sta,
		const u8 *p2p_dev_addr);
	void wpas_aidl_notify_eap_error(
		struct wpa_supplicant *wpa_s, int error_code);
	void wpas_aidl_notify_dpp_config_received(struct wpa_supplicant *wpa_s,
		struct wpa_ssid *ssid, bool conn_status_requested);
	void wpas_aidl_notify_dpp_config_sent(struct wpa_supplicant *wpa_s);
#ifdef CONFIG_DPP
	void wpas_aidl_notify_dpp_connection_status_sent(struct wpa_supplicant *wpa_s,
		enum dpp_status_error result);
#endif /* CONFIG_DPP */
	void wpas_aidl_notify_dpp_auth_success(struct wpa_supplicant *wpa_s);
	void wpas_aidl_notify_dpp_resp_pending(struct wpa_supplicant *wpa_s);
	void wpas_aidl_notify_dpp_not_compatible(struct wpa_supplicant *wpa_s);
	void wpas_aidl_notify_dpp_missing_auth(struct wpa_supplicant *wpa_s);
	void wpas_aidl_notify_dpp_configuration_failure(struct wpa_supplicant *wpa_s);
	void wpas_aidl_notify_dpp_invalid_uri(struct wpa_supplicant *wpa_s);
	void wpas_aidl_notify_dpp_timeout(struct wpa_supplicant *wpa_s);
	void wpas_aidl_notify_dpp_auth_failure(struct wpa_supplicant *wpa_s);
	void wpas_aidl_notify_dpp_fail(struct wpa_supplicant *wpa_s);
	void wpas_aidl_notify_dpp_config_sent_wait_response(struct wpa_supplicant *wpa_s);
	void wpas_aidl_notify_dpp_config_accepted(struct wpa_supplicant *wpa_s);
	void wpas_aidl_notify_dpp_config_rejected(struct wpa_supplicant *wpa_s);
#ifdef CONFIG_DPP
	void wpas_aidl_notify_dpp_conn_status(struct wpa_supplicant *wpa_s,
		enum dpp_status_error status, const char *ssid,
		const char *channel_list, unsigned short band_list[], int size);
#endif /* CONFIG_DPP */
	void wpas_aidl_notify_pmk_cache_added(
		struct wpa_supplicant *wpas, struct rsn_pmksa_cache_entry *pmksa_entry);
	void wpas_aidl_notify_bss_tm_status(struct wpa_supplicant *wpa_s);
	void wpas_aidl_notify_transition_disable(
		struct wpa_supplicant *wpa_s, struct wpa_ssid *ssid, u8 bitmap);
	void wpas_aidl_notify_network_not_found(struct wpa_supplicant *wpa_s);
	void wpas_aidl_notify_frequency_changed(struct wpa_supplicant *wpa_s, int frequency);
	void wpas_aidl_notify_ceritification(struct wpa_supplicant *wpa_s,
		int depth, const char *subject,
		const char *altsubject[],
		int num_altsubject,
		const char *cert_hash,
		const struct wpabuf *cert);
	void wpas_aidl_notify_eap_method_selected(struct wpa_supplicant *wpa_s,
		const char *reason_string);
	void wpas_aidl_notify_ssid_temp_disabled(struct wpa_supplicant *wpa_s,
		const char *reason_string);
	void wpas_aidl_notify_open_ssl_failure(struct wpa_supplicant *wpa_s,
		const char *reason_string);
	void wpas_aidl_notify_qos_policy_reset(struct wpa_supplicant *wpa_s);
	void wpas_aidl_notify_qos_policy_request(struct wpa_supplicant *wpa_s,
		struct dscp_policy_data *policies, int num_policies);
	ssize_t wpas_aidl_get_certificate(const char* alias, uint8_t** value);
	ssize_t wpas_aidl_list_aliases(const char *prefix, char ***aliases);
	void wpas_aidl_notify_qos_policy_scs_response(struct wpa_supplicant *wpa_s,
		unsigned int count, int **scs_resp);
	void wpas_aidl_notify_auth_status_code(struct wpa_supplicant *wpa_s,
		u16 auth_type, u16 auth_transaction, u16 status_code);
	void wpas_aidl_notify_nan_service_discovered(
		struct wpa_supplicant* wpa_s,
		enum nan_service_protocol_type srv_proto_type, int subscribe_id,
		int peer_publish_id, const u8* peer_addr, bool fsd, const u8* ssi,
		size_t ssi_len, const u8* match_filter,
		size_t match_filter_len, bool pairing_setup,
		bool pairing_cache, bool pairing_verification,
		u16 pbm, const u8* nonce, const u8* tag);
	void wpas_aidl_notify_nan_publish_replied(
		struct wpa_supplicant* wpa_s,
		enum nan_service_protocol_type srv_proto_type, int publish_id,
		int peer_subscribe_id, const u8* peer_addr, const u8* ssi,
		size_t ssi_len);
	void wpas_aidl_notify_nan_message_received(
		struct wpa_supplicant* wpa_s, int id, int peer_instance_id,
		const u8* peer_addr, const u8* message, size_t message_len);
	void wpas_aidl_notify_nan_publish_terminated(
		struct wpa_supplicant* wpa_s, int publish_id,
		enum nan_de_reason reason);
	void wpas_aidl_notify_nan_subscribe_terminated(
		struct wpa_supplicant* wpa_s, int subscribe_id,
		enum nan_de_reason reason);
	void wpas_aidl_notify_nan_transmit_status(struct wpa_supplicant *wpa_s,
		u32 cookie, bool ack);
	void wpas_aidl_notify_nan_cluster_event(
		struct wpa_supplicant* wpa_s, u8 event_type, const u8* peer_addr);
	void wpas_aidl_notify_nan_match_expired(
		struct wpa_supplicant* wpa_s, int subscribe_id,
		int peer_publish_id);
	void wpas_aidl_notify_nan_bootstrap_request(
		struct wpa_supplicant* wpa_s, const u8* peer_nmi_addr,
		u8 bootstrap_method, u8 discovery_session_id, int peer_id, int bootstrapping_id);
	void wpas_aidl_notify_nan_bootstrap_confirmed(
		struct wpa_supplicant* wpa_s, u8 handle, u8 bootstrapping_instance_id,
		const u8* peer_nmi_addr, u8 bootstrap_method,
		bool is_success, u8 reason, const u8* cookie, size_t cookie_len);
	void wpas_aidl_notify_nan_pairing_request(
		struct wpa_supplicant* wpa_s, u8 discovery_session_id, int peer_id,
		const u8* peer_nmi_addr, int pairing_id, bool is_setup,
		const u8* nonce, const u8* tag);
	void wpas_aidl_notify_nan_pairing_confirmed(
		struct wpa_supplicant* wpa_s, int pairing_id, const u8 *peer_addr,
		bool is_success, u8 status_code, int request_type);
	void wpas_aidl_notify_nan_nik_received(
		struct wpa_supplicant* wpa_s, const u8 *nik, size_t nik_len,
		int cipher_ver, int akmp, const u8 *npk, size_t npk_len,
		int nik_lifetime, int identity_id, int cipher,
		int discovery_session_id, int pairing_id);
	void wpas_aidl_notify_nan_ndp_request(
		struct wpa_supplicant* wpa_s, u8 ndp_id, const u8* peer_nmi_addr,
		const u8* init_ndi_addr, u8 discovery_session_id, u8 csid, const u8* app_info,
		size_t app_info_len);
	void wpas_aidl_notify_nan_ndp_confirmed(
		struct wpa_supplicant* wpa_s, u8 ndp_id, const u8* peer_ndi_addr,
		bool is_success, u8 reason, const u8* app_info, size_t app_info_len);
	void wpas_aidl_notify_nan_ndp_terminated(struct wpa_supplicant* wpa_s, u8 ndp_id);
	void wpas_aidl_notify_rtt_continuous_ranging_result(
		struct wpa_supplicant* wpa_s, const void *data);
	void wpas_aidl_notify_rtt_continuous_ranging_status(
		struct wpa_supplicant* wpas_s, enum wpas_continuous_ranging_status_code status);
	void wpas_aidl_notify_rtt_continuous_ranging_terminated(
		struct wpa_supplicant* wpa_s, u32 reason);
	// TODO(b/460750167): Add NAN NDP Schedule Update Event notification
	// TODO(b/460750167): Add NAN Pairing Event notification
#else   // CONFIG_CTRL_IFACE_AIDL
static inline int wpas_aidl_register_interface(struct wpa_supplicant *wpa_s)
{
	return 0;
}
static inline int wpas_aidl_unregister_interface(struct wpa_supplicant *wpa_s)
{
	return 0;
}
static inline int wpas_aidl_register_network(
	struct wpa_supplicant *wpa_s, struct wpa_ssid *ssid)
{
	return 0;
}
static inline int wpas_aidl_unregister_network(
	struct wpa_supplicant *wpa_s, struct wpa_ssid *ssid)
{
	return 0;
}
static inline int wpas_aidl_notify_state_changed(struct wpa_supplicant *wpa_s)
{
	return 0;
}
static inline int wpas_aidl_notify_network_request(
	struct wpa_supplicant *wpa_s, struct wpa_ssid *ssid,
	enum wpa_ctrl_req_type rtype, const char *default_txt)
{
	return 0;
}
static void wpas_aidl_notify_permanent_id_req_denied(struct wpa_supplicant *wpa_s)
{}
static void wpas_aidl_notify_anqp_query_done(
	struct wpa_supplicant *wpa_s, const u8 *bssid, const char *result,
	const struct wpa_bss_anqp *anqp)
{}
static void wpas_aidl_notify_hs20_icon_query_done(
	struct wpa_supplicant *wpa_s, const u8 *bssid, const char *file_name,
	const u8 *image, u32 image_length)
{}
static void wpas_aidl_notify_hs20_rx_subscription_remediation(
	struct wpa_supplicant *wpa_s, const char *url, u8 osu_method)
{}
static void wpas_aidl_notify_hs20_rx_deauth_imminent_notice(
	struct wpa_supplicant *wpa_s, u8 code, u16 reauth_delay, const char *url)
{}
static void wpas_aidl_notify_hs20_rx_terms_and_conditions_acceptance(
		struct wpa_supplicant *wpa_s, const char *url)
{}
static void wpas_aidl_notify_disconnect_reason(struct wpa_supplicant *wpa_s) {}
static void wpas_aidl_notify_mlo_info_change_reason(
	struct wpa_supplicant *wpa_s, enum mlo_info_change_reason reason)
{}
static void wpas_aidl_notify_assoc_reject(struct wpa_supplicant *wpa_s, const u8 *bssid,
	u8 timed_out, const u8 *assoc_resp_ie, size_t assoc_resp_ie_len) {}
static void wpas_aidl_notify_auth_timeout(struct wpa_supplicant *wpa_s) {}
static void wpas_aidl_notify_wps_event_fail(
	struct wpa_supplicant *wpa_s, uint8_t *peer_macaddr, uint16_t config_error,
	uint16_t error_indication)
{}
static void wpas_aidl_notify_bssid_changed(struct wpa_supplicant *wpa_s) {}
static void wpas_aidl_notify_wps_event_success(struct wpa_supplicant *wpa_s) {}
static void wpas_aidl_notify_wps_event_pbc_overlap(struct wpa_supplicant *wpa_s)
{}
static void wpas_aidl_notify_p2p_device_found(
	struct wpa_supplicant *wpa_s, const u8 *addr,
	const struct p2p_peer_info *info, const u8 *peer_wfd_device_info,
	u8 peer_wfd_device_info_len, const u8 *peer_wfd_r2_device_info,
	u8 peer_wfd_r2_device_info_len)
{}
static void wpas_aidl_notify_p2p_device_lost(
	struct wpa_supplicant *wpa_s, const u8 *p2p_device_addr)
{}
static void wpas_aidl_notify_p2p_find_stopped(struct wpa_supplicant *wpa_s) {}
static void wpas_aidl_notify_p2p_go_neg_req(
	struct wpa_supplicant *wpa_s, const u8 *src_addr, u16 dev_passwd_id,
	u8 go_intent)
{}
static void wpas_aidl_notify_p2p_go_neg_completed(
	struct wpa_supplicant *wpa_s, const struct p2p_go_neg_results *res)
{}
static void wpas_aidl_notify_p2p_group_formation_failure(
	struct wpa_supplicant *wpa_s, const char *reason)
{}
static void wpas_aidl_notify_p2p_group_started(
	struct wpa_supplicant *wpa_s, const struct wpa_ssid *ssid, int persistent,
	int client, const u8 *ip)
{}
static void wpas_aidl_notify_p2p_group_removed(
	struct wpa_supplicant *wpa_s, const struct wpa_ssid *ssid, const char *role)
{}
static void wpas_aidl_notify_p2p_invitation_received(
	struct wpa_supplicant *wpa_s, const u8 *sa, const u8 *go_dev_addr,
	const u8 *bssid, int id, int op_freq)
{}
static void wpas_aidl_notify_p2p_invitation_result(
	struct wpa_supplicant *wpa_s, int status, const u8 *bssid)
{}
static void wpas_aidl_notify_p2p_provision_discovery(
	struct wpa_supplicant *wpa_s, const u8 *dev_addr, int request,
	enum p2p_prov_disc_status status, u16 config_methods,
	unsigned int generated_pin, const char *group_ifname)
{}
static void wpas_aidl_notify_p2p_bootstrap_request(
	struct wpa_supplicant *wpa_s, const u8 *dev_addr,
	int status, u16 bootstrap_method, const char *group_ifname)
{}
static void wpas_aidl_notify_p2p_bootstrap_response(
	struct wpa_supplicant *wpa_s, const u8 *dev_addr,
	int status, u16 bootstrap_method, const char *group_ifname)
{}
static void wpas_aidl_notify_p2p_sd_response(
	struct wpa_supplicant *wpa_s, const u8 *sa, u16 update_indic,
	const u8 *tlvs, size_t tlvs_len)
{}
static void wpas_aidl_notify_ap_sta_authorized(
	struct wpa_supplicant *wpa_s, const u8 *sta, const u8 *p2p_dev_addr,
	const u8 *ip)
{}
static void wpas_aidl_notify_ap_sta_deauthorized(
	struct wpa_supplicant *wpa_s, const u8 *sta, const u8 *p2p_dev_addr)
{}
static void wpas_aidl_notify_eap_error(
	struct wpa_supplicant *wpa_s, int error_code)
{}
static void wpas_aidl_notify_dpp_config_received(struct wpa_supplicant *wpa_s,
	struct wpa_ssid *ssid, bool conn_status_requested)
{}
static void wpas_aidl_notify_dpp_config_sent(struct wpa_supplicant *wpa_s)
{}
#ifdef CONFIG_DPP
static void wpas_aidl_notify_dpp_connection_status_sent(struct wpa_supplicant *wpa_s,
	enum dpp_status_error result)
{}
#endif /* CONFIG_DPP */
static void wpas_aidl_notify_dpp_auth_success(struct wpa_supplicant *wpa_s)
{}
static void wpas_aidl_notify_dpp_resp_pending(struct wpa_supplicant *wpa_s)
{}
static void wpas_aidl_notify_dpp_not_compatible(struct wpa_supplicant *wpa_s)
{}
static void wpas_aidl_notify_dpp_missing_auth(struct wpa_supplicant *wpa_s)
{}
static void wpas_aidl_notify_dpp_configuration_failure(struct wpa_supplicant *wpa_s)
{}
static void wpas_aidl_notify_dpp_invalid_uri(struct wpa_supplicant *wpa_s)
{}
static void wpas_aidl_notify_dpp_timeout(struct wpa_supplicant *wpa_s)
{}
static void wpas_aidl_notify_dpp_auth_failure(struct wpa_supplicant *wpa_s)
{}
static void wpas_aidl_notify_dpp_fail(struct wpa_supplicant *wpa_s)
{}
static void wpas_aidl_notify_dpp_config_sent_wait_response(struct wpa_supplicant *wpa_s)
{}
static void wpas_aidl_notify_dpp_config_accepted(struct wpa_supplicant *wpa_s)
{}
static void wpas_aidl_notify_dpp_config_rejected(struct wpa_supplicant *wpa_s)
{}
#ifdef CONFIG_DPP
static void wpas_aidl_notify_dpp_conn_status(struct wpa_supplicant *wpa_s,
			enum dpp_status_error status, const char *ssid,
			const char *channel_list, unsigned short band_list[], int size)
{}
#endif /* CONFIG_DPP */
static void wpas_aidl_notify_pmk_cache_added(struct wpa_supplicant *wpas,
						 struct rsn_pmksa_cache_entry *pmksa_entry)
{}
static void wpas_aidl_notify_bss_tm_status(struct wpa_supplicant *wpa_s)
{}
static void wpas_aidl_notify_transition_disable(struct wpa_supplicant *wpa_s,
						struct wpa_ssid *ssid,
						u8 bitmap)
{}
static void wpas_aidl_notify_network_not_found(struct wpa_supplicant *wpa_s)
{}
static void wpas_aidl_notify_frequency_changed(struct wpa_supplicant *wpa_s, int frequency)
{}
static void wpas_aidl_notify_ceritification(struct wpa_supplicant *wpa_s,
	int depth, const char *subject,
	const char *altsubject[],
	int num_altsubject,
	const char *cert_hash,
	const struct wpabuf *cert)
{}
static void wpas_aidl_notify_eap_method_selected(struct wpa_supplicant *wpa_s,
	const char *reason_string)
{}
static void wpas_aidl_notify_ssid_temp_disabled(struct wpa_supplicant *wpa_s,
	const char *reason_string)
{}
static void wpas_aidl_notify_open_ssl_failure(struct wpa_supplicant *wpa_s,
	const char *reason_string)
{}
static void wpas_aidl_notify_qos_policy_reset(struct wpa_supplicant *wpa_s) {}
static void wpas_aidl_notify_qos_policy_request(struct wpa_supplicant *wpa_s,
						struct dscp_policy_data *policies,
						int num_policies)
{}
static ssize_t wpas_aidl_get_certificate(const char* alias, uint8_t** value)
{
	return -1;
}
static ssize_t wpas_aidl_list_aliases(const char *prefix, char ***aliases)
{
	return -1;
}
static void wpas_aidl_notify_qos_policy_scs_response(struct wpa_supplicant *wpa_s,
	unsigned int count, int **scs_resp) {}
static void wpas_aidl_notify_auth_status_code(struct wpa_supplicant *wpa_s,
		u16 auth_type, u16 auth_transaction, u16 status_code) {}
static void wpas_aidl_notify_nan_service_discovered(
    struct wpa_supplicant* wpa_s, enum nan_service_protocol_type srv_proto_type,
    int subscribe_id, int peer_publish_id, const u8* peer_addr, bool fsd,
    const u8* ssi, size_t ssi_len, const u8* match_filter,
    size_t match_filter_len, bool pairing_setup, bool pairing_cache,
    bool pairing_verification, u16 pbm, const u8* nonce, const u8* tag) {}
static void wpas_aidl_notify_nan_publish_replied(
    struct wpa_supplicant* wpa_s, enum nan_service_protocol_type srv_proto_type,
    int publish_id, int peer_subscribe_id, const u8* peer_addr, const u8* ssi,
    size_t ssi_len) {}
static void wpas_aidl_notify_nan_message_received(
    struct wpa_supplicant* wpa_s, int id, int peer_instance_id,
    const u8* peer_addr, const u8* message, size_t message_len) {}
static void wpas_aidl_notify_nan_publish_terminated(
    struct wpa_supplicant* wpa_s, int subscribe_id, enum nan_de_reason reason) {}
static void wpas_aidl_notify_nan_subscribe_terminated(
    struct wpa_supplicant* wpa_s, int subscribe_id, enum nan_de_reason reason) {}
static void wpas_aidl_notify_nan_transmit_status(struct wpa_supplicant *wpa_s,
		u32 cookie, bool ack) {}
static void wpas_aidl_notify_nan_cluster_event(
    struct wpa_supplicant* wpa_s, u8 event_type, const u8* peer_addr) {}
static void wpas_aidl_notify_nan_match_expired(
    struct wpa_supplicant* wpa_s, int subscribe_id, int peer_publish_id) {}
static void wpas_aidl_notify_nan_bootstrap_request(
		struct wpa_supplicant* wpa_s, const u8* peer_nmi_addr,
		u8 bootstrap_method, u8 discovery_session_id, int peer_id, int bootstrapping_id) {}
static void wpas_aidl_notify_nan_bootstrap_confirmed(
		struct wpa_supplicant* wpa_s, u8 handle, u8 bootstrapping_instance_id,
		const u8* peer_nmi_addr, u8 bootstrap_method,
		bool is_success, u8 reason, const u8* cookie, size_t cookie_len) {}
static void wpas_aidl_notify_nan_pairing_request(
		struct wpa_supplicant* wpa_s, u8 discovery_session_id, int peer_id,
		const u8* peer_nmi_addr, int pairing_id, bool is_setup,
		const u8* nonce, const u8* tag) {}
static void wpas_aidl_notify_nan_pairing_confirmed(
		struct wpa_supplicant* wpa_s, int pairing_id, const u8 *peer_addr,
		bool is_success, u8 status_code, int request_type) {}
static void wpas_aidl_notify_nan_nik_received(
		struct wpa_supplicant* wpa_s, const u8 *nik, size_t nik_len,
		int cipher_ver, int akmp, const u8 *npk, size_t npk_len,
		int nik_lifetime, int identity_id, int cipher,
		int discovery_session_id, int pairing_id) {}
static void wpas_aidl_notify_nan_ndp_request(
		struct wpa_supplicant* wpa_s, u8 ndp_id, const u8* peer_nmi_addr,
		const u8* init_ndi_addr, u8 discovery_session_id, u8 csid, const u8* app_info,
		size_t app_info_len) {}
static void wpas_aidl_notify_nan_ndp_confirmed(
		struct wpa_supplicant* wpa_s, u8 ndp_id, const u8* peer_ndi_addr,
		bool is_success, u8 reason, const u8* app_info, size_t app_info_len) {}
static void wpas_aidl_notify_nan_ndp_terminated(struct wpa_supplicant* wpa_s, u8 ndp_id) {}
static void wpas_aidl_notify_rtt_continuous_ranging_result(
	struct wpa_supplicant* wpa_s, const void *data) {}
static void wpas_aidl_notify_rtt_continuous_ranging_status(
	struct wpa_supplicant* wpa_s, enum wpas_continuous_ranging_status_code status) {}
static void wpas_aidl_notify_rtt_continuous_ranging_terminated(
		struct wpa_supplicant* wpa_s, u32 reason) {}
#endif  // CONFIG_CTRL_IFACE_AIDL

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // WPA_SUPPLICANT_AIDL_AIDL_H
