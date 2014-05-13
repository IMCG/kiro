#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "kiro-client.h"
#include "kiro-trb.h"
#include <SDL/SDL.h>
#include <assert.h>


static _Bool init_app(const char * name, SDL_Surface * icon, uint32_t flags)
{
    atexit(SDL_Quit);
    if(SDL_Init(flags) < 0)
        return 0;

    SDL_WM_SetCaption(name, name);
    SDL_WM_SetIcon(icon, NULL);

    return 1;
}

static void render(SDL_Surface * sf)
{
    SDL_Surface * screen = SDL_GetVideoSurface();
    if(SDL_BlitSurface(sf, NULL, screen, NULL) == 0)
        SDL_UpdateRect(screen, 0, 0, 0, 0);
}

static int filter(const SDL_Event * event)
{ return event->type == SDL_QUIT; }


int main ( int argc, char *argv[] )
{
    if (argc < 3)
    {
        printf("Not enough aruments. Usage: ./client <address> <port>\n");
        return -1;
    }
    KiroClient *client = g_object_new(KIRO_TYPE_CLIENT, NULL);
    if(-1 == kiro_client_connect(client, argv[1], argv[2]))
    {
        g_object_unref(client);
        return -1;
    }
    
    kiro_client_sync(client);
    KiroTrb *trb = g_object_new(KIRO_TYPE_TRB, NULL);
    kiro_trb_adopt(trb, kiro_client_get_memory(client));
    
    _Bool ok =
        init_app("UCA Images", NULL, SDL_INIT_VIDEO) &&
        SDL_SetVideoMode(512, 512, 8, SDL_HWSURFACE);

    assert(ok);

    uint32_t mask = 0xffffffff;
    SDL_Surface * data_sf = SDL_CreateRGBSurfaceFrom(
        kiro_trb_get_element(trb, 0), 512, 512, 8, 512,
        mask, mask, mask, 0);
    
    SDL_Color colors[256];
    for(int i=0;i<256;i++){
      colors[i].r=i;
      colors[i].g=i;
      colors[i].b=i;
    }    
    SDL_SetPalette(data_sf, SDL_LOGPAL|SDL_PHYSPAL, colors, 0, 256);
    
    SDL_SetEventFilter(filter);
    
    int cont = 1;
    
    //struct KiroTrbInfo *header = (struct KiroTrbInfo *)kiro_trb_get_raw_buffer(trb);
    
    while(cont)
    {
        for(SDL_Event event; SDL_PollEvent(&event);)
            if(event.type == SDL_QUIT) cont=0;
        
        kiro_client_sync(client);
        SDL_Delay(10);
        render(data_sf);
    }
        
        
    g_object_unref(client);
    return 0;
}








