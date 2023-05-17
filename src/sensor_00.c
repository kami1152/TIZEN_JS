#include "sensor_00.h"
#include <sensor/hrm_sen.h>
#include <app_common.h>
#include <device/power.h>
#include <privacy_privilege_manager.h>

typedef struct appdata {
	Evas_Object *win;
	Evas_Object *conform;
	Evas_Object *label;
} appdata_s;

sensor_type_e sensor_type = SENSOR_HRM;
sensor_h hrm_sensor_handle = 0;

bool check_hrm_sensor_is_supported();
bool initialize_hrm_sensor();

const char *sensor_privilege = "http://tizen.org/privilege/healthinfo";
const char *mediastorage_privilege = "http://tizen.org/privilege/mediastorage";

bool check_and_request_sensor_permission();
bool request_sensor_permission();
void request_health_sensor_permission_response_callback(ppm_call_cause_e cause,
		ppm_request_result_e result, const cahr *privilege, void *user_data);
void request_media_permission_response_callback(ppm_call_cause_e cause,
		ppm_request_result_e result, const char *privilege, void *user_data);

static void win_delete_request_cb(void *data, Evas_Object *obj,
		void *event_info) {
	ui_app_exit();
}

static void win_back_cb(void *data, Evas_Object *obj, void *event_info) {
	appdata_s *ad = data;
	/* Let window go to hide state. */
	elm_win_lower(ad->win);
}

static void create_base_gui(appdata_s *ad) {
	/* Window */
	/* Create and initialize elm_win.
	 elm_win is mandatory to manipulate window. */
	ad->win = elm_win_util_standard_add(PACKAGE, PACKAGE);
	elm_win_autodel_set(ad->win, EINA_TRUE);

	if (elm_win_wm_rotation_supported_get(ad->win)) {
		int rots[4] = { 0, 90, 180, 270 };
		elm_win_wm_rotation_available_rotations_set(ad->win,
				(const int *) (&rots), 4);
	}

	evas_object_smart_callback_add(ad->win, "delete,request",
			win_delete_request_cb, NULL);
	eext_object_event_callback_add(ad->win, EEXT_CALLBACK_BACK, win_back_cb,
			ad);

	/* Conformant */
	/* Create and initialize elm_conformant.
	 elm_conformant is mandatory for base gui to have proper size
	 when indicator or virtual keypad is visible. */
	ad->conform = elm_conformant_add(ad->win);
	elm_win_indicator_mode_set(ad->win, ELM_WIN_INDICATOR_SHOW);
	elm_win_indicator_opacity_set(ad->win, ELM_WIN_INDICATOR_OPAQUE);
	evas_object_size_hint_weight_set(ad->conform, EVAS_HINT_EXPAND,
	EVAS_HINT_EXPAND);
	elm_win_resize_object_add(ad->win, ad->conform);
	evas_object_show(ad->conform);

	/* Label */
	/* Create an actual view of the base gui.
	 Modify this part to change the view. */
	ad->label = elm_label_add(ad->conform);
	elm_object_text_set(ad->label, "<align=center>Hello Tizen</align>");
	evas_object_size_hint_weight_set(ad->label, EVAS_HINT_EXPAND,
	EVAS_HINT_EXPAND);
	elm_object_content_set(ad->conform, ad->label);

	/* Show window after base gui is set up */
	evas_object_show(ad->win);
}

static bool app_create(void *data) {
	/* Hook to take necessary actions before main event loop starts
	 Initialize UI resources and application's data
	 If this function returns true, the main loop of application starts
	 If this function returns false, the application is terminated */
	appdata_s *ad = data;

	create_base_gui(ad);
	if (!check_hrm_sensor_is_supported()) {
		dlog_print(DLOG_ERROR, HRM_SENSOR_LOG_TAG,
				"A HRM sensor is not supported.");
		return false;
	} else
		dlog_print(DLOG_INFO, HRM_SENSOR_LOG_TAG, "A HRM sensor is supported.");

	return true;
}

static void app_control(app_control_h app_control, void *data) {
	/* Handle the launch request. */
}

static void app_pause(void *data) {
	/* Take necessary actions when application becomes invisible. */
	appdata_s *ad = data;
}

static void app_resume(void *data) {
	/* Take necessary actions when application becomes visible. */
	device_power_request_lock(POWER_LOCK_DISPLAY, 0);
	device_power_request_lock(POWER_LOCK_CPU, 0);

	appdata_s *ad = data;

	if (!check_and_request_sensor_permission()) {

	}
}

