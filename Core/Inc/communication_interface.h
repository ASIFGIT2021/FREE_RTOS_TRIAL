#ifndef COMMUNICATION_INTERFACE_H
#define COMMUNICATION_INTERFACE_H

#include <stdint.h>
#include <stddef.h>

/**
 * @brief Enum for communication status codes.
 */
typedef enum {
    COMM_STATUS_SUCCESS = 0,                 /**< Communication was successful. */
    COMM_STATUS_ERROR,                       /**< General communication error. */
    COMM_STATUS_TIMEOUT,                     /**< Communication timeout occurred. */
    COMM_STATUS_INVALID_PACKET_LENGTH,       /**< Packet length is invalid (e.g., exceeds the maximum size). */
    COMM_STATUS_INVALID_PACKET_LOCATION,     /**< Packet location is invalid (e.g., NULL pointer). */
    COMM_STATUS_UNKNOWN
} CommStatus;


/**
 * @brief Sends a packet of data to MCU 2.
 *
 * This function transmits the data wirelessly to MCU 2. It adheres to the protocol constraints:
 * - Maximum packet size: 200 bytes
 * - Minimum transmission interval: 10 ms
 *
 * @param packet Pointer to the packet data to be sent.
 * @param packet_length Length of the packet data in bytes.
 * @return A value of type CommStatus indicating the result of the operation.
 */
CommStatus send_data_to_mcu2(const uint8_t *packet, size_t packet_length);

#endif // COMMUNICATION_INTERFACE_H
