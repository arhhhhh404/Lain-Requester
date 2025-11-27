#include "request_parser.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int ParseRequest(const char *raw_text, ParsedRequest *req) {
  if (!raw_text || !req) return -1;

  memset(req, 0, sizeof(ParsedRequest));
  
  const char *current = raw_text;

  const char *line_end = strstr(current, "\r\n");
  if (!line_end) return -1;

  char first_line[1024];
  size_t first_line_len = line_end - current;
  if (first_line_len >= MAX_METHOD_LENGHT + MAX_URL_LENGHT) return -1;
  strncpy(first_line, current, first_line_len);
  first_line[first_line_len] = '\0';

  if (sscanf(first_line, "%15s" "%511s" "%31s", req->method, req->url, req->version) != 3) return -1;

  current = line_end + 2;
  req->header_count = 0;
  while(*current != '\0' && !(current[0] == '\r' && current[1] == '\n')) {
      if (req->header_count >= MAX_HEADER_LENGHT) break;

      const char *next_line = strstr(current, "\r\n");
      if (!next_line) break;

      size_t header_len = next_line - current;
      if (header_len >= MAX_HEADER_LENGHT) header_len = MAX_HEADER_LENGHT - 1;

      strncpy(req->headers[req->header_count], current, header_len);
      req->headers[req->header_count][header_len] = '\0';
      req->header_count++;

      current = next_line + 2;
  }

  if (current[0] == '\r' && current[1] == '\n') current += 2;

  strncpy(req->body, current, MAX_BODY_LENGHT - 1);
  req->body[MAX_BODY_LENGHT -1] = '\0';
  
  return 0;
}

int ParseHeader(ParsedRequest *req, ParsedHeader *head) {
  if (!req || !head) return -1;
  memset(head, 0, sizeof(ParsedHeader));

  for (size_t i = 0; i < req->header_count; ++i) {
    const char *line = req->headers[i];
    char *colon = strchr(line, ':');
    if (!colon) continue;

    size_t name_len = colon - line;
    char name[64];
    strncpy(name, line, name_len);
    name[name_len] = '\0';

    const char *value_start = colon + 1;
    while (*value_start == ' ') value_start++;
    char value[128];
    strncpy(value, value_start, sizeof(value)-1);
    value[sizeof(value)-1] = '\0';

    if (strcasecmp(name, "Host") == 0) {
      snprintf(head->host, sizeof(head->host), "%s", value);
    } else if (strcasecmp(name, "User-Agent") == 0) {
      snprintf(head->user_agent, sizeof(head->user_agent), "%s", value);
    } else if (strcasecmp(name, "Accept") == 0) {
      snprintf(head->accept, sizeof(head->accept), "%s", value);
    } else if (strcasecmp(name, "Accept-Language") == 0) {
      snprintf(head->accept_language, sizeof(head->accept_language), "%s", value);
    } else if (strcasecmp(name, "Accept-Encoding") == 0) {
      snprintf(head->accept_encoding, sizeof(head->accept_encoding), "%s", value);
    } else if (strcasecmp(name, "Connection") == 0) {
      snprintf(head->connection, sizeof(head->connection), "%s", value);
    } else if (strcasecmp(name, "Content-Type") == 0) {
      snprintf(head->content_type, sizeof(head->content_type), "%s", value);
    } else if (strcasecmp(name, "Content-Length") == 0) {
      head->content_length = strtol(value, NULL, 10);
    } else if (strcasecmp(name, "Authorization") == 0) {
      snprintf(head->authorization, sizeof(head->authorization), "%s", value);
    } else if (strcasecmp(name, "Cookie") == 0) {
      snprintf(head->cookie, sizeof(head->cookie), "%s", value);
    } else if (strcasecmp(name, "Transfer-Encoding") == 0) {
      snprintf(head->transfer_encoding, sizeof(head->transfer_encoding), "%s", value);
    } else if (strcasecmp(name, "Content-Encoding") == 0) {
      snprintf(head->content_encoding, sizeof(head->content_encoding), "%s", value);
    }
  }
  return 0;
}
