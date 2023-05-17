/*
 * hrm_sen.h
 *
 *  Created on: May 12, 2023
 *      Author: kami1
 */

#ifndef SENSOR_HRM_SEN_H_
#define SENSOR_HRM_SEN_H_

#include <sensor_00.h>

bool create_hrm_sensor_listener(sensor_h hrm_sensor_handle);

bool set_hrm_sensor_listener_attribute();
bool set_hrm_sensor_listener_event_callback();

bool start_hrm_sensor_listener();
bool stop_hrm_sensor_listener();
bool destroy_hrm_sensor_listener();
bool check_hrm_sensor_listener_is_created();

#endif
