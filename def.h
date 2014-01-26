#ifndef DEF_H
#define DEF_H

#define WIDTH_CELL 64
#define HEIGHT_CELL 64
#define NUMBER_OF_SPRITES 9

#define PATH_TILESET "Tilesets\\tileset.bmp"
#define PATH_LEVELS  "mazes.txt"

#include <SDL/SDL.h>

typedef enum {EMPTY, WALL, LOCATION, PUMPKIN, OK} index_t;
typedef enum {DOWN = 5, RIGHT, LEFT, UP} state_t;

typedef struct
{
    SDL_Rect pos[NUMBER_OF_SPRITES];
    SDL_Surface *image;
}
tileset_t;

typedef struct
{
    tileset_t tileset;
    SDL_Surface *screen;
    unsigned int rows, columns;
    index_t *board;
    SDL_Rect CharPos;
    state_t CharState;
}
context_t;

/*
    these functions are used to initiate and free the main variable : the context (IniTileset is called by InitContext)
*/
SDL_bool InitContext(context_t *ctxt);
void InitTileset(tileset_t *tileset);
void FreeContext(context_t *ctxt);

void BlitAll(context_t *ctxt);
/*
    Loads an image and converts it to 32 bits if it is not
*/
SDL_Surface *LoadImage(const char *path);

#define PLAY 1
#define EDIT 2
#define TILE 3
#define EXIT 4
int Menu(SDL_Surface *screen);

void ChoseTileset(context_t *ctxt);
/*
    returns SDL_TRUE if a LOCATION or a PUMPKIN is still on the board
*/
SDL_bool isover(context_t *ct);
int Play(context_t *ctxt, unsigned int level);
/*
    checks if moving is alowed on this cell of the maze and does if it is
*/
void Move(context_t *ctxt, state_t direction);
void Editor(context_t *ctxt);
/*
    Updates the context_t structre after resizing the screen
*/
void UpdateContext(context_t *ctxt, unsigned int y, unsigned int x);

#endif
