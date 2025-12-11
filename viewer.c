#include <SDL3/SDL.h>


#include <assert.h>
#include <stdlib.h>

#include <pretty.h>

#include "types.h"
#include "decode.h"

static inline void update_texture_from_surface(SDL_Texture* texture, SDL_Surface* surface){
  
  float tex_w, tex_h;
  SDL_GetTextureSize(texture, &tex_w, &tex_h);
    
  void* pixels;
  int pitch;
  u32 bbp = SDL_GetPixelFormatDetails(surface->format) -> bytes_per_pixel;
    
  if ( SDL_LockTexture(texture, NULL, &pixels, &pitch) )
    for (int y = 0; y < surface->h; y++) {
      char* src_row = (char*)surface->pixels + (y * surface->pitch);
      char* dst_row = (char*)pixels + (y * pitch);
      memcpy(dst_row, src_row, surface->w*bbp);
    }
  SDL_UnlockTexture(texture); 
}


static inline void set_pixel(SDL_Surface* surface, SDL_Palette* palette, u32 x, u32 y, u8 r, u8 g, u8 b){ 
  char* pixel = surface->pixels + y * surface->pitch + x * 4; 
  SDL_PixelFormatDetails *pfd = SDL_GetPixelFormatDetails(surface -> format); 
  u32 color = SDL_MapRGBA(pfd, palette, r, g, b, 255);
  *(uint *)pixel = color;

}

static inline void set_pixels(SDL_Surface* surface, SDL_Palette* palette, u32 width, u32 height, u8* buffer){
  if ( SDL_SURFACE_LOCK_NEEDED ) {
    SDL_LockSurface(surface);
  }
  
  for(uint y = 0; y < height; y++) {
    for (uint x = 0; x < width; x++) {
      uint ppm_index = (y * width + x) * 3;
      
      // Read RGB values from PPM buffer
      char r = buffer[ppm_index];
      char g = buffer[ppm_index + 1];
      char b = buffer[ppm_index + 2];
      
      // Set pixel in surface (converts to RGBA8888)
      set_pixel(surface, palette, x, y, r, g, b);
    }
  }
  
  if (SDL_SURFACE_LOCK_NEEDED) SDL_UnlockSurface(surface);

}

void display_ppm_p6(u8* buffer){
  if (!SDL_Init(SDL_INIT_VIDEO)){
    error("Error initializing the video subsystem for SDL3!: %s", SDL_GetError());
    exit(EXIT_FAILURE);
  }

  assert(buffer[0] == 'P' && buffer[1] == '6' && buffer[2] == '\n');

  u32 width = 0, height = 0;
  u64 i = 3;
  while(buffer[i] != ' '){
    width = width * 10 + buffer[i] - '0';
    i++;
  }
  i++;

  while(buffer[i] != '\n'){
    height = height * 10 + buffer[i] - '0';
    i++;
  }
  i++;

  SDL_Window* win = SDL_CreateWindow("P6 Viewer", width, height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_BORDERLESS);
  if ( win == NULL ){
    error("Error Creating a Window!: %s", SDL_GetError());
    SDL_Quit();
    exit(EXIT_FAILURE);
  }

  SDL_Surface* surface = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA8888);
  if ( surface == NULL ){
    error("Error creating a Surface!: %s", SDL_GetError());
    SDL_DestroyWindow(win);
    SDL_Quit();
    exit(EXIT_FAILURE);
  }


  SDL_Renderer* renderer = SDL_CreateRenderer(win, NULL);
  if ( renderer == NULL ){
    error("Error creating a renderer!: %s", SDL_GetError());
    SDL_DestroySurface(surface);
    SDL_DestroyWindow(win);
    SDL_Quit();
    exit(EXIT_FAILURE);
  }

  SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, width, height);
  if ( texture == NULL ){
    error("Error creating a texture!: %s", SDL_GetError());
    SDL_DestroyRenderer(renderer);
    SDL_DestroySurface(surface);
    SDL_DestroyWindow(win);
    SDL_Quit();
    exit(EXIT_FAILURE);
  }
  SDL_Palette* palette = SDL_CreatePalette(256);
  if (palette == NULL){
    error("Error creating a Palette!: %s", SDL_GetError());
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroySurface(surface);
    SDL_DestroyWindow(win);
    SDL_Quit();
    exit(EXIT_FAILURE); 
  }
  
  set_pixels(surface, palette, width, height, buffer);

  SDL_RenderClear(renderer);
  update_texture_from_surface(texture, surface);
  SDL_RenderTexture(renderer, texture, NULL, NULL);
  SDL_RenderPresent(renderer);

  SDL_Event event;
  bool run = true;

  while ( run ){
    SDL_PollEvent(&event);
    switch(event.type){
      case SDL_EVENT_QUIT:
      case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        run = false; 
        break;
      default:
    }
  }

  SDL_DestroyPalette(palette);
  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroySurface(surface);
  SDL_DestroyWindow(win);
  SDL_Quit();
}




