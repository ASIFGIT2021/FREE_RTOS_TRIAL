#ifndef SENSOR_INTERFACE_H
#define SENSOR_INTERFACE_H

#include <stdint.h>
#include <stddef.h>

/**
 * @brief Callback function invoked when data is received from a sensor.
 *
 * This function will be called automatically when new sensor data is available.
 *
 * @param sensor_id Sensor ID from where data is coming from
 * @param data Pointer to the location containing sensor data.
 * @param size Size of the incoming data
 */
void sensor_data_callback(uint8_t sensor_id, const void* data, uint8_t size);

#endif // SENSOR_INTERFACE_H

