#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include "include/request_parser.h"
#include "include/darknet_launcher.h"
#include "include/request_response.h"

#define WINDOW_LENGHT 800
#define WINDOW_HEIGHT 600

void SDL_ExitWithError(const char *error) {
  SDL_Log("[!] %s > %s", error, SDL_GetError());
  TTF_Quit();
  IMG_Quit();
  SDL_Quit();
  exit(EXIT_FAILURE);
}

void IMG_ExitWithError(const char *error) {
  SDL_Log("[!] %s > %s", error, IMG_GetError());
  TTF_Quit();
  IMG_Quit();
  SDL_Quit();
  exit(EXIT_FAILURE);
}

void TTF_ExitWithError(const char *error) {
  SDL_Log("[!] %s > %s", error, TTF_GetError());
  TTF_Quit();
  IMG_Quit();
  SDL_Quit();
  exit(EXIT_FAILURE);
}

int main(void) {
  SDL_Window *window = NULL;
  SDL_Renderer *renderer = NULL;

  if (SDL_Init(SDL_INIT_VIDEO) != 0) SDL_ExitWithError("Init SDL");
  if (TTF_Init() != 0) TTF_ExitWithError("Init TTF");
  if (IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) == 0) IMG_ExitWithError("Init IMG");
  
  if(SDL_CreateWindowAndRenderer(WINDOW_LENGHT, WINDOW_HEIGHT, 0, &window, &renderer) != 0) SDL_ExitWithError("Init Window Renderer");

  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
  
  TTF_Font *font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 16);
  if (!font) TTF_ExitWithError("Police TTF");

  SDL_Texture *background = NULL;
  SDL_Surface *bg_surface = IMG_Load("./images/lain_request.jpg");
  if (!bg_surface) IMG_ExitWithError("Background IMG");

  background = SDL_CreateTextureFromSurface(renderer, bg_surface);
  SDL_FreeSurface(bg_surface);
  
  SDL_SetWindowTitle(window, "Lain Requester");
  SDL_bool program_launched = SDL_TRUE;
  SDL_StartTextInput();  
  Uint32 cursor_timer = SDL_GetTicks();
  SDL_bool cursor_visible = SDL_TRUE;

  int request_scroll_offset = 0;
  int response_scroll_offset = 0;
  SDL_bool mouse_in_request = SDL_FALSE;
  SDL_bool mouse_in_response = SDL_FALSE;

  char request_text[4096] = "";
  char response_text[4096] = "Response there...";
  char send[256] = "SEND";
  SDL_Color textColor = {255, 255, 255, 255};
  
  ParsedHeader head = {0};
  ParsedRequest req = {0}; 
  HTTPresponse response = {0};

  while(program_launched) {
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        program_launched = SDL_FALSE;
      } else if (event.type == SDL_MOUSEMOTION) {
        int x = event.motion.x;
        int y = event.motion.y;
        
        mouse_in_request = (x >= 20 && x <= 380 && y >= 20 && y <= 520);
        mouse_in_response = (x >= 420 && x <= 780 && y >= 20 && y <= 520);
        
      } else if (event.type == SDL_MOUSEWHEEL) {
        if (mouse_in_request) {
          request_scroll_offset -= event.wheel.y * 20;
          if (request_scroll_offset < 0) request_scroll_offset = 0;
        } else if (mouse_in_response) {
          response_scroll_offset -= event.wheel.y * 20;
          if (response_scroll_offset < 0) response_scroll_offset = 0;
        }
      } else if (event.type == SDL_MOUSEBUTTONDOWN) {
        int x = event.button.x;
        int y = event.button.y;
        if (x >= 350 && x <= 450 && y >= 550 &&  y <= 580) {
          printf("[+]> Submit click\n");
          char http_request[4096];
          size_t j = 0;
          for (size_t i = 0; request_text[i] != '\0' && j < sizeof(http_request)-1; i++) {
            if (request_text[i] == '\n') {
              if (j < sizeof(http_request)-2) {
                http_request[j++] = '\r';
                http_request[j++] = '\n';
              } else {
                break;
              }
            } else {
              http_request[j++] = request_text[i];
            }
          }
          http_request[j] = '\0';
          printf("\nRequest: \n%s\n", http_request);
          FreeResponse(&response);
          memset(&response, 0, sizeof(HTTPresponse));

          response_scroll_offset = 0;

          if (ParseRequest(http_request, &req) == 0) {
            if (ParseHeader(&req, &head) == 0) {
              const char *TOR_BIN = "/usr/bin/tor";
              const char *TORRC   = "/etc/tor/torrc";
              const char *I2P_BIN = "/usr/bin/i2pd";
              const char *I2P_CONF = "/etc/i2pd/i2pd.conf";

              size_t host_len = strlen(head.host);
              if (host_len >= 6 && strcmp(head.host + host_len - 6, ".onion") == 0) {
                printf("[+] Detection .onion - Starting Tor\n");
                tor_launcher(TOR_BIN, TORRC);
              } else if (host_len >= 4 && strcmp(head.host + host_len - 4, ".i2p") == 0) {
                printf("[+] Detection .i2p - Starting I2P\n");
                i2p_launcher(I2P_BIN, I2P_CONF);
              }

              printf("\n[+]> Parsed complete\n");
            } else {
              printf("[!]> ERROR Parsedheader\n");
              strncpy(response_text, "Error: Invalid header format", sizeof(response_text) - 1);
              response_text[sizeof(response_text) - 1] = '\0';
              continue;
            }
          } else {
            printf("[!]> ERROR Parsedrequest\n");
            strncpy(response_text, "Error: Invalid request format", sizeof(response_text) - 1);
            response_text[sizeof(response_text) - 1] = '\0';
            continue;
          }

          if (SendRequest(&req, &head, &response) == 0) {
            if (response.body || response.header) {
              char strstatus[32];
              snprintf(strstatus, sizeof(strstatus), "Status: %ld\n", response.status);
              snprintf(response_text, sizeof(response_text), "%s%s%s", strstatus, response.header ? response.header : "", response.body ? response.body : "");
              printf("[+]> Request complete\n");
              printf("\nResponse: \n%s\n", response_text);
            } else {
              strncpy(response_text, "[-]> No response from the server", sizeof(response_text) - 1);
              response_text[sizeof(response_text) - 1] = '\0';
            }
          } else {
            printf("[!]> ERROR Sending\n");
            strncpy(response_text, "Error: Failed to send request", sizeof(response_text) - 1);
            response_text[sizeof(response_text) - 1] = '\0';
          }
        }
      } else if (event.type == SDL_TEXTINPUT) {
        if (strlen(request_text) + strlen(event.text.text) < sizeof(request_text) - 1) {
          strcat(request_text, event.text.text);
        }
      } else if (event.type == SDL_KEYDOWN) { 
          if (event.key.keysym.sym == SDLK_BACKSPACE) {
            size_t len_backspace = strlen(request_text);
            if (len_backspace > 0) {
              request_text[len_backspace - 1] = '\0';
            }
          } else if (event.key.keysym.sym == SDLK_RETURN) {
            size_t len_return = strlen(request_text);
            if (len_return < sizeof(request_text) - 2) {
              request_text[len_return] = '\n';
              request_text[len_return+1] = '\0';
            }
          } else if (event.key.keysym.sym == SDLK_UP) {
            request_scroll_offset -= 20;
            if (request_scroll_offset < 0) request_scroll_offset = 0;
          } else if (event.key.keysym.sym == SDLK_DOWN) {
            request_scroll_offset += 20;
          } else if ((event.key.keysym.mod & KMOD_CTRL) && event.key.keysym.sym == SDLK_v) {
            if (SDL_HasClipboardText()) {
              char *clipboard = SDL_GetClipboardText();
              if (clipboard) {
                size_t clipboard_len = strlen(clipboard);
                size_t current_len = strlen(request_text);
                size_t available = sizeof(request_text) - current_len - 1;
                
                if (clipboard_len <= available) {
                  strcat(request_text, clipboard);
                  printf("[+]> Text pasted from clipboard\n");
                } else {
                  strncat(request_text, clipboard, available);
                  printf("[!]> Clipboard text truncated (buffer full)\n");
                }
                SDL_free(clipboard);
              }
            }
          }
      } 
    }
    
    Uint32 now = SDL_GetTicks();
    if (now - cursor_timer > 500) {
      cursor_visible = !cursor_visible;
      cursor_timer = now;
    }
    
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, background, NULL, NULL);
    
    SDL_Rect request = {20, 20, 360, 500};
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
    SDL_RenderFillRect(renderer, &request);
    if (mouse_in_request) {
      SDL_SetRenderDrawColor(renderer, 150, 150, 150, 200);
      SDL_RenderDrawRect(renderer, &request);
    }
    
    SDL_Rect response_rect = {420, 20, 360, 500};
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
    SDL_RenderFillRect(renderer, &response_rect);
    if (mouse_in_response) {
      SDL_SetRenderDrawColor(renderer, 150, 150, 150, 200);
      SDL_RenderDrawRect(renderer, &response_rect);
    }
    
    SDL_Rect submit = {350, 550, 100, 30};
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(renderer, &submit);
    
    if (strlen(request_text) > 0) {
      SDL_Surface *request_surface = TTF_RenderText_Blended_Wrapped(font, request_text, textColor, 340);
      if (request_surface) {
        SDL_Texture *request_texture = SDL_CreateTextureFromSurface(renderer, request_surface);
        SDL_SetTextureBlendMode(request_texture, SDL_BLENDMODE_BLEND);
        SDL_SetTextureAlphaMod(request_texture, 180);
        
        int max_scroll_request = request_surface->h - 480;
        if (max_scroll_request < 0) max_scroll_request = 0;
        if (request_scroll_offset > max_scroll_request) request_scroll_offset = max_scroll_request;
        
        SDL_Rect clip_rect = {20, 20, 360, 500};
        SDL_RenderSetClipRect(renderer, &clip_rect);
        
        SDL_Rect request_text_rect = {30, 30 - request_scroll_offset, request_surface->w, request_surface->h};
        SDL_RenderCopy(renderer, request_texture, NULL, &request_text_rect);
        
        if(cursor_visible) {
          const char *last_line_start = request_text;
          const char *ptr = request_text;
          
          while (*ptr != '\0') {
            if (*ptr == '\n') {
              last_line_start = ptr + 1;
            }
            ptr++;
          }
          
          size_t last_line_len = strlen(last_line_start);

          char last_line[512] = {0};
          if (last_line_len > 0 && last_line_len < sizeof(last_line)) {
            memcpy(last_line, last_line_start, last_line_len);
            last_line[last_line_len] = '\0';
          }
          
          int line_width = 0;
          if (last_line_len > 0) {
            SDL_Surface *line_surface = TTF_RenderText_Blended(font, last_line, textColor);
            if (line_surface) {
              line_width = line_surface->w;
              SDL_FreeSurface(line_surface);
            }
          }
          
          int line_count = 1;
          ptr = request_text;
          while (*ptr != '\0') {
            if (*ptr == '\n') line_count++;
            ptr++;
          }
          
          int cursor_x = 30 + line_width;
          int cursor_y = 30 - request_scroll_offset + (line_count - 1) * 20;
          if (cursor_y >= 20 && cursor_y <= 520) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_Rect cursor_rect = { cursor_x, cursor_y, 2, 16 };
            SDL_RenderFillRect(renderer, &cursor_rect);
          }
          
          if (cursor_y + 16 > 510) {
            request_scroll_offset += (cursor_y + 16 - 510);
          } else if (cursor_y < 30) {
            request_scroll_offset -= (30 - cursor_y);
            if (request_scroll_offset < 0) request_scroll_offset = 0;
          }
        }
        
        SDL_RenderSetClipRect(renderer, NULL);
        
        SDL_FreeSurface(request_surface);
        SDL_DestroyTexture(request_texture);
      }
    } else {
      if(cursor_visible) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_Rect cursor_rect = { 32, 32 - request_scroll_offset, 2, 16 };
        SDL_RenderFillRect(renderer, &cursor_rect);
      }
    }
    
    if (strlen(response_text) > 0) {
      SDL_Surface *response_surface = TTF_RenderText_Blended_Wrapped(font, response_text, textColor, 340);
      if (response_surface) {
        SDL_Texture *response_texture = SDL_CreateTextureFromSurface(renderer, response_surface);
        SDL_SetTextureBlendMode(response_texture, SDL_BLENDMODE_BLEND);
        SDL_SetTextureAlphaMod(response_texture, 180);
        
        SDL_Rect clip_rect = {420, 20, 360, 500};
        SDL_RenderSetClipRect(renderer, &clip_rect);
        
        int max_scroll = response_surface->h - 480;
        if (max_scroll < 0) max_scroll = 0;
        if (response_scroll_offset > max_scroll) response_scroll_offset = max_scroll;
        
        SDL_Rect response_text_rect = {430, 30 - response_scroll_offset, response_surface->w, response_surface->h};
        SDL_RenderCopy(renderer, response_texture, NULL, &response_text_rect);
        
        SDL_RenderSetClipRect(renderer, NULL);
        
        SDL_FreeSurface(response_surface);
        SDL_DestroyTexture(response_texture);
      }
    }

    if (strlen(send) > 0) {
      SDL_Surface *send_surface = TTF_RenderText_Blended(font, send, textColor);
      if (send_surface) {
        SDL_Texture *send_texture = SDL_CreateTextureFromSurface(renderer, send_surface);
        SDL_SetTextureBlendMode(send_texture, SDL_BLENDMODE_BLEND);
        SDL_SetTextureAlphaMod(send_texture, 225);
        SDL_Rect send_text_rect = {submit.x + (submit.w - send_surface->w) / 2, submit.y + (submit.h - send_surface->h) / 2, send_surface->w, send_surface->h};
        SDL_RenderCopy(renderer, send_texture, NULL, &send_text_rect);
        SDL_FreeSurface(send_surface);
        SDL_DestroyTexture(send_texture);
      }
    }
    
    SDL_RenderPresent(renderer);
    SDL_Delay(16);
  }
  
  FreeResponse(&response);
  SDL_StopTextInput();
  SDL_DestroyTexture(background);
  TTF_CloseFont(font);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  TTF_Quit();
  IMG_Quit();
  SDL_Quit();
  printf("[-]> Finish.");

  return EXIT_SUCCESS;
}
