#pragma once

#include <stdint.h>

extern "C" {
/**
 * @brief Injects raw arbitrary 802.11 frame
 * @param *frame_buffer Buffer that contains the frame to be injected
 * @param size Size of the frame buffer
 */
void injectFrame(uint8_t *frame_buffer, int size);
}