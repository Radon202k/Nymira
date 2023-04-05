#pragma once

function v2f
board_get_stone_size(Board *board)
{
    return (v2f){board->lineSpace + 10, board->lineSpace + 10};
}

function v2f
board_stone_pos(Board *board, s32 x, s32 y)
{
    assert(x >= 0 && x < board->lineCount);
    assert(y >= 0 && y < board->lineCount);
    
    float posX = x*board->lineThickness + x*board->lineSpace;
    float posY = y*board->lineThickness + y*board->lineSpace;
    return (v2f){posX, posY};
}

function v2f
board_origin(Board *board, v2f boardSize)
{
    float left = 0.5f*xd11.back_buffer_size.x - 0.5f*boardSize.x;
    float bottom = 0.5f*xd11.back_buffer_size.y - 0.5f*boardSize.y;
    return (v2f){left, bottom};
}

function v2f
board_size(Board *board)
{
    v2f result = 
    {
        (board->lineCount-1)*board->lineSpace + board->lineCount*board->lineThickness,
        (board->lineCount-1)*board->lineSpace + board->lineCount*board->lineThickness,
    };
    return result;
}

function bool
board_closest_pos(v2f origin, s32 *x, s32 *y)
{
    // Search to see if mouse is close to a valid board position
    float minLength = 1000;
    v2f closesPos = {0,0};
    u32 boardX = 0;
    u32 boardY = 0;
    
    Board *board = editor.currentBoard;
    
    // For each Y line
    for (s32 j = 0; j < board->lineCount; j++)
    {
        // For each X line
        for (s32 i = 0; i < board->lineCount; i++)
        {
            // Get absolute position for cell
            v2f absPos = add2f(origin, board_stone_pos(board, i, j));
            
            // Get difference from mouse to it
            v2f diff = sub2f(xwin.mouse.pos, absPos);
            
            // Get length of that difference
            float length = len2f(diff);
            
            // If it is smaller than the current min
            if (length < minLength)
            {
                // Assign as the min
                minLength = length;
                
                // Assign as the current pos
                closesPos = absPos;
                
                // Assign X,Y
                boardX = i;
                boardY = j;
            }
        }
    }
    
    if (minLength < 100)
    {
        *x = boardX;
        *y = boardY;
        return true;
    }
    return false;
}

function void
board_get_stones_from_move_list(Board *board, 
                                Stone *destStones,
                                Move *moves, s32 moveCount)
{
    // For each move in the path from the root to point
    for (s32 i = moveCount-1; i >= 0; i--)
    {
        // Get the move from the pathToPoint array
        Move move = moves[i];
        
        // Get the pointer to the stone we will be writing into the stones array
        Stone *stone = destStones + move.y*board->lineCount + move.x;
        
        // Set the stone data
        stone->x = move.x;
        stone->y = move.y;
        stone->player = move.player;
        
        // Remove captures from the stones
        Capture *capture = move.firstCapture;
        while (capture)
        {
            // Remove stone
            Stone *removed = destStones + capture->y*board->lineCount + capture->x;
            removed->player = 0;
            
            capture = capture->nextChainEntry;
        }
    }
}

function void 
board_draw_lines(Board *board)
{
    v2f boardSize = board_size(board);
    v2f origin = board_origin(board, boardSize);
    
    // Draw board's vertical lines
    for (s32 i = 0; i < board->lineCount; i++)
    {
        v2f pos = board_stone_pos(board, i, 0);
        draw_rect(&editor.layer1, 
                  (v2f){origin.x + pos.x, origin.y + pos.y},
                  (v2f){1,boardSize.y},
                  (v4f){0,0,0,1});
    }
    
    // Draw board's horizontal lines
    for (s32 j = 0; j < board->lineCount; j++)
    {
        v2f pos = board_stone_pos(board, 0, j);
        draw_rect(&editor.layer1, 
                  (v2f){origin.x + pos.x, origin.y + pos.y},
                  (v2f){boardSize.x,1},
                  (v4f){0,0,0,1});
    }
}

