#ifndef REQUEST_PARSER_H
#define REQUEST_PARSER_H

#include <stddef.h>
#include <string.h>

#define MAX_HEADERS 50
#define MAX_HEADER_LENGHT 1024
#define MAX_URL_LENGHT 512
#define MAX_METHOD_LENGHT 128
#define VERSION_LENGHT 128
#define MAX_BODY_LENGHT 1024

typedef struct {
  char method[MAX_METHOD_LENGHT];
  char url[MAX_URL_LENGHT];
  char version[VERSION_LENGHT];
  char headers[MAX_HEADERS][MAX_HEADER_LENGHT];
  size_t header_count;
  char body[MAX_BODY_LENGHT];
} ParsedRequest;

typedef struct {
    char host[128];
    char user_agent[256];
    char accept[256];
    char accept_language[128];
    char accept_encoding[128];
    char connection[128];
    char content_type[128];
    long content_length;
    char authorization[256];
    char cookie[512];
    char transfer_encoding[256];
    char content_encoding[256];
} ParsedHeader;

int ParseRequest(const char *raw_text, ParsedRequest *req);

int ParseHeader(ParsedRequest *req, ParsedHeader *head);

#endif