static void app_terminate(void *data) {
	/* Release all resources. */
}

static void ui_app_lang_changed(app_event_info_h event_info, void *user_data) {
	/*APP_EVENT_LANGUAGE_CHANGED*/
	char *locale = NULL;
	system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE,
			&locale);
	elm_language_set(locale);
	free(locale);
	return;
}

static void ui_app_orient_changed(app_event_info_h event_info, void *user_data) {
	/*APP_EVENT_DEVICE_ORIENTATION_CHANGED*/
	return;
}

static void ui_app_region_changed(app_event_info_h event_info, void *user_data) {
	/*APP_EVENT_REGION_FORMAT_CHANGED*/
}

static void ui_app_low_battery(app_event_info_h event_info, void *user_data) {
	/*APP_EVENT_LOW_BATTERY*/
}

static void ui_app_low_memory(app_event_info_h event_info, void *user_data) {
	/*APP_EVENT_LOW_MEMORY*/
}

int main(int argc, char *argv[]) {
	appdata_s ad = { 0, };
	int ret = 0;

	ui_app_lifecycle_callback_s event_callback = { 0, };
	app_event_handler_h handlers[5] = { NULL, };

	event_callback.create = app_create;
	event_callback.terminate = app_terminate;
	event_callback.pause = app_pause;
	event_callback.resume = app_resume;
	event_callback.app_control = app_control;

	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_BATTERY],
			APP_EVENT_LOW_BATTERY, ui_app_low_battery, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_MEMORY],
			APP_EVENT_LOW_MEMORY, ui_app_low_memory, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_DEVICE_ORIENTATION_CHANGED],
			APP_EVENT_DEVICE_ORIENTATION_CHANGED, ui_app_orient_changed, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED],
			APP_EVENT_LANGUAGE_CHANGED, ui_app_lang_changed, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED],
			APP_EVENT_REGION_FORMAT_CHANGED, ui_app_region_changed, &ad);

	ret = ui_app_main(argc, argv, &event_callback, &ad);
	if (ret != APP_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "app_main() is failed. err = %d", ret);
	}

	return ret;
}

bool check_hrm_sensor_is_supported() {
	int hrm_retval;
	bool hrm_supported = false;
	hrm_retval = sensor_is_supported(SENSOR_HRM, &hrm_supported);
	if (hrm_retval != SENSOR_ERROR_NONE) {
		dlog_print(DLOG_ERROR, HRM_SENSOR_LOG_TAG,
				"Function sensor_is_supported() return value = %s",
				get_error_message(hrm_retval));
		dlog_print(DLOG_ERROR, HRM_SENSOR_LOG_TAG,
				"Failed to checks whether a HRM sensor is supported in the current device.");
		return false;
	} else
		dlog_print(DLOG_INFO, HRM_SENSOR_LOG_TAG,
				"Succeeded in checking whether a HRM sensor is supported in the current device.");
	if (!hrm_supported) {
		dlog_print(DLOG_ERROR, HRM_SENSOR_LOG_TAG,
				"Function sensor_is_supported() output supported = %d",
				hrm_supported);
		return false;
	} else
		return true;

}
bool initalize_hrm_sensor() {
	int retval;
	retval = sensor_get_default_sensor(SENSOR_HRM, &hrm_sensor_handle);
	if (retval != SENSOR_ERROR_NONE) {
		dlog_print(DLOG_INFO, HRM_SENSOR_LOG_TAG,
				"Function sensor_get_default_sensor() return value = %s",
				get_error_message(retval));
		return false;
	} else
		return true;
}

