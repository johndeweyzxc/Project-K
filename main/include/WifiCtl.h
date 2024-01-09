#pragma once

extern "C" {
/**
 * @brief Stops the running access point
 */
void wifiCtlApStop();

/**
 * @brief Starts the access point
 */
void wifiCtlApStart();

/**
 * @brief Creates and configures and access point to be run
 */
void wifiCtlCreateAp();

/**
 * @brief Initializes the Wi-Fi controller of the ESP32
 */
void wifiCtlInit();
}