// Returns an array of Move objects from the root node until the
// point node. This allows us to traverse the tree forward to the point
// without having to visit all the nodes of the tree.
// dest is an Array that will hold board->lineCount^2 elements
// to hold the maximum number of possible moves (the full go board)
// Returns the size of the dest array
// ###!!!! dest MUST BE ALLOCATED by the caller !!!!###
function u32
move_get_nodes_in_path(Board *board, MoveNode *point,
                       Move *dest)
{
    // Start at the point
    MoveNode *node = board->tree.point;
    
    // Variable to increment as we insert into the dest array
    u32 destIndex = 0; 
    
    // While the node is not the root
    while (node != board->tree.root)
    {
        // Insert the node into the array and advance the index
        dest[destIndex++] = node->data;
        
        // Go to the parent node
        node = node->parent;
    }
    
    if (node)
    {
        assert(node == board->tree.root);
        // Include the root as well
        dest[destIndex++] = node->data;
    }
    
    // Returns the size of the dest array
    return destIndex;
}

function void
board_get_stones(Board *board, Stone *destStones)
{
    u32 maxStoneCount = board->lineCount*board->lineCount;
    
    // Allocate array of Moves and clear it to zero
    Move *pathToPoint = xnalloc(maxStoneCount, Move);
    
    // Initialize the moves with their X,Y positions
    for (s32 x = 0; x < board->lineCount; x++)
    {
        for (s32 y = 0; y < board->lineCount; y++)
        {
            s32 index = y * board->lineCount + x;
            Stone *stone = destStones + index;
            stone->x = x;
            stone->y = y;
        }
    }
    
    // Get the MoveNodes from the point to the root
    // In other words, gets the path of the point node
    u32 pathToPointCount = move_get_nodes_in_path(board, board->tree.point,
                                                  pathToPoint);
    
    // Get the stones that should be on the board based on the sequence of moves
    board_get_stones_from_move_list(board, 
                                    destStones,
                                    pathToPoint, pathToPointCount);
    
    // Free path to point array
    xfree(pathToPoint);
}

function void
move_capture(Board *board, Stone stone)
{
    Capture *capture = xalloc(sizeof *capture);
    capture->x = stone.x;
    capture->y = stone.y;
    capture->nextChainEntry = board->tree.point->data.firstCapture;
    board->tree.point->data.firstCapture = capture;
}

function Stone *
board_get_stone(Board *board, Stone *stones, s32 x, s32 y)
{
    Stone *result = 0;
    
    if ((x >= 0 && y >= 0) && (x < board->lineCount && y < board->lineCount))
    {
        result = stones + y*board->lineCount + x;
    }
    
    return result;
}

function void
board_draw_stones(Board *board)
{
    v2f boardSize = board_size(board);
    v2f origin = board_origin(board, boardSize);
    v2f stoneSize = board_get_stone_size(board);
    
    u32 maxStoneCount = board->lineCount*board->lineCount;
    
    // Allocate array of Stones and clear it to zero
    Stone *stones = xnalloc(maxStoneCount, Stone);
    
    // Get stones from board
    board_get_stones(board, stones);
    
    // Draw the stones from the array of stones placed
    for (s32 y = 0; y < board->lineCount; y++)
    {
        for (s32 x = 0; x < board->lineCount; x++)
        {
            Stone *stone = board_get_stone(board, stones, x, y);
            if (stone->player > 0)
            {
                v2f pos = add2f(origin, board_stone_pos(board, x, y));
                draw_sprite(&editor.layer2, 
                            sub2f(pos, mul2f(.5f, stoneSize)),
                            stoneSize,
                            (v4f){1,1,1,1},
                            (stone->player == 1) ? editor.stoneBlack : editor.stoneWhite);
            }
        }
    }
    
    // Free stones array
    xfree(stones);
}

function void
board_draw_closest_pos(Board *board)
{
    v2f boardSize = board_size(board);
    v2f origin = board_origin(board, boardSize);
    v2f stoneSize = board_get_stone_size(board);
    
    if (editor.closestPosActive)
    {
        v2f closestPos = board_stone_pos(board, 
                                         editor.closestBoardPoint.x,
                                         editor.closestBoardPoint.y);
        
        draw_rect(&editor.layer2,
                  add2f(origin, sub2f(closestPos, mul2f(.2f, stoneSize))),
                  mul2f(.4f, stoneSize),
                  !board->currentPlayer ? (v4f){0,0,0,1} : (v4f){1,1,1,1});
    }
}

function void
board_draw(Board *board)
{
    // Draw the lines of the board
    board_draw_lines(board);
    
    // Draw the stones of the board
    board_draw_stones(board);
    
    // Draw the closest position on the board if it is active
    board_draw_closest_pos(board);
}

