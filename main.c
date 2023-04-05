#define STB_IMAGE_IMPLEMENTATION
#include "w:\libs\stb_image.h"

#define SPRITE_ATLAS_SIZE 1024
#define BACKBUFFER_WIDTH 800
#define BACKBUFFER_HEIGHT 600
#define WINDOW_TITLE L"Nymira"
#define CLEAR_COLOR rgba(.9f,.9f,.9f,1)

#include "w:\libs\xlibs\xd3d11.h"
#include "w:\libs\xlibs\xrender2d.h"

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

f32 map_range_to_range(f32 range1_min, f32 range1_max,
                       f32 point,
                       f32 range2_min, f32 range2_max)
{
    f32 ratio = (point - range1_min) / (range1_max - range1_min);
    f32 mapped_value = ratio * (range2_max - range2_min) + range2_min;
    return mapped_value;
}

function float
point_to_thumb_y(float barH, float thumbH, float point) // [0,1]
{
    return map_range_to_range(0,1,            // range 1
                              point,          // value
                              barH-thumbH,0); // range 2
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
    StoneListNode *node = xalloc(sizeof *node);
    node->data = data;
    
    node->next = list->start;
    list->start = node;
    return true;
}

function void initialize()
{
    // Init the editor renderer
    editor_render_init();
    
    // Init a default board
    editor.currentBoard = &board1;
    Board *board = editor.currentBoard;
    board->lineThickness = 1;
    board->lineCount = 9;
    board->lineSpace = 50;
    
    /*
    // audio
     editor.stoneHit0 = load_wav(L"sounds/stone0.wav", L"Stone Hit 0");
    
     dsbdesc.lpwfxFormat = &waveFormat;
    
    // Play the wave file
     IDirectSoundBuffer_Play(editor.audioBuffer, 0, 0, DSBPLAY_LOOPING);
    
    // Wait until the sound is done playing
    DWORD status;
    while (IDirectSoundBuffer_GetStatus(editor.audioBuffer, &status) & DSBSTATUS_PLAYING)
    {
        Sleep(100);
    }
    
    // Clean up
    IDirectSoundBuffer_Release(editor.audioBuffer);
    IDirectSound8_Release(editor.dsound);
    //
    */
    
    StoneQueue test = stone_queue_new(10);
    stone_queue_push(&test, make_stone(0, 0, 1));
    Stone a;
    stone_queue_pop(&test, &a);
    stone_queue_free(&test);
    
    editor.point = 1;
    
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

XWINMAIN()
{
    initialize();
    
    while (xd11.running)
    {
        xrender2d_pre_update();
        
        // Get the currently selected board
        Board *board = editor.currentBoard;
        
        // Handle the user input
        input();
        
        float treeOriginY = 0;
        
        float barX = 0;
        float barY = xd11.back_buffer_size.y;
        float barW = 20;
        float barH = xd11.back_buffer_size.y;
        
        editor.point += 0.0167f * editor.pointVel;
        
        editor.pointVel *= 0.8f;
        
        if (board->tree.root)
        {
            
            // int treeDepth = 2;
            int treeDepth = move_tree_height(board->tree.root, 0);
            float treeH = (treeDepth * .5f*editor.stoneBlack.size.y +
                           editor.stoneBlack.size.y);
            
            if (treeH > xd11.back_buffer_size.y)
            {
                float thumbW = barW;
                float thumbH = (barH * (barH / treeH));
                
                float thumbX = barX;
                // float thumbY = barY - thumbHeight;
                
                
                float thumbY = point_to_thumb_y(barH, thumbH, editor.point);
                
                draw_rect(&editor.layer1, 
                          (v2f){thumbX, thumbY},
                          (v2f){20, thumbH},
                          (v4f){0.2f,0.2f,0.2f,0.8f});
                
                float scrollAcc = 2.5f;
                
                if ((xwin.mouse.wheel < 0 && editor.point > 0) || 
                    (xwin.mouse.wheel > 0 && editor.point < 1))
                {
                    editor.pointVel += scrollAcc * xwin.mouse.wheel;
                }
            }
            
            treeOriginY = point_to_content_y(treeH, barH, editor.point);
            
            editor.contentH = treeH;
        }
        
        editor.contentY = treeOriginY;
        
        
        if (editor.point < 0)
        {
            editor.pointVel += -2*editor.point;
            editor.pointVel *= 0.4f;
        }
        
        if (editor.point > 1)
        {
            editor.pointVel += -0.2f*powf(editor.point, 20);
            editor.pointVel *= 0.4f;
        }
        
        draw_text(&editor.layer1, (v2f){200,xd11.back_buffer_size.y - 50}, 
                  (v4f){0,0,0,1}, &editor.font32,
                  L"Nymira Go/Baduk/Weiqi editor");
        
        
        // Draw tree diagram
        move_tree_draw_diagram(&board->tree, board->tree.root, 
                               (v2f){40, treeOriginY});
        
        // Draw the board
        board_draw(board);
        
        v2f stoneSize = board_get_stone_size(board);
        v2f boardSize = board_size(board);
        v2f origin = board_origin(board, boardSize);
        
        for (s32 j = 0; j < editor.libGroupIndex; j++)
        {
            LibertyGroup *group = editor.libGroups + j;
            if (group->count > 0)
            {
                for (s32 i = 0; i < group->count; i++)
                {
                    Stone liberty = group->array[i];
                    
                    v2f libSize = mul2f(.3f, stoneSize);
                    
                    v2f pos = board_stone_pos(board, liberty.x, liberty.y);
                    draw_rect(&editor.layer2, sub2f(add2f(origin, pos), mul2f(.5f, libSize)),
                              libSize, (v4f){1,0,0,0.5f});
                }
            }
        }
        
        XRenderBatch batches[2] = {editor.layer1, editor.layer2};
        xrender2d_post_update(batches, narray(batches));
        
        /* Reset Batches */
        xrender2d_reset_batch(&editor.layer1);
        xrender2d_reset_batch(&editor.layer2);
    }
}

LRESULT window_proc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;
    switch (message) {
        XWNDPROC;
        
        default:
        {
            result = DefWindowProcW(window, message, wParam, lParam);
        } break;
    }
    return result;
}