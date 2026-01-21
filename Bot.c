#include "Bot.h"

#include "map.h"

struct Bot* CreateBot()
{
    struct Bot* bot = (struct Bot*)malloc(sizeof(struct Bot));
    
    bot->position = (sfVector2i){0, 0};
    
    bot->sprite = sfSprite_create();
    sfTexture* tex = sfTexture_createFromFile("./Assets/Characters/Bot01.png", NULL);
    sfSprite_setTexture(bot->sprite, tex, sfTrue);
    sfSprite_setPosition(bot->sprite, (sfVector2f){0, 0});
    float scale = ((float)CELL_SIZE / 24.f) * 0.75f;
    sfSprite_setScale(bot->sprite, (sfVector2f){scale, scale});
    
    return bot;
}

void SpawnBotAtStartCell(struct Bot* bot, Grid* grid)
{
    for (int i = 0; i < GRID_ROWS; i++)
    {
        for (int j = 0; j < GRID_COLS; j++)
        {
            if (grid->cell[i][j]->type == START)
            {
                bot->position = grid->cell[i][j]->coord;
                sfVector2f startCelPosition = sfSprite_getPosition(grid->cell[i][j]->sprite);
                startCelPosition.x += 5.f;
                startCelPosition.y += 5.f;
                sfSprite_setPosition(bot->sprite, startCelPosition);
                break;
            }
        }
    }
    
}

void DestroyBot(struct Bot* bot)
{
    if (!bot->sprite) return;
    sfSprite_destroy(bot->sprite);
    free(bot);
}

void DrawBot(sfRenderWindow* window, struct Bot* bot)
{
    if (!window || !bot || !bot->sprite) return;
    sfRenderWindow_drawSprite(window, bot->sprite, NULL);
}

int MoveBot(struct Bot* bot, Grid* grid, enum MovementType type, enum Direction direction)
{
    int distance = 1;
    if (type == JUMP) distance = 2;
    
    sfVector2i newPosition = bot->position;
    
    switch (direction)
    {
    case NORTH:
        if (newPosition.y > 0)
        {
            newPosition.y -= distance;
        }
        break;
    case EAST:
        if (newPosition.x < (GRID_COLS - 1))
        {
            newPosition.x += distance;
        }
        break;
    case SOUTH:
        if (newPosition.y < (GRID_ROWS - 1))
        {
            newPosition.y += distance;
        }
        break;
    case WEST:
        if (newPosition.x > 0)
        {
            newPosition.x -= distance;
        }
        break;
    default:
        break;
    }

    enum CellType destinationCellType = grid->cell[newPosition.y][newPosition.x]->type;

    if (destinationCellType != OBSTACLE)
    {
        bot->position = newPosition;
        sfVector2f newSpritePosition = sfSprite_getPosition(grid->cell[bot->position.y][bot->position.x]->sprite);
        newSpritePosition.x += 5.f;
        newSpritePosition.y += 5.f;
        sfSprite_setPosition(bot->sprite, newSpritePosition);
    } else
    {
        printf("can't go there ! \n");
    }

    switch (destinationCellType)
    {
    case END:
        return REACH_END;
    case EMPTY:
        return FAILURE;
    case START:
    case WALKABLE:
    case OBSTACLE:
    default:
        return NOTHING;
    }
}

void AddMovement(struct Bot* bot, enum MovementType type, enum Direction direction)
{
    if (!bot) return;
    // Add a new element Move to bot's MoveQueue
    int currentLength = 0;
    while (bot->MoveQueue[currentLength].type == MOVE_TO || bot->MoveQueue[currentLength].type == JUMP)
    {
        currentLength++;
    }
    bot->MoveQueue[currentLength].type = type;
    bot->MoveQueue[currentLength].direction = direction;
    bot->MoveQueue[currentLength + 1].type = INVALID;
}