function Stone *
board_get_stone_above(Board *board, Stone *stones, 
                      s32 x, s32 y)
{
    return board_get_stone(board, stones, x, y+1);
}

function Stone *
board_get_stone_below(Board *board, Stone *stones, 
                      s32 x, s32 y)
{
    Stone *result = 0;
    
    if (y > 0)
    {
        result = board_get_stone(board, stones, x, y-1);
    }
    
    return result;
}

function Stone *
board_get_stone_left(Board *board, Stone *stones, 
                     s32 x, s32 y)
{
    Stone *result = 0;
    
    if (x >= 1)
    {
        result = board_get_stone(board, stones, x-1, y);
    }
    
    return result;
}

function Stone *
board_get_stone_right(Board *board, Stone *stones, 
                      s32 x, s32 y)
{
    Stone *result = 0;
    
    if (x < board->lineCount-1)
    {
        result = board_get_stone(board, stones, x+1, y);
    }
    
    return result;
}

function bool
stone_is_placed(Stone *stone)
{
    return (stone && stone->player > 0);
}

function s32
board_find_touching_stones(Board *board, Stone *stones,
                           Stone *destStones, 
                           s32 x, s32 y)
{
    s32 foundCount = 0;
    
    Stone *up = board_get_stone_above(board, stones, x, y);
    Stone *left = board_get_stone_left(board, stones, x, y);
    Stone *down = board_get_stone_below(board, stones, x, y);
    Stone *right = board_get_stone_right(board, stones, x, y);
    
    if (stone_is_placed(up)) destStones[foundCount++] = *up;
    if (stone_is_placed(left)) destStones[foundCount++] = *left;
    if (stone_is_placed(down)) destStones[foundCount++] = *down;
    if (stone_is_placed(right)) destStones[foundCount++] = *right;
    
    return foundCount;
}

function bool
board_stone_array_find(Board *board, 
                       Stone *array, s32 arrayCount,
                       Stone obj)
{
    bool result = false;
    
    for(s32 i = 0; i < arrayCount; i++)
    {
        Stone *stone = array + i;
        if (stone->x == obj.x &&
            stone->y == obj.y)
        {
            result = true;
            break;
        }
    }
    
    return result;
}

function bool
board_stone_array_remove(Board *board, 
                         Stone *array, s32 *arrayCount,
                         Stone obj)
{
    bool result = false;
    
    // For each in the array
    for(s32 i = 0; i < *arrayCount; i++)
    {
        // Get the stone
        Stone *stone = array + i;
        
        // If X,Y are equal
        if (stone->x == obj.x && stone->y == obj.y)
        {
            // Mark as removed
            result = true;
            
            // Put the last position of the array in this removed pos
            array[i] = array[*arrayCount-1];
            
            // Decrement the array
            *arrayCount = *arrayCount - 1;
            
            break;
        }
    }
    
    return result;
}


function void
board_stone_liberties_count_unique(Board *board, Stone *stones,
                                   Stone *liberties, s32 *libertyCount,
                                   s32 x, s32 y)
{
    if (y < board->lineCount-1)
    {
        Stone *up = board_get_stone(board, stones, x, y+1);
        if (!stone_is_placed(up))
        {
            if (!board_stone_array_find(board, liberties, *libertyCount, *up))
            {
                liberties[*libertyCount] = *up;
                *libertyCount = *libertyCount + 1;
            }
        }
    }
    
    if (x < board->lineCount-1)
    {
        Stone *right = board_get_stone(board, stones, x+1, y);
        if (!stone_is_placed(right))
        {
            if (!board_stone_array_find(board, liberties, *libertyCount, *right))
            {
                liberties[*libertyCount] = *right;
                *libertyCount = *libertyCount + 1;
            }
        }
    }
    
    if (y > 0)
    {
        Stone *down = board_get_stone(board, stones, x, y-1);
        if (!stone_is_placed(down))
        {
            if (!board_stone_array_find(board, liberties, *libertyCount, *down))
            {
                liberties[*libertyCount] = *down;
                *libertyCount = *libertyCount + 1;
            }
        }
    }
    
    if (x > 0)
    {
        Stone *left = board_get_stone(board, stones, x-1, y);
        if (!stone_is_placed(left))
        {
            if (!board_stone_array_find(board, liberties, *libertyCount, *left))
            {
                liberties[*libertyCount] = *left;
                *libertyCount = *libertyCount + 1;
            }
        }
    }
}

