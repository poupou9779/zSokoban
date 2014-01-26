#include <stdio.h>
#include "def.h"
#include <dirent.h>
#include <string.h>

SDL_bool InitContext(context_t *ctxt)
{
    ctxt->screen = SDL_SetVideoMode(768, 768, 32, SDL_HWSURFACE|SDL_DOUBLEBUF);
    if(ctxt->screen == NULL)
    {
        fprintf(stderr, "Unable to get the screen : %s\n", SDL_GetError());
        return SDL_FALSE;
    }
    InitTileset(&ctxt->tileset);
    ctxt->CharState = DOWN;
    return SDL_TRUE;
}

void InitTileset(tileset_t *tileset)
{
    int i;
    for(i = 0; i < NUMBER_OF_SPRITES; ++i)
    {
        tileset->pos[i].h = HEIGHT_CELL;
        tileset->pos[i].w = WIDTH_CELL;
        tileset->pos[i].x = i * WIDTH_CELL;
        tileset->pos[i].y = 0;
    }
    tileset->image = LoadImage(PATH_TILESET);
}

void FreeContext(context_t *ctxt)
{
    free(ctxt->board);
    ctxt->board = NULL;
}

SDL_Surface* LoadImage(const char* path)
{
    SDL_Surface *ret;
    SDL_Surface *tmp = SDL_LoadBMP(path);
    if(tmp==NULL)
    {
        fprintf(stderr, "Unable to load file : %s\n",path);
        exit(EXIT_FAILURE);
    }
    ret = SDL_DisplayFormat(tmp);
    SDL_FreeSurface(tmp);
    return ret;
}

void BlitAll(context_t *ctxt)
{
    unsigned int i, j;
    SDL_Rect dest;
    dest.h = HEIGHT_CELL;
    dest.w = WIDTH_CELL;
    for(i = 0; i < ctxt->rows; ++i)
    {
        for(j = 0; j < ctxt->columns; ++j)
        {
            dest.x = j * WIDTH_CELL;
            dest.y = i * HEIGHT_CELL;
            SDL_BlitSurface(ctxt->tileset.image, &ctxt->tileset.pos[ctxt->board[i*ctxt->columns + j]],
                            ctxt->screen, &dest);
        }
    }
    dest.x = ctxt->CharPos.x * WIDTH_CELL;
    dest.y = ctxt->CharPos.y * HEIGHT_CELL;
    SDL_BlitSurface(ctxt->tileset.image, &ctxt->tileset.pos[ctxt->CharState], ctxt->screen, &dest);
}

int Menu(SDL_Surface *screen)
{
    SDL_Event e;
    SDL_bool _continue = SDL_TRUE;
    SDL_Surface *image = LoadImage("Menu.bmp");
    SDL_BlitSurface(image, NULL, screen, NULL);
    SDL_Flip(screen);
    while(_continue)
    {
        SDL_WaitEvent(&e);
        if(e.type == SDL_KEYDOWN)
            if((e.key.keysym.sym < SDLK_5 || e.key.keysym.sym < SDLK_KP5)
               && (e.key.keysym.sym > SDLK_0 || e.key.keysym.sym > SDLK_KP0))
                _continue = SDL_FALSE;
    }
    return e.key.keysym.sym - (e.key.keysym.sym - SDLK_0 < 5 ?  SDLK_0 : SDLK_KP0);
}

SDL_bool isover(context_t *ct)
{
    unsigned int i, j;
    for(i = 0; i < ct->rows; ++i)
        for(j = 0; j < ct->columns; ++j)
            if(ct->board[i*ct->columns + j] == LOCATION || ct->board [i*ct->columns + j] == PUMPKIN)
                return SDL_FALSE;
    return SDL_TRUE;
}