void MoveBot_AI(struct GameData* data)
{
    if (!data || !data->bot || !data->grid) return;

    bool visited[GRID_ROWS][GRID_COLS] = { false };
    sfVector2i history[GRID_ROWS * GRID_COLS];
    int historyIndex = 0;

    visited[data->bot->position.y][data->bot->position.x] = true;
    history[historyIndex] = data->bot->position;

    while (data->pathResult == NOTHING)
    {
        int x = data->bot->position.x;
        int y = data->bot->position.y;
        bool moveFound = false;
        enum Direction chosenDir;
        enum MovementType chosenType = MOVE_TO;

        // --- PHASE 1 : RECHERCHE DE MOUVEMENT ---
        // (On teste Nord, Est, Sud, Ouest comme avant...)
        if (y > 0 && !moveFound && !visited[y - 1][x] && (data->grid->cell[y - 1][x]->type == WALKABLE || data->grid->cell[y - 1][x]->type == END)) {
            chosenDir = NORTH; chosenType = MOVE_TO; moveFound = true;
        }
        else if (x + 1 < GRID_COLS && !moveFound && !visited[y][x + 1] && (data->grid->cell[y][x + 1]->type == WALKABLE || data->grid->cell[y][x + 1]->type == END)) {
            chosenDir = EAST; chosenType = MOVE_TO; moveFound = true;
        }
        else if (y + 1 < GRID_ROWS && !moveFound && !visited[y + 1][x] && (data->grid->cell[y + 1][x]->type == WALKABLE || data->grid->cell[y + 1][x]->type == END)) {
            chosenDir = SOUTH; chosenType = MOVE_TO; moveFound = true;
        }
        else if (x > 0 && !moveFound && !visited[y][x - 1] && (data->grid->cell[y][x - 1]->type == WALKABLE || data->grid->cell[y][x - 1]->type == END)) {
            chosenDir = WEST; chosenType = MOVE_TO; moveFound = true;
        }

        // --- TEST DES SAUTS ---
        if (!moveFound) {
            if (y > 1 && !visited[y - 2][x] && data->grid->cell[y - 1][x]->type == OBSTACLE && (data->grid->cell[y - 2][x]->type == WALKABLE || data->grid->cell[y - 2][x]->type == END)) {
                chosenDir = NORTH; chosenType = JUMP; moveFound = true;
            }
            else if (x + 2 < GRID_COLS && !visited[y][x + 2] && data->grid->cell[y][x + 1]->type == OBSTACLE && (data->grid->cell[y][x + 2]->type == WALKABLE || data->grid->cell[y][x + 2]->type == END)) {
                chosenDir = EAST; chosenType = JUMP; moveFound = true;
            }
            else if (y + 2 < GRID_ROWS && !visited[y + 2][x] && data->grid->cell[y + 1][x]->type == OBSTACLE && (data->grid->cell[y + 2][x]->type == WALKABLE || data->grid->cell[y + 2][x]->type == END)) {
                chosenDir = SOUTH; chosenType = JUMP; moveFound = true;
            }
            else if (x > 1 && !visited[y][x - 2] && data->grid->cell[y][x - 1]->type == OBSTACLE && (data->grid->cell[y][x - 2]->type == WALKABLE || data->grid->cell[y][x - 2]->type == END)) {
                chosenDir = WEST; chosenType = JUMP; moveFound = true;
            }
        }

        // --- PHASE 2 : EXÉCUTION VISUELLE ---
        if (moveFound) {
            // Mouvement classique vers l'avant
            AddMovement(data->bot, chosenType, chosenDir);
            data->pathResult = MoveBot(data->bot, data->grid, chosenType, chosenDir);

            visited[data->bot->position.y][data->bot->position.x] = true;
            historyIndex++;
            history[historyIndex] = data->bot->position;
        }
        else if (historyIndex > 0) {
            // BACKTRACKING : On prépare le mouvement de retour
            historyIndex--;
            sfVector2i target = history[historyIndex];

            int distY = abs(target.y - data->bot->position.y);
            int distX = abs(target.x - data->bot->position.x);
            enum MovementType backType = (distY == 2 || distX == 2) ? JUMP : MOVE_TO;

            enum Direction backDir;
            if (target.y < data->bot->position.y) backDir = NORTH;
            else if (target.y > data->bot->position.y) backDir = SOUTH;
            else if (target.x < data->bot->position.x) backDir = WEST;
            else backDir = EAST;

            // EXÉCUTION DU RETOUR : On utilise MoveBot pour mettre à jour le sprite
            printf("Backtracking vers [%d, %d]\n", target.x, target.y);
            AddMovement(data->bot, backType, backDir);
            data->pathResult = MoveBot(data->bot, data->grid, backType, backDir);
        }
        else {
            break;
        }

        // --- L'ÉTAPE CRUCIALE POUR ÉVITER LA TÉLÉPORTATION ---
        // 1. On attend un peu pour que l'oeil humain voit le mouvement
        sfSleep(sfMilliseconds(400));

        
    }
}


bool SearchPath_AI(struct Bot* bot, Grid* grid)
{
    // Implement pathfinding algorithm to fill bot's MoveQueue
    return false;
}