function void
board_find_connected_stones(Board *board, Stone *stones,
                            Stone *destStones, s32 *destStoneIndex,
                            s32 x, s32 y, s32 player)
{
    // ****************************** ABOVE
    // If there is a stone above
    Stone *up = board_get_stone_above(board, stones, x, y);
    
    // And it is of the same player
    if (stone_is_placed(up) && up->player == player)
    {
        bool foundStone = false;
        // Look in the array to see if find the stone
        for (s32 i = 0; i < *destStoneIndex; i++)
        {
            Stone *stone = destStones + i;
            // If X,Y are the same
            if (stone_is_placed(stone) &&
                stone->x == up->x && stone->y == up->y)
            {
                // Found the stone in the array
                foundStone = true;
                break;
            }
        }
        
        // If the stones is not in the array yet
        if (!foundStone)
        {
            // Add it to the connected stones array
            destStones[*destStoneIndex] = *up;
            
            // Increment the array index
            *destStoneIndex = *destStoneIndex + 1;
            
            // Call recursively on it to check for its neighbors
            board_find_connected_stones(board, stones,
                                        destStones, destStoneIndex,
                                        up->x, up->y, player);
        }
    }
    
    // ****************************** LEFT
    // If there is a stone to the left
    Stone *left = board_get_stone_left(board, stones, x, y);
    
    // And it is of the same player
    if (stone_is_placed(left) && left->player == player)
    {
        bool foundStone = false;
        // Look in the array to see if find the stone
        for (s32 i = 0; i < *destStoneIndex; i++)
        {
            Stone *stone = destStones + i;
            // If X,Y are the same
            if (stone_is_placed(stone) &&
                stone->x == left->x && stone->y == left->y)
            {
                // Found the stone in the array
                foundStone = true;
                break;
            }
        }
        
        // If the stones is not in the array yet
        if (!foundStone)
        {
            // Add it to the connected stones array
            destStones[*destStoneIndex] = *left;
            
            // Increment the array index
            *destStoneIndex = *destStoneIndex + 1;
            
            // Call recursively on it to check for its neighbors
            board_find_connected_stones(board, stones,
                                        destStones, destStoneIndex,
                                        left->x, left->y, player);
        }
    }
    
    // ****************************** BELOW
    // If there is a stone to the down
    Stone *down = board_get_stone_below(board, stones, x, y);
    
    // And it is of the same player
    if (stone_is_placed(down) && down->player == player)
    {
        bool foundStone = false;
        // Look in the array to see if find the stone
        for (s32 i = 0; i < *destStoneIndex; i++)
        {
            Stone *stone = destStones + i;
            // If X,Y are the same
            if (stone_is_placed(stone) &&
                stone->x == down->x && stone->y == down->y)
            {
                // Found the stone in the array
                foundStone = true;
                break;
            }
        }
        
        // If the stones is not in the array yet
        if (!foundStone)
        {
            // Add it to the connected stones array
            destStones[*destStoneIndex] = *down;
            
            // Increment the array index
            *destStoneIndex = *destStoneIndex + 1;
            
            // Call recursively on it to check for its neighbors
            board_find_connected_stones(board, stones,
                                        destStones, destStoneIndex,
                                        down->x, down->y, player);
        }
    }
    
    // ****************************** RIGHT
    // If there is a stone to the right
    Stone *right = board_get_stone_right(board, stones, x, y);
    
    // And it is of the same player
    if (stone_is_placed(right) && right->player == player)
    {
        bool foundStone = false;
        // Look in the array to see if find the stone
        for (s32 i = 0; i < *destStoneIndex; i++)
        {
            Stone *stone = destStones + i;
            // If X,Y are the same
            if (stone_is_placed(stone) &&
                stone->x == right->x && stone->y == right->y)
            {
                // Found the stone in the array
                foundStone = true;
                break;
            }
        }
        
        // If the stones is not in the array yet
        if (!foundStone)
        {
            // Add it to the connected stones array
            destStones[*destStoneIndex] = *right;
            
            // Increment the array index
            *destStoneIndex = *destStoneIndex + 1;
            
            // Call recursively on it to check for its neighbors
            board_find_connected_stones(board, stones,
                                        destStones, destStoneIndex,
                                        right->x, right->y, player);
        }
    }
}