void display_qoi(u8* buffer){
  u8* p6_buffer;
  decode(buffer, &p6_buffer);
  if (!SDL_Init(SDL_INIT_VIDEO)){
    error("Error initializing the video subsystem for SDL3!: %s", SDL_GetError());
    exit(EXIT_FAILURE);
  }

  assert(p6_buffer[0] == 'P' && p6_buffer[1] == '6' && p6_buffer[2] == '\n');

  u32 width = 0, height = 0;
  u64 i = 3;
  while(p6_buffer[i] != ' '){
    width = width * 10 + p6_buffer[i] - '0';
    i++;
  }
  i++;

  while(p6_buffer[i] != '\n'){
    height = height * 10 + p6_buffer[i] - '0';
    i++;
  }
  i++;

  SDL_Window* win = SDL_CreateWindow("P6 Viewer", width, height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_BORDERLESS);
  if ( win == NULL ){
    error("Error Creating a Window!: %s", SDL_GetError());
    SDL_Quit();
    exit(EXIT_FAILURE);
  }

  SDL_Surface* surface = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA8888);
  if ( surface == NULL ){
    error("Error creating a Surface!: %s", SDL_GetError());
    SDL_DestroyWindow(win);
    SDL_Quit();
    exit(EXIT_FAILURE);
  }


  SDL_Renderer* renderer = SDL_CreateRenderer(win, NULL);
  if ( renderer == NULL ){
    error("Error creating a renderer!: %s", SDL_GetError());
    SDL_DestroySurface(surface);
    SDL_DestroyWindow(win);
    SDL_Quit();
    exit(EXIT_FAILURE);
  }

  SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, width, height);
  if ( texture == NULL ){
    error("Error creating a texture!: %s", SDL_GetError());
    SDL_DestroyRenderer(renderer);
    SDL_DestroySurface(surface);
    SDL_DestroyWindow(win);
    SDL_Quit();
    exit(EXIT_FAILURE);
  }
  SDL_Palette* palette = SDL_CreatePalette(256);
  if (palette == NULL){
    error("Error creating a Palette!: %s", SDL_GetError());
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroySurface(surface);
    SDL_DestroyWindow(win);
    SDL_Quit();
    exit(EXIT_FAILURE); 
  }
  
  set_pixels(surface, palette, width, height, p6_buffer);

  SDL_RenderClear(renderer);
  update_texture_from_surface(texture, surface);
  SDL_RenderTexture(renderer, texture, NULL, NULL);
  SDL_RenderPresent(renderer);

  SDL_Event event;
  bool run = true;

  while ( run ){
    SDL_PollEvent(&event);
    switch(event.type){
      case SDL_EVENT_QUIT:
      case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        run = false; 
        break;
      default:
    }
  }

  SDL_DestroyPalette(palette);
  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroySurface(surface);
  SDL_DestroyWindow(win);
  SDL_Quit();
  free(p6_buffer);
}
