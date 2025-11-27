#include "request_response.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
  size_t totalsize = size * nmemb;
  RespBody *resp = (RespBody *)userdata;

  char *temp = realloc(resp->data, resp->size + totalsize + 1);
  if (!temp) return 0;

  resp->data = temp;
  memcpy(resp->data + resp->size, ptr, totalsize);
  resp->size += totalsize;
  resp->data[resp->size] = '\0';

  return totalsize;
}

static size_t header_callback(char *buffer, size_t size, size_t nitems, void *userdata) {
  size_t totalsize = size * nitems;
  RespHeader *hder = (RespHeader *)userdata;

  char *temp = realloc(hder->data, hder->size + totalsize + 1);
  if (!temp) return 0;

  hder->data = temp;
  memcpy(hder->data + hder->size, buffer, totalsize);
  hder->size += totalsize;
  hder->data[hder->size] = '\0';

  return totalsize;
}

int SendRequest(const ParsedRequest *req, const ParsedHeader *head, HTTPresponse *response) {
  if (!req || !head || !response) return -1;

  CURL *curl = curl_easy_init();
  if (!curl) return -1;

  char url[1024];
  if (strncmp(req->url, "http", 4) == 0) {
      snprintf(url, sizeof(url), "%s", req->url);
  } else {
      snprintf(url, sizeof(url), "http://%s%s", head->host, req->url);
  }

  curl_easy_setopt(curl, CURLOPT_URL, url);

  if (strstr(head->host, ".onion")) {
      curl_easy_setopt(curl, CURLOPT_PROXY, "127.0.0.1:9050");
      curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5_HOSTNAME);
  } else if (strstr(head->host, ".i2p")) {
      curl_easy_setopt(curl, CURLOPT_PROXY, "127.0.0.1:4444");
      curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_HTTP);
  }

  if (strcmp(req->method, "GET") == 0) {
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
  } else if (strcmp(req->method, "POST") == 0) {
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(req->body));
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, req->body);
  }

  struct curl_slist *header = NULL;
  const char *keys[] = {"Host","User-Agent","Accept","Accept-Language","Accept-Encoding","Connection","Content-Type","Content-Length","Authorization","Cookie"};
  static char content_length_buf[32];
  snprintf(content_length_buf, sizeof(content_length_buf), "%ld", head->content_length);
  const char *values[] = {head->host, head->user_agent, head->accept, head->accept_language, head->accept_encoding, head->connection, head->content_type, content_length_buf, head->authorization, head->cookie};
  int lenhead = 2;

  char buffer_header[4096];
  for (int i = 0; i < lenhead; ++i) {
    snprintf(buffer_header, sizeof(buffer_header), "%s: %s", keys[i], values[i]);
    header = curl_slist_append(header, buffer_header);
  }
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);

  RespHeader responshead = {NULL, 0};
  curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
  curl_easy_setopt(curl, CURLOPT_HEADERDATA, &responshead);

  RespBody responsbody = {NULL, 0};
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responsbody);

  CURLcode res = curl_easy_perform(curl);

  long status = 0;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);

  curl_slist_free_all(header);
  curl_easy_cleanup(curl);

  if (res != CURLE_OK) {
    fprintf(stderr, "[!] Curl error: %s\n", curl_easy_strerror(res));
    free(responshead.data);
    free(responsbody.data);
    return -1;
  }

  response->status = status;
  response->header = responshead.data;
  response->body = responsbody.data;

  return 0;
}

void FreeResponse(HTTPresponse *resp) {
  if (!resp) return;

  free(resp->header);
  free(resp->body);

  resp->header = NULL;
  resp->body = NULL;
  resp->status = 0;
}
