#include "raylib.h"
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#define BOARD_SIZE 8
#define TILE_SIZE 32
#define TILE_TYPES 5
#define MAX_SCORE_POPUPS 32

const char tile_chars[TILE_TYPES] = {'#', '$', '@', '%', '&'};
Vector2 grid_origin;
Texture2D backgroundText;
int score = 000000;
float fallSpeed = 8.0f;
float matchDelayTimer = 0.0f;
const float MATCH_DELAY_DURATION = 0.2f;

float scoreScale = 1.0f;
float scoreScaleVelocity = 0.0f;
bool scoreAnimating =false;

char board[BOARD_SIZE][BOARD_SIZE];
bool matched[BOARD_SIZE][BOARD_SIZE] = {false};
float fallOffsets[BOARD_SIZE][BOARD_SIZE] = {0};
typedef struct
{
    Vector2 position;
    int amount;
    float lifetime;
    float alpha;
    bool active;
}scorePopup;

scorePopup scorePopups[MAX_SCORE_POPUPS] = {0};

typedef enum
{
    STATE_IDLE,
    STATE_ANIMATING,
    STATE_MATCH_DELAY
} ETileState;

ETileState TileState;

char randomTile(void)
{
    return tile_chars[rand() % TILE_TYPES];
}

void addScorePopup(int x , int y , int amount)
{
    for(int i = 0; i < MAX_SCORE_POPUPS; i++)
    {
        if(!scorePopups[i].active)
        {
            scorePopups[i].position = (Vector2){grid_origin.x + x * TILE_SIZE + TILE_SIZE / 2,
                                                grid_origin.y + y * TILE_SIZE + TILE_SIZE / 2};
            scorePopups[i].amount = amount;
            scorePopups[i].lifetime = 1.0f;
            scorePopups[i].alpha = 1.0f;
            scorePopups[i].active = true;   
            break;
        }
    }
}
bool SearchHorizontally(void)
{
        for (int y = 0; y < BOARD_SIZE; y++)
    {
        for (int x = 0; x < BOARD_SIZE - 2; x++)
        {
            char t = board[y][x];
            if (t == board[y][x + 1] && t == board[y][x + 2])
            {
                matched[y][x] = matched[y][x + 1] = matched[y][x + 2] = true;
                score += 10;
                
                // playSound()

                scoreAnimating = true;
                scoreScale = 2.0f;
                scoreScaleVelocity = -2.5f;
                addScorePopup(x,y,10);
                return true;
            }
        }
    }
    return false;
}
bool SearchVertically(void)
{
    for (int x = 0; x < BOARD_SIZE; x++)
    {
        for (int y = 0; y < BOARD_SIZE - 2; y++)
        {
            char t = board[y][x];
            if (t == board[y + 1][x] && t == board[y + 2][x])
            {
                matched[y][x] = matched[y + 1][x] = matched[y + 2][x] = true;
                score += 10;
               
                // playSound()
                scoreAnimating = true;
                scoreScale = 2.0f;
                scoreScaleVelocity = -2.5f;
                addScorePopup(x,y,10);
                return true;
            }
        }
    }
    return false;
}

bool findMatches(void)
{
    for (int y = 0; y < BOARD_SIZE; y++)
    {
        for (int x = 0; x < BOARD_SIZE; x++)
        {
            matched[y][x] = false;
        }
    }
    return SearchHorizontally() || SearchVertically();
}

void resolveMatches(void)
{
    for (int x = 0; x < BOARD_SIZE; x++)
    {
        int writeY = BOARD_SIZE - 1;
        for (int y = BOARD_SIZE - 1; y >= 0; y--)
        {
            if (!matched[y][x])
            {
                if (y != writeY)
                {
                    board[writeY][x] = board[y][x];
                    fallOffsets[writeY][x] = (writeY - y) * TILE_SIZE;
                    board[y][x] = ' ';
                }
                writeY--;
            }
        }
        // FILL EMPTY SPOTS with RANDOM
        while (writeY >= 0)
        {
            board[writeY][x] = randomTile();
            fallOffsets[writeY][x] = (writeY + 1) * TILE_SIZE;
            writeY--;
        }
    }
    TileState = STATE_ANIMATING;
}

void initBoard(void)
{
    for (int y = 0; y < BOARD_SIZE; y++)
    {
        for (int x = 0; x < BOARD_SIZE; x++)
        {
            board[y][x] = randomTile();
        }
    }
    // Center screen
    int grid_width = BOARD_SIZE * TILE_SIZE;
    int grid_height = BOARD_SIZE * TILE_SIZE;
    grid_origin = (Vector2){(GetScreenWidth() - grid_width) / 2, (GetScreenHeight() - grid_height) / 2};
    if (findMatches())
    {
        resolveMatches();
    }
    else
    {
        TileState = STATE_IDLE;
    }
}

void swapTiles(Vector2 A, Vector2 B)
{
    char temp = board[(int)A.y][(int)A.x];
    board[(int)A.y][(int)A.x] = board[(int)B.y][(int)B.x];
    board[(int)B.y][(int)B.x] = temp;
}

bool areTilesAdjacent(Vector2 A, Vector2 B)
{
    return (abs((int)A.x - (int)B.x) + abs((int)A.y - (int)B.y)) == 1;
}

void UpdateScorePopup(void)
{
    for (int i = 0; i < MAX_SCORE_POPUPS; i++)
    {
        if (scorePopups[i].active)
        {
            scorePopups[i].lifetime -= GetFrameTime();
            scorePopups[i].position.y -= 30 * GetFrameTime();
            scorePopups[i].alpha = scorePopups[i].lifetime;

            if (scorePopups[i].lifetime <= 0.0f)
            {
                scorePopups[i].active = -false;
            }
        }
    }
}

