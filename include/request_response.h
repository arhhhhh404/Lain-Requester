#ifndef REQUEST_SENDER_H
#define REQUEST_SENDER_H

#include <curl/curl.h>
#include "request_parser.h"

typedef struct {
  char *data;
  size_t size;
} RespHeader;

typedef struct {
  char *data;
  size_t size;
} RespBody;

typedef struct {
  long status;
  char *header;
  char *body;
} HTTPresponse;

int SendRequest(const ParsedRequest *req, const ParsedHeader *head, HTTPresponse *response);

void FreeResponse(HTTPresponse *resp);

#endif
