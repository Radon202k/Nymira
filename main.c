#define STB_IMAGE_IMPLEMENTATION
#include "w:\libs\stb_image.h"

#define SPRITE_ATLAS_SIZE 1024
#define BACKBUFFER_WIDTH 800
#define BACKBUFFER_HEIGHT 600
#define WINDOW_TITLE "Nymira"
#define CLEAR_COLOR rgba(.9f,.9f,.9f,1)

#include "engine2d.h"
#include "render.h"

#include "data.h"

global Editor editor;
global Board board1;

#include "editor.h"
#include "board.h"
#include "move.h"
#include "input.h"

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
}

function void update()
{
    // Get the currently selected board
    Board *board = editor.currentBoard;
    
    // Handle the user input
    input();
    
    // Draw tree diagram
    move_tree_draw_diagram(&board->tree, board->tree.root, v2(0,200));
    
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