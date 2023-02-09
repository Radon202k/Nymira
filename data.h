#pragma once

/*

The Capture structure is a linked list that holds information about 
a captured stone in the game, including its x and y coordinates.

The Stone structure holds information about a single stone in the game, 
including its x and y coordinates and the player that placed it.

The Move structure is a representation of a move made in the game, 
including the x and y coordinates of the placed stone, the player that 
made the move, and a linked list of captured stones (represented by the 
Capture structure).

The MoveNode structure is a node in a tree data structure representing 
all possible moves in the game. Each MoveNode has information about a 
single move (represented by the Move structure), as well as pointers to 
its parent, left sibling, right sibling, and first child in the tree.

The MoveTree structure holds information about the tree data structure 
representing all possible moves in the game. It contains a pointer to the 
current move being considered (point) and a pointer to the root of the 
tree (root).

*/

typedef struct Capture
{
    s32 x;
    s32 y;
    struct Capture *nextChainEntry;
} Capture;

typedef struct
{
    s32 x;
    s32 y;
    s32 player;
} Stone;

typedef struct
{
    int rear;
    int front;
    s32 storageSize;
    Stone *storage;
} StoneQueue;

inline Stone
make_stone(int x, int y, int player)
{
    Stone stone =
    {
        x, y, player
    };
    
    return stone;
}

typedef struct
{
    s32 x;
    s32 y;
    s32 player;
    struct Capture *firstCapture;
} Move;

typedef struct MoveNode
{
    Move data;
    
    struct MoveNode *parent;
    struct MoveNode *leftSibling;
    struct MoveNode *rightSibling;
    struct MoveNode *firstChild;
    
} MoveNode;

typedef struct
{
    MoveNode *point;
    MoveNode *root;
} MoveTree;

typedef struct
{
    float lineThickness;
    float lineSpace;
    s32 lineCount;
    s32 currentPlayer;
    MoveTree tree;
} Board;

typedef struct
{
    Stone array[19*19];
    s32 count;
    
} LibertyGroup;

typedef struct
{
    SpriteGroup *layer1;
    SpriteGroup *layer2;
    
    Sprite font;
    Sprite white;
    Sprite naruto;
    Sprite stoneBlack;
    Sprite stoneWhite;
    
    Board *currentBoard;
    
    bool closestPosActive;
    Vector2i closestBoardPoint;
    
    LibertyGroup libGroups[4];
    s32 libGroupIndex;
    
    float point;
    float pointVel;
    
} Editor;

#define alloc_size(size) _safe_alloc(size)
#define alloc_type(type) (type *)_safe_alloc(sizeof(type))
#define alloc_array(count, type) (type *)_safe_alloc(count*sizeof(type))

function void *_safe_alloc(u32 size)
{
    // Allocate it
    void *result = malloc(size);
    
    // Clear it to zero
    memset(result, 0, size);
    
    // Return the pointer
    return result;
}
