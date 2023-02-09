#define STB_IMAGE_IMPLEMENTATION
#include "w:\libs\stb_image.h"

#define SPRITE_ATLAS_SIZE 1024
#define BACKBUFFER_WIDTH 800
#define BACKBUFFER_HEIGHT 600
#define WINDOW_TITLE "Nymira"
#define CLEAR_COLOR rgba(.9f,.9f,.9f,1)

#include "C:\Users\Kaeshi\Desktop\c\Runeforma\engine2d.h"
#include "C:\Users\Kaeshi\Desktop\c\Runeforma\render.h"

#include "data.h"

global Editor editor;
global Board board1;

#include "editor.h"
#include "stone_queue.h"
#include "board.h"
#include "move.h"
#include "input.h"

typedef struct StoneListNode
{
    Stone data;
    struct StoneListNode *next;
    
} StoneListNode;

typedef struct
{
    StoneListNode *start;
    
} StoneList;

function float
safe_divide(float a, float b)
{
    assert(b != 0);
    float result = 0;
    
    if (b != 0)
    {
        result = a / b;
    }
    
    return result;
}

function float
map_range_to_range(float a1, float a2, float s,
                   float b1, float b2)
{
    float t = (b1+(s-a1)*safe_divide((b2-b1),(a2-a1)));
    return t;
}

function float
point_to_thumb_y(float barH, float thumbH, float point) // [0,1]
{
    return map_range_to_range(0,1,point,
                              barH-thumbH,0);
}

function float
thumb_y_to_point(float barH, float thumbH, float thumbY) // [0,1]
{
    return map_range_to_range(barH-thumbH,0,thumbY,
                              0,1);
}

function float
point_to_content_y(float contentH, float barH, float point) // [0,1]
{
    return map_range_to_range(0,1,point,
                              barH, contentH);
}

function bool
stone_list_push(StoneList *list, Stone data)
{
    // TODO: Use a pool of available nodes
    StoneListNode *node = alloc_type(StoneListNode);
    node->data = data;
    
    node->next = list->start;
    list->start = node;
    return true;
}

function void init()
{
    // Init the editor renderer
    editor_render_init();
    
    // Init a default board
    editor.currentBoard = &board1;
    Board *board = editor.currentBoard;
    board->lineThickness = 1;
    board->lineCount = 9;
    board->lineSpace = 50;
    
    StoneQueue test = stone_queue_new(10);
    stone_queue_push(&test, make_stone(0, 0, 1));
    Stone a;
    stone_queue_pop(&test, &a);
    stone_queue_free(&test);
    
    move_place(&board->tree, 0, 0, 1);
    move_place(&board->tree, 1, 0, 2);
    move_place(&board->tree, 2, 0, 1);
    move_place(&board->tree, 3, 0, 2);
    move_place(&board->tree, 4, 0, 1);
    move_place(&board->tree, 5, 0, 2);
    move_place(&board->tree, 6, 0, 1);
    move_place(&board->tree, 7, 0, 2);
    move_place(&board->tree, 8, 0, 1);
    
    move_place(&board->tree, 0, 1, 1);
    move_place(&board->tree, 1, 1, 2);
    move_place(&board->tree, 2, 1, 1);
    move_place(&board->tree, 3, 1, 2);
    move_place(&board->tree, 4, 1, 1);
    move_place(&board->tree, 5, 1, 2);
    move_place(&board->tree, 6, 1, 1);
    move_place(&board->tree, 7, 1, 2);
    move_place(&board->tree, 8, 1, 1);
    
    move_place(&board->tree, 0, 2, 1);
    move_place(&board->tree, 1, 2, 2);
    move_place(&board->tree, 2, 2, 1);
    move_place(&board->tree, 3, 2, 2);
    move_place(&board->tree, 4, 2, 1);
    move_place(&board->tree, 5, 2, 2);
    move_place(&board->tree, 6, 2, 1);
    move_place(&board->tree, 7, 2, 2);
    move_place(&board->tree, 8, 2, 1);
}

function void update()
{
    // Get the currently selected board
    Board *board = editor.currentBoard;
    
    // Handle the user input
    input();
    
    float treeOriginY = 0;
    
    float barX = 0;
    float barY = engine.backBufferSize.y;
    float barW = 20;
    float barH = engine.backBufferSize.y;
    
    editor.point += 0.0167f * editor.pointVel;
    
    editor.pointVel *= 0.8f;
    
    if (board->tree.root)
    {
        
        // int treeDepth = 2;
        int treeDepth = move_tree_height(board->tree.root, 0);
        float treeH = treeDepth * 50.0f + 40;
        
        if (treeH > engine.backBufferSize.y)
        {
            float thumbW = barW;
            float thumbH = (barH * (barH / treeH));
            
            float thumbX = barX;
            // float thumbY = barY - thumbHeight;
            
            
            float thumbY = point_to_thumb_y(barH, thumbH, editor.point);
            
            draw_rect(editor.layer1, editor.white, 
                      v2(thumbX, thumbY),
                      v2(20, thumbH), rgba(1,0,0,1), 1);
            
            
            float scrollAcc = 2.5f;
            if (engine.mouse.wheel > 0)
            {
                if (editor.point < 1.0f)
                {
                    editor.pointVel += scrollAcc * engine.mouse.wheel;
                }
            }
            else if (engine.mouse.wheel < 0)
            {
                if (editor.point > 0.0f)
                {
                    editor.pointVel += scrollAcc * engine.mouse.wheel;
                }
            }
        }
        
        treeOriginY = point_to_content_y(treeH, barH, editor.point);
    }
    
    if (editor.point < 0)
    {
        editor.point = 0;
        editor.pointVel = 0;
    }
    
    if (editor.point > 1)
    {
        editor.point = 1;
        editor.pointVel = 0;
    }
    
    
    // Draw tree diagram
    move_tree_draw_diagram(&board->tree, board->tree.root, 
                           v2(40, treeOriginY));
    
    // Draw the board
    board_draw(board);
    
    Vector2 stoneSize = board_get_stone_size(board);
    Vector2 boardSize = board_size(board);
    Vector2 origin = board_origin(board, boardSize);
    
    for (s32 j = 0; j < editor.libGroupIndex; j++)
    {
        LibertyGroup *group = editor.libGroups + j;
        if (group->count > 0)
        {
            for (s32 i = 0; i < group->count; i++)
            {
                Stone liberty = group->array[i];
                
                Vector2 libSize = v2_mul(.3f, stoneSize);
                
                Vector2 pos = board_stone_pos(board, liberty.x, liberty.y);
                draw_rect(editor.layer2, editor.white,
                          v2_sub(v2_add(origin, pos), v2_mul(.5f, libSize)),
                          libSize, rgba(1,0,0,0.5f), 0);
            }
        }
    }
}