void UpdateScorePopupAnimation(void)
{
    if (scoreAnimating)
    {
        scoreScale += scoreScaleVelocity * GetFrameTime();
        if (scoreScale <= 1.0f)
        {
            scoreScale = 1.0f;
            scoreAnimating = false;
        }
    }
}

int main(void)
{
    const int screen_width = 800;
    const int screen_height = 450;
    Vector2 mouse = {0, 0};

    Vector2 selected_tile = {-1, -1};
    InitWindow(screen_width, screen_height, "Match 3 Raylib");
    SetTargetFPS(60);
    srand(time(NULL));
    InitAudioDevice();

    backgroundText = LoadTexture("Assets/bkg.png");
    // bgm = LoadMusicStream("Assets/bgm/blabla.mp3");
    // Sfx = LoadSound("Assets/Sfx/Match.mp3");
    // PlayMusicStream(bgm);
    // SetMusicVolume(bgm,0.6f);
    // SetSoundVolume(sfx,0.4f);

    initBoard();

    /////////////////////////// LOOP ////////////////////////////////////
    while (!WindowShouldClose())
    {
        // UpdateMusicStream(bgm);
        // update mouse pos
        mouse = GetMousePosition();

        /// Gameplay
        if (TileState == STATE_IDLE && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            int x = (mouse.x - grid_origin.x) / TILE_SIZE;
            int y = (mouse.y - grid_origin.y) / TILE_SIZE;
            if (x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE)
            {
                Vector2 currentTile = (Vector2){x,y};
                if(selected_tile.x < 0)
                {
                    selected_tile = currentTile;
                }
                else
                {
                    if(areTilesAdjacent(selected_tile,currentTile))
                    {
                        swapTiles(selected_tile,currentTile);
                        if(findMatches())
                        {
                            resolveMatches();
                        }
                        else
                        {
                            swapTiles(selected_tile,currentTile);
                        }
                    }
                    selected_tile = (Vector2){-1,-1};
                }
            }
        }

        // ANIMATION
        if(TileState == STATE_ANIMATING)
        {
            bool stillAnim = false;
            for (int y = 0; y < BOARD_SIZE; y++)
            {
                for (int x = 0; x < BOARD_SIZE; x++)
                {
                    if (fallOffsets[y][x] > 0)
                    {
                        fallOffsets[y][x] -= fallSpeed;
                        if (fallOffsets[y][x] < 0)
                        {
                            fallOffsets[y][x] = 0;
                        }
                        else
                        {
                            stillAnim = true;
                        }
                    }
                }
            }
            if(!stillAnim)
            {
                TileState = STATE_MATCH_DELAY;
                matchDelayTimer = MATCH_DELAY_DURATION;
            }
        }
        if (TileState == STATE_MATCH_DELAY)
        {
            matchDelayTimer -= GetFrameTime();
            if (matchDelayTimer >= 0.0f)
            {
                if (findMatches())
                {
                    resolveMatches();
                }
                else
                {
                    TileState = STATE_IDLE;
                }
            }
        }

        // Update score popup
        UpdateScorePopup();
        // update score animation
        UpdateScorePopupAnimation();

        ///////////////////// DRAW ///////////////////////////////////////
        BeginDrawing();
        ClearBackground(BLACK);

        // Background
        DrawTexturePro(backgroundText,
                       (Rectangle){0, 0, backgroundText.width, backgroundText.height},
                       (Rectangle){0, 0, GetScreenWidth(), GetScreenHeight()},
                       (Vector2){0, 0},
                       0.f,
                       (Color){255, 100, 100, 50});

        DrawRectangle(grid_origin.x,grid_origin.y,BOARD_SIZE*TILE_SIZE,BOARD_SIZE*TILE_SIZE,Fade(DARKGRAY,0.6f));

        for (int y = 0; y < BOARD_SIZE; y++)
        {
            for (int x = 0; x < BOARD_SIZE; x++)
            {
                Rectangle rect = {grid_origin.x + x * TILE_SIZE, grid_origin.y + y * TILE_SIZE, TILE_SIZE, TILE_SIZE};

                DrawRectangleLinesEx(rect, 1, DARKGRAY);

                if (board[y][x] != ' ')
                {
                    DrawTextEx(GetFontDefault(),
                               TextFormat("%c", board[y][x]),
                               (Vector2){rect.x + 12, rect.y + 8 - fallOffsets[y][x]},
                               20,
                               1,
                               matched[y][x] ? GREEN : WHITE);
                }
            }
        }

        // DRAW SELECTED TILE
        if (selected_tile.x >= 0)
        {
            DrawRectangleLinesEx((Rectangle){grid_origin.x + (selected_tile.x * TILE_SIZE),
                                             grid_origin.y + (selected_tile.y * TILE_SIZE),
                                             TILE_SIZE,
                                             TILE_SIZE},
                                 2,
                                 YELLOW);
        }
        DrawText(TextFormat("Score: %d", score), 20, 20, 20, YELLOW);

        // Draw score popups
        for(int i = 0;i < MAX_SCORE_POPUPS;i++)
        {
            if(scorePopups[i].active)
            {
                Color col = Fade(YELLOW,scorePopups[i].alpha);
                DrawText(TextFormat("+%d",scorePopups[i].amount),
                                    scorePopups[i].position.x,
                                    scorePopups[i].position.y,
                                    20,col);
            }
        }

        EndDrawing();
    }
    UnloadTexture(backgroundText);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
