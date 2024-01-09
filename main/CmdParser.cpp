#include "CmdParser.h"

#include <stdio.h>
#include <string.h>

#include <vector>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static TaskHandle_t cmd_parser_task_handle = NULL;

void setTargetClient(uint8_t *target_client, uint8_t *user_in_buff) {
  for (uint8_t i = 2; i < CLIENT_MAC_SIZE + MODE_SIZE; i++) {
    if (user_in_buff[i] == 0) {
      break;
    }
    target_client[i] = user_in_buff[i];
  }
}

void setAp(uint8_t *target_ap, uint8_t *user_in_buff) {
  for (uint8_t i = 2; i < MAX_AP_CHAR + MODE_SIZE; i++) {
    if (user_in_buff[i] == 0) {
      break;
    }
    target_ap[i] = user_in_buff[i];
  }
}

void printCurrentCommand(uint8_t *user_in_buff) {
  printf("{CMD_PARSER,CURRENT_CMD,");
  for (uint8_t i = 0; i < MAX_AP_CHAR + MODE_SIZE; i++) {
    if (user_in_buff[i] == 0) {
      break;
    }
    printf("%c", user_in_buff[i]);
  }
  printf(",}\n");
}

void setAllZeroToUserInputBuffer(uint8_t *user_in_buff) {
  for (uint8_t i = 0; i < MAX_AP_CHAR + MODE_SIZE; i++) {
    user_in_buff[i] = 0;
  }
}

void cmdParser() {
  uint8_t user_in_buff[MODE_SIZE + MAX_AP_CHAR];
  setAllZeroToUserInputBuffer(user_in_buff);
  uint8_t target_client[CLIENT_MAC_SIZE];
  for (uint8_t i = 0; i < CLIENT_MAC_SIZE; i++) {
    target_client[i] = 0;
  }
  uint8_t target_ap[MAX_AP_CHAR];
  for (uint8_t i = 0; i < MAX_AP_CHAR; i++) {
    target_ap[i] = 0;
  }

  while (1) {
    vTaskDelay(CMD_INPUT_DELAY / portTICK_PERIOD_MS);
    scanf("%66s", user_in_buff);

    if (memcmp(user_in_buff, DEAUTH_START_MODE, MODE_SIZE) == 0) {
      setTargetClient(target_client, user_in_buff);
      printCurrentCommand(user_in_buff);
      setAllZeroToUserInputBuffer(user_in_buff);

      // TODO: Start deauthenticating target client
    } else if (memcmp(user_in_buff, DEAUTH_STOP_MODE, MODE_SIZE) == 0) {
      setTargetClient(target_client, user_in_buff);
      printCurrentCommand(user_in_buff);
      setAllZeroToUserInputBuffer(user_in_buff);

      // TODO: Stop deauthentication target client
    } else if (memcmp(user_in_buff, AP_START_MODE, MODE_SIZE) == 0) {
      setAp(target_ap, user_in_buff);
      printCurrentCommand(user_in_buff);
      setAllZeroToUserInputBuffer(user_in_buff);

      // TODO: Start access point mode to host web server
    } else if (memcmp(user_in_buff, AP_STOP_MODE, MODE_SIZE) == 0) {
      setAp(target_ap, user_in_buff);
      printCurrentCommand(user_in_buff);
      setAllZeroToUserInputBuffer(user_in_buff);

      // TODO: Stop the running access point
    }
  }
}

extern "C" void cmdParserCreateTask() {
  xTaskCreatePinnedToCore((TaskFunction_t)cmdParser, CMD_PARSER_TASK_NAME,
                          CMD_PARSER_STACK_SIZE, NULL, CMD_PARSER_TASK_PRIORITY,
                          &cmd_parser_task_handle, CMD_PARSER_CORE_TO_USE);
  printf("{CMD,STARTED,}\n");
}