int Play(context_t *ctxt, unsigned int level)
{
    SDL_Event e;
    unsigned int i, j, n;
    FILE *levelFile = fopen(PATH_LEVELS, "r");
    if(levelFile  == NULL)
        return -1;
    for(n = 0; n < level; ++n)
    {
        if(feof(levelFile))
        {
            fclose(levelFile);
            return 1;
        }
        if(fscanf(levelFile, "%d %d\n", &ctxt->rows, &ctxt->columns) < 2)
            return 0;
        if(ctxt->board != NULL)
            free(ctxt->board);
        ctxt->board = malloc(ctxt->columns * ctxt->rows * sizeof(*ctxt->board));
        for(i = 0; i < ctxt->rows; ++i)
            for(j = 0; j < ctxt->columns; ++j)
                if(fscanf(levelFile, "%d%*c", (int*)(&ctxt->board[i*ctxt->columns+j])) < 1)
                    return 0;
        if(fscanf(levelFile, "%d %d\n", (int*)&ctxt->CharPos.x, (int*)&ctxt->CharPos.y) < 2)
            return 0;
    }
    fclose(levelFile);
    SDL_FreeSurface(ctxt->screen);
    ctxt->screen = SDL_SetVideoMode(ctxt->columns * WIDTH_CELL, ctxt->rows * HEIGHT_CELL,
                                    32, SDL_DOUBLEBUF|SDL_HWSURFACE/*|SDL_RESIZABLE*/);
    if(ctxt->screen == NULL)
    {
        fprintf(stderr, "Unable to create screen : %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    while(!isover(ctxt))
    {
        BlitAll(ctxt);
        SDL_Flip(ctxt->screen);
        do
            SDL_WaitEvent(&e);
        while(e.type != SDL_KEYDOWN && e.type != SDL_QUIT);
        if(e.type == SDL_QUIT || e.key.keysym.sym == SDLK_ESCAPE)
            return 0;
        switch(e.key.keysym.sym)
        {
        case SDLK_r:
            return Play(ctxt, level);
            break;
        case SDLK_UP:
            ctxt->CharState = UP;
            Move(ctxt, UP);
            break;
        case SDLK_RIGHT:
            ctxt->CharState = RIGHT;
            Move(ctxt, RIGHT);
            break;
        case SDLK_LEFT:
            ctxt->CharState = LEFT;
            Move(ctxt, LEFT);
            break;
        case SDLK_DOWN:
            ctxt->CharState = DOWN;
            Move(ctxt, DOWN);
        default:
            break;
        }
    }
    BlitAll(ctxt);
    SDL_Flip(ctxt->screen);
    SDL_Delay(1000);
    return Play(ctxt, level+1);
}

void Move(context_t *ctxt, state_t direction)
{
    unsigned int currCoord = ctxt->CharPos.y*ctxt->columns + ctxt->CharPos.x,
                 nextCoord,
                 thrdCoord;
    int movingCoord[4] = {+0, +1, -1, -0},
        movingYaxis[4] = {+1, +0, -0, -1},
        movingXaxis[4] = {-0, +1, -1, +0};
    movingCoord[0] = ctxt->columns; movingCoord[3] = -movingCoord[0];
    nextCoord = currCoord + movingCoord[direction - DOWN];
    if(nextCoord >= 144)
        return;
    else if(ctxt->board[nextCoord] == PUMPKIN || ctxt->board[nextCoord] == OK)
    {
        if(((ctxt->CharPos.x+2 == (int)(ctxt->columns)) && direction == RIGHT) ||
           (ctxt->CharPos.x == 1 && direction == LEFT) ||
           (ctxt->CharPos.y+2 == (int)(ctxt->rows) && direction == DOWN) ||
           (ctxt->CharPos.y == 1 && direction == UP)
          ) return;
        thrdCoord = nextCoord + movingCoord[direction - DOWN];
        if(ctxt->board[thrdCoord] == EMPTY)
            ctxt->board[thrdCoord] = PUMPKIN;
        else if(ctxt->board[thrdCoord] == LOCATION)
            ctxt->board[thrdCoord] = OK;
        else
            return;
        ctxt->board[nextCoord] = ctxt->board[nextCoord] == PUMPKIN ? EMPTY : LOCATION;
    }
    else if(ctxt->board[nextCoord] != EMPTY && ctxt->board[nextCoord] != LOCATION)
        return;
    ctxt->CharPos.y += movingYaxis[direction - DOWN];
    ctxt->CharPos.x += movingXaxis[direction - DOWN];
    if(ctxt->CharPos.y < 0) ctxt->CharPos.y = 0;
    if(ctxt->CharPos.x < 0) ctxt->CharPos.x = 0;
    if(ctxt->CharPos.y >= (int)(ctxt->rows)) ctxt->CharPos.y = ctxt->rows-1;
    if(ctxt->CharPos.x >= (int)(ctxt->columns)) ctxt->CharPos.x = ctxt->columns-1;
}

void UpdateContext(context_t *ctxt, unsigned int y, unsigned int x)
{
    unsigned int i, j;
    index_t *tab_tmp;
    if(y < 1 || x < 1)
        return;
    tab_tmp = malloc(y * x * sizeof(*tab_tmp));
    if(tab_tmp != NULL)
    {
        for(i = 0; i < y; ++i)
            for(j = 0; j < x; ++j)
                tab_tmp[i*x + j] = i < ctxt->rows && j < ctxt->columns ? ctxt->board[i*ctxt->columns + j] : EMPTY;
        /*replace the board*/
        free(ctxt->board);
        ctxt->board = NULL;
        ctxt->board = tab_tmp;
        /*update the values*/
        ctxt->columns = x;
        ctxt->rows = y;
        /*update the screen*/
        SDL_FreeSurface(ctxt->screen);
        ctxt->screen = SDL_SetVideoMode(ctxt->columns*WIDTH_CELL, ctxt->rows*HEIGHT_CELL, 32, SDL_HWSURFACE|SDL_DOUBLEBUF);
    }
}

void Editor(context_t *ctxt)
{
    SDL_bool _continue = SDL_TRUE;
    SDL_Event e;
    int xmouse, ymouse;
    unsigned int i, j,
                 tmp = EMPTY;
    FILE *save;
    SDL_Rect src, dst;
    src.h = dst.h = HEIGHT_CELL;
    src.w = dst.w = WIDTH_CELL;
    src.y = 0;
    src.x = 0;
#define N_W 3
#define N_H 2
    ctxt->rows = N_H; ctxt->columns = N_W;
    ctxt->CharPos.x = 0; ctxt->CharPos.y = 0;
    ctxt->CharState = DOWN;
    ctxt->screen = SDL_SetVideoMode(ctxt->columns * WIDTH_CELL, ctxt->rows * HEIGHT_CELL, 32, SDL_HWSURFACE|SDL_DOUBLEBUF);
    ctxt->board = malloc(ctxt->columns * ctxt->rows * sizeof(*ctxt->board));
    for(i = 0; i < ctxt->rows; ++i)
        for(j = 0; j < ctxt->columns; ++j)
            ctxt->board[i*ctxt->columns + j] = EMPTY;
    do
    {
        SDL_WaitEvent(&e);
        SDL_Delay(1);
        if(e.type == SDL_KEYDOWN)
        {
            if(e.key.keysym.sym == SDLK_RETURN)
            {
                save = fopen(PATH_LEVELS, "a");
                if(save != NULL)
                {
                    fprintf(save, "%d %d\n", ctxt->rows, ctxt->columns);
                    for(i = 0; i < ctxt->rows; ++i)
                        for(j = 0; j < ctxt->columns; ++j)
                            fprintf(save, "%d%c", ctxt->board[i*ctxt->columns + j], j+1 == ctxt->columns ? '\n' : ' ');
                    fprintf(save, "%d %d\n", ctxt->CharPos.x, ctxt->CharPos.y);
                    fclose(save);
                    save = NULL;
                }
            }
            else if(e.key.keysym.sym == SDLK_ESCAPE)
                _continue = SDL_FALSE;
            else if(e.key.keysym.sym == SDLK_DOWN)
                UpdateContext(ctxt, ctxt->rows+1, ctxt->columns);
            else if(e.key.keysym.sym == SDLK_UP)
                UpdateContext(ctxt, ctxt->rows-1, ctxt->columns);
            else if(e.key.keysym.sym == SDLK_LEFT)
                UpdateContext(ctxt, ctxt->rows, ctxt->columns-1);
            else if(e.key.keysym.sym == SDLK_RIGHT)
                UpdateContext(ctxt, ctxt->rows, ctxt->columns+1);
        }
        else if(e.type == SDL_MOUSEBUTTONDOWN)
        {
            if(e.button.button == SDL_BUTTON_WHEELDOWN)
                --tmp;
            else if(e.button.button == SDL_BUTTON_WHEELUP)
                ++tmp;
            else if(e.button.button == SDL_BUTTON_LEFT)
            {
                (void)SDL_GetMouseState(&xmouse, &ymouse);
                if(tmp != DOWN)
                    ctxt->board[(ymouse/HEIGHT_CELL)*ctxt->columns + (xmouse/WIDTH_CELL)] = tmp;
                else
                {
                    ctxt->CharPos.x = xmouse/WIDTH_CELL;
                    ctxt->CharPos.y = ymouse/HEIGHT_CELL;
                }
            }
            tmp = (tmp+RIGHT) % RIGHT;
            src.x = tmp * src.w;
        }
        else if(e.type == SDL_MOUSEMOTION)
        {
            dst.x = e.motion.x;
            dst.y = e.motion.y;
        }
        SDL_FillRect(ctxt->screen, NULL, 0xFFFFFF);
        BlitAll(ctxt);
        SDL_BlitSurface(ctxt->tileset.image, &src, ctxt->screen, &dst);
        SDL_Flip(ctxt->screen);
        SDL_Delay(1);
    } while(_continue);
}

void ChoseTileset(context_t *ctxt)
{
    DIR *tilesetDir = opendir("Tilesets\\");
    SDL_Surface **tabTilesets;
    SDL_Surface *swap;
    SDL_Rect dst;
    SDL_Event e;
    struct dirent *CurrFile;
    int cpt = 0, i;
    char tmp[50];

    SDL_ShowCursor(1);
    while((CurrFile = readdir(tilesetDir)) != NULL)
    {
        if(strstr(CurrFile->d_name, ".bmp") != NULL)
            ++cpt;
    }
    tabTilesets = malloc(cpt * sizeof(SDL_Surface *));
    if(tabTilesets != NULL)
    {
        rewinddir(tilesetDir);
        cpt = 0;
        while((CurrFile = readdir(tilesetDir)) != NULL)
        {
            if(strstr(CurrFile->d_name, ".bmp") != NULL)
            {
                sprintf(tmp, "Tilesets\\%s", CurrFile->d_name);
                tabTilesets[cpt] = LoadImage(tmp);
                ++cpt;
            }
        }
        SDL_FreeSurface(ctxt->screen);
        ctxt->screen = SDL_SetVideoMode(NUMBER_OF_SPRITES*WIDTH_CELL, cpt*HEIGHT_CELL, 32, SDL_HWSURFACE);
        dst.h = HEIGHT_CELL;
        dst.w = WIDTH_CELL;
        dst.x = 0;
        for(i = 0; i < cpt; ++i)
        {
            dst.y = HEIGHT_CELL * i;
            SDL_BlitSurface(tabTilesets[i], NULL, ctxt->screen, &dst);
        }
        SDL_Flip(ctxt->screen);
        do
            SDL_WaitEvent(&e);
        while(e.type != SDL_MOUSEBUTTONDOWN || e.button.button != SDL_BUTTON_LEFT);
        swap = ctxt->tileset.image;
        ctxt->tileset.image = tabTilesets[e.motion.y/HEIGHT_CELL];
        tabTilesets[e.motion.y/HEIGHT_CELL] = swap;
        for(i = 0; i < cpt; ++i)
            SDL_FreeSurface(tabTilesets[i]);
        free(tabTilesets);
    }
    SDL_ShowCursor(0);
}

