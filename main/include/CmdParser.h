#pragma once

#include <stdint.h>

#define MODE_SIZE 2
#define MAX_AP_CHAR 64
#define CLIENT_MAC_SIZE 12

#define NULL_MODE "00"
#define DEAUTH_START_MODE "01"
#define DEAUTH_STOP_MODE "02"
#define AP_START_MODE "03"
#define AP_STOP_MODE "04"

#define CMD_INPUT_DELAY 1000
#define CMD_PARSER_TASK_NAME "CMD_PARSER_TASK"
#define CMD_PARSER_STACK_SIZE 2048
#define CMD_PARSER_TASK_PRIORITY 2
#define CMD_PARSER_CORE_TO_USE 1

void setTargetClient(uint8_t *target_client, uint8_t *user_in_buff);

void setAp(uint8_t *target_ap, uint8_t *user_in_buff);

void printCurrentCommand(uint8_t *user_in_buff);

void setAllZeroToUserInputBuffer(uint8_t *user_in_buff);

void cmdParser();

extern "C" {
void cmdParserCreateTask();
}