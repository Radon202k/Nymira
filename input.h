#pragma once

function void input()
{
    Board *board = editor.currentBoard;
    
    // Gets the stones of the board constructed by going from
    // the tree root to the point
    s32 maxStoneCount = board->lineCount*board->lineCount;
    Stone *stones = xnalloc(maxStoneCount, Stone);
    board_get_stones(board, stones);
    bool moveTree = false;
    // Handle arrow navigation
    if (xwin.key.up.pressed)
    {
        move_up(&board->tree);
        board->currentPlayer = !board->currentPlayer;
        moveTree = true;
    }
    else if (xwin.key.left.pressed)
    {
        move_left(&board->tree);
    }
    else if (xwin.key.down.pressed)
    {
        move_down(&board->tree);
        board->currentPlayer = !board->currentPlayer;
        moveTree = true;
    }
    else if (xwin.key.right.pressed)
    {
        move_right(&board->tree);
    }
    
    if (moveTree)
    {
        float bbH = xd11.back_buffer_size.y;
        float bot = editor.contentY - (editor.contentH - bbH);
        
        float desiredPoint = 1 - (editor.nodeY / editor.contentY);
        
        float dist = desiredPoint - editor.point;
        
        editor.pointVel += dist;
    }
    
    // Handle mouse input
    
    v2f boardSize = board_size(board);
    v2f origin = board_origin(board, boardSize);
    // If the mouse is close enough to a valid move position
    bool foundClosesPos = board_closest_pos(origin, 
                                            &editor.closestBoardPoint.x, 
                                            &editor.closestBoardPoint.y);
    if (foundClosesPos)
    {
        // If the mouse left button is pressed
        if (xwin.mouse.left.pressed)
        {
            // Check if there is a stone in that position already
            Stone *stone = board_get_stone(board, stones, 
                                           editor.closestBoardPoint.x, 
                                           editor.closestBoardPoint.y);
            
            // If there is a player set it means someone placed a stone there
            if (stone->player > 0)
            {
                // Stone at this position is already placed on the board
            }
            // Otherwise, there is no stone in place
            else
            {
                // So we can place a move in that position
                move_place(&board->tree,
                           editor.closestBoardPoint.x, 
                           editor.closestBoardPoint.y, 
                           board->currentPlayer+1);
            }
            
            // Check if the placed stone touches any other stones
            // Can have up to 4 stones as a result (up, left, down, right)
            Stone touchingStones[4] = {0};
            s32 touchingCount = board_find_touching_stones(board, stones, 
                                                           touchingStones,
                                                           editor.closestBoardPoint.x, 
                                                           editor.closestBoardPoint.y);
            
            editor.libGroupIndex = 0;
            
            // For each of the found touching stones
            for (s32 i = 0; i < touchingCount; i++)
            {
                Stone *touchingStone = touchingStones + i;
                
                // If it is an enemy stone
                if (touchingStone->player != (board->currentPlayer+1))
                {
                    
                    
                    // Get all connected stones from the same player
                    // to this touching enemy stone
                    Stone *connectedEnemies = xnalloc(maxStoneCount, Stone);
                    s32 connectedEnemyCount = 0;
                    board_find_connected_stones(board, stones,
                                                connectedEnemies, &connectedEnemyCount, 
                                                touchingStone->x, touchingStone->y,
                                                touchingStone->player);
                    
                    s32 maxLibertyCount = 4 + connectedEnemyCount*4;
                    Stone *liberties = xnalloc(maxLibertyCount, Stone);
                    
                    s32 groupLibertyCount = 0;
                    
                    if (connectedEnemyCount)
                    {
                        // Count the liberties of all the connected enemy stones
                        for (s32 j = 0; j < connectedEnemyCount; j++)
                        {
                            Stone connectedEnemy = connectedEnemies[j];
                            board_stone_liberties_count_unique(board, stones,
                                                               liberties,
                                                               &groupLibertyCount,
                                                               connectedEnemy.x,
                                                               connectedEnemy.y);
                        }
                    }
                    else
                    {
                        board_stone_liberties_count_unique(board, stones,
                                                           liberties,
                                                           &groupLibertyCount,
                                                           touchingStone->x,
                                                           touchingStone->y);
                    }
                    
                    board_stone_array_remove(board, 
                                             liberties, &groupLibertyCount,
                                             *stone);
                    
                    // liberties, if the result is zero liberties, it
                    // means this move has captured that group.
                    if (groupLibertyCount == 0)
                    {
                        if (connectedEnemyCount > 0)
                        {
                            for (s32 k = 0; k < connectedEnemyCount; k++)
                            {
                                move_capture(board, connectedEnemies[k]);
                            }
                        }
                        else
                        {
                            move_capture(board, *touchingStone);
                        }
                    }
                    else
                    {
                        LibertyGroup *group = editor.libGroups + editor.libGroupIndex++; 
                        
                        group->count = groupLibertyCount;
                        
                        memcpy(group->array, liberties, groupLibertyCount*sizeof(Stone));
                    }
                    
                    xfree(liberties);
                    
                    // Free connected enemies array
                    xfree(connectedEnemies);
                }
            }
            
            // Switch the current player
            board->currentPlayer = !board->currentPlayer;
        }
        
        editor.closestPosActive = true;
    }
    else
    {
        editor.closestPosActive = false;
    }
    
    // Free stones array
    xfree(stones);
}