void request_health_sensor_permission_response_callback(ppm_call_cause_e,
		ppm_request_result_e result, const char *privilege, void *user_data) {
	if (cause == PRIVACY_PRIVILEGE_MANAGER_CALL_CAUSE_ERROR) {
		dlog_print(DLOG_INFO, HRM_SENSOR_LOG_TAG,
				"Function request_sensor_permission_response_callback() output cause = PRIVACY_PRIVILEGE_MANAGER_CALL_CAUSE_ERROR");
		dlog_print(DLOG_ERROR, HRM_SENSOR_LOG_TAG,
				"Function request_sensor_permission_response_callback() was called because of an error.");
	} else {
		dlog_print(DLOG_INFO, HRM_SENSOR_LOG_TAG,
				"Function request_sensor_permission_response_callback() was called with a valid answer.");

		switch (result) {
		case PRIVACY_PRIVILEGE_MANAGER_REQUEST_RESULT_ALLOW_FOREVER:
			dlog_print(DLOG_INFO, HRM_SENSOR_LOG_TAG,
					"The user granted permission to use a sensor privilege for an indefinite period of time.");
			if (!initalize_hrm_sensor()) {
				dlog_print(DLOG_ERROR, HRM_SENSOR_LOG_TAG,
						"Failed to get the handle for the default sensor of a HRM sensor.");
				ui_app_exit();
			} else {
				dlog_print(DLOG_INFO, HRM_SENSOR_LOG_TAG,
						"Succeeded in getting the handle for the default sensor of a HRM sensor.");

			}
			if (!create_hrm_sensor_listener(hrm_sensor_handle)) {
				dlog_print(DLOG_ERROR, HRM_SENSOR_LOG_TAG,
						"Failed to create a HRM sensor listener.");
				ui_app_exit();
			} else
				dlog_print(DLOG_INFO, HRM_SENSOR_LOG_TAG,
						"Succeeded in creating a HRM sensor listener.");

			if (!start_hrm_sensor_listener()) {
				dlog_print(DLOG_ERROR, HRM_SENSOR_LOG_TAG,
						"Failed to start observing the sensor events regarding a HRM sensor listener.");
				ui_app_exit();
			} else
				dlog_print(DLOG_INFO, HRM_SENSOR_LOG_TAG,
						"Succeeded in starting observing the sensor events regarding a HRM sensor listener.");
			break;
		case PRIVACY_PRIVILEGE_MANAGER_REQUEST_RESULT_DENY_FOREVER:
			dlog_print(DLOG_INFO, HRM_SENSOR_LOG_TAG,
					"Function request_sensor_permission_response_callback() output result = PRIVACY_PRIVILEGE_MANAGER_REQUEST_RESULT_DENY_FOREVER");
			dlog_print(DLOG_ERROR, HRM_SENSOR_LOG_TAG,
					"The user denied granting permission to use a sensor privilege for an indefinite period of time.");
			ui_app_exit();
			break;
		case PRIVACY_PRIVILEGE_MANAGER_REQUEST_RESULT_DENY_ONCE:
			dlog_print(DLOG_INFO, HRM_SENSOR_LOG_TAG,
					"Function request_sensor_permission_response_callback() output result = PRIVACY_PRIVILEGE_MANAGER_REQUEST_RESULT_DENY_ONCE");
			dlog_print(DLOG_ERROR, HRM_SENSOR_LOG_TAG,
					"The user denied granting permission to use a sensor privilege once.");
			ui_app_exit();
			break;
		}
	}
}

bool request_sensor_permission() {
	int health_retval;
	int mediastorage_retval;

	health_retval = ppm_request_permission(sensor_privilege,
			request_health_sensor_permission_response_callback, NULL);
	mediastorage_retval = ppm_request_permission(mediastorage_privilege,
			request_media_permission_response_callbacks, NULL);
	if (health_retval == PRIVACY_PRIVILEGE_MANAGER_ERROR_NONE
			&& mediastorage_retval == PRIVACY_PRIVILEGE_MANAGER_ERROR_NONE) {
		return true;
	} else if (health_retval
			== PRIVACY_PRIVILEGE_MANAGER_ERROR_ALREADY_IN_PROGRESS
			&& mediastorage_retval
					== PRIVACY_PRIVILEGE_MANAGER_ERROR_ALREADY_IN_PROGRESS) {
		return true;
	} else {
		dlog_print(DLOG_INFO, HRM_SENSOR_LOG_TAG,
				"Function ppm_request_permission() return value = %s",
				get_error_message(health_retval));
		dlog_print(DLOG_INFO, MEDIA_LOG_TAG,
				"Function ppm_request_permission() return value = %s",
				get_error_message(mediastorage_retval));
		return false;
	}
}

