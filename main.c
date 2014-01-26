#include <SDL/SDL.h>
#include "def.h"

int main(int argc, char **argv)
{
    int tmp;
    context_t context;
    if(SDL_Init(SDL_INIT_VIDEO) < 0)
        return EXIT_FAILURE;
    atexit(SDL_Quit);
    if(!InitContext(&context))
    {
        fprintf(stderr, "Unable to load screen\n");
        return EXIT_FAILURE;
    }
    SDL_ShowCursor(0);
    SDL_WM_SetCaption("Minecraft Sokoban", NULL);
    SDL_WM_SetIcon(SDL_LoadBMP("ico32.bmp"), NULL);
    SDL_EnableKeyRepeat(125, 125);
    do
    {
        SDL_FreeSurface(context.screen);
        context.screen = SDL_SetVideoMode(768, 768, 32, SDL_HWSURFACE|SDL_DOUBLEBUF);
        tmp = Menu(context.screen);
        if(tmp == PLAY)
            Play(&context, 1);
        else if(tmp == EDIT)
            Editor(&context);
        else if(tmp == TILE)
            ChoseTileset(&context);
    }
    while(tmp != EXIT);
    FreeContext(&context);
    return EXIT_SUCCESS;
    (void)argc;
    (void)argv;
}
