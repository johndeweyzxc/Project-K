#pragma once
#include <stdint.h>

#include <cstddef>

void hexStringToBytes(const char *hex_string, uint8_t *result);

extern "C" {
bool httpdUriMatcher(const char *template_input, const char *uri, size_t len);
void webServerStop();
void webServerStart();
}