bool check_and_request_sensor_permission() {
	bool health_usable = true;
	bool media_usable = true;

	int health_retval;
	int mediastorage_retval;
	ppm_check_result_e health_result;
	ppm_check_result_e mediastorage_result;

	health_retval = ppm_check_permission(sensor_privilege, &health_result);
	mediastorage_retval = ppm_check_permission(mediastorage_privilege,
			&mediastorage_result);

	if (health_retval == PRIVACY_PRIVILEGE_MANAGER_ERROR_NONE) {
		switch (health_result) {
		case PRIVACY_PRIVILEGE_MANAGER_CHECK_RESULT_ALLOW:
			dlog_print(DLOG_INFO, HRM_SENSOR_LOG_TAG,
					"The application has permission to use a sensor privilege.");
			if (!check_hrm_sensor_listener_is_created()) {
				if (!initalize_hrm_sensor()) {
					dlog_print(DLOG_ERROR, HRM_SENSOR_LOG_TAG,
							"Failed to get the handle for the default sensor of a HRM sensor.");
					health_usable = false;
				} else {
					dlog_print(DLOG_INFO, HRM_SENSOR_LOG_TAG,
							"Succeeded in getting the handle for the default sensor of a HRM sensor.");
				}
				if (!create_sensor_listener(hrm_sensor_handle)) {
					dlog_print(DLOG_ERROR, HRM_SENSOR_LOG_TAG,
							"Failed to create a HRM sensor listener.");
					health_usable = false;
				} else
					dlog_print(DLOG_INFO, HRM_SENSOR_LOG_TAG,
							"Succeeded in creating a HRM sensor listener.");
				if (!start_hrm_sensor_listner())
					dlog_print(DLOG_ERROR, HRM_SENSOR_LOG_TAG,
							"Failed to start observing the sensor events regarding a HRM sensor listener.");
				else
					dlog_print(DLOG_INFO, HRM_SENSOR_LOG_TAG,
							"Succeeded in starting observing the sensor events regarding a HRM sensor listener.");
			}
			break;
		case PRIVACY_PRIVILEGE_MANAGER_CHECK_RESULT_DENY:
			dlog_print(
					"Function ppm_check_permission() output result = PRIVACY_PRIVILEGE_MANAGER_CHECK_RESULT_DENY");
			dlog_print(DLOG_ERROR, HRM_SENSOR_LOG_TAG,
					"The application doesn't have permission to use a sensor privilege.");
			health_usable = false;
			break;
		case PRIVACY_PRIVILEGE_MANAGER_CHECK_RESULT_ASK:
			dlog_print(DLOG_INFO, HRM_SENSOR_LOG_TAG,
					"The user has to be asked whether to grant permission to use a sensor privilege.");
			if (!request_sensor_permission()) {
				dlog_print(DLOG_ERROR, HRM_SENSOR_LOG_TAG,
						"Failed to request a user's response to obtain permission for using the sensor privilege.");
				health_usable = false;
			} else {
				dlog_print(DLOG_INFO, HRM_SENSOR_LOG_TAG,
						"Succeeded in requesting a user's response to obtain permission for using the sensor privilege.");
				health_usable = true;
			}
			break;
		}
	} else {
		dlog_print(DLOG_INFO, MEDIA_LOG_TAG,
				"Succeeded in requesting a user's response to obtain permission for using the sensor privilege.");
		media_usable = true;
	}
	if (mediastorage_retval == PRIVACY_PRIVILEGE_MANAGER_ERROR_NONE) {
		if (mediastorage_result
				== PRIVACY_PRIVILEGE_MANAGER_CHECK_RESULT_ALLOW) {
			dlog_print(DLOG_INFO, MEDIA_LOG_TAG,
					"The application has permission to use a storage privilege.");
		} else if (mediastorage_result
				== PRIVACY_PRIVILEGE_MANAGER_CHECK_RESULT_ASK) {
			dlog_print(DLOG_INFO, MEDIA_LOG_TAG,
					"The user has to be asked whether to grant permission to use a sensor privilege.");
			if (!request_sensor_permission()) {
				dlog_print(DLOG_ERROR, MEDIA_LOG_TAG,
						"Failed to request a user's response to obtain permission for using the sensor privilege.");
				media_usable = false;
			} else {
				dlog_print(DLOG_INFO, MEDIA_LOG_TAG,
						"Succeeded in requesting a user's response to obtain permission for using the sensor privilege.");
				media_usable = true;
			}
		} else {
			dlog_print(DLOG_INFO, MEDIA_LOG_TAG,
					"Function ppm_check_permission() output result = PRIVACY_PRIVILEGE_MANAGER_CHECK_RESULT_DENY");
			dlog_print(DLOG_ERROR, MEDIA_LOG_TAG,
					"The application doesn't have permission to use a sensor privilege.");
			media_usable = false;
		}
	} else {
		dlog_print(DLOG_ERROR, MEDIA_LOG_TAG,
				"Function ppm_check_permission() return %s",
				get_error_message(mediastorage_retval));
		media_usable = false;
	}
	return health_usable && media_usable;
}

void request_media_permission_response_callback(ppm_call_cause_e cause,
		ppm_request_result_e result, const char *privilege, void *user_data) {
}

