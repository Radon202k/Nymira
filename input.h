#pragma once

function void input()
{
    Board *board = editor.currentBoard;
    
    // Gets the stones of the board constructed by going from
    // the tree root to the point
    s32 maxStoneCount = board->lineCount*board->lineCount;
    Stone *stones = alloc_array(maxStoneCount, Stone);
    board_get_stones(board, stones);
    
    // Handle arrow navigation
    if (engine.key.up.pressed)
    {
        move_up(&board->tree);
        board->currentPlayer = !board->currentPlayer;
    }
    else if (engine.key.left.pressed)
    {
        move_left(&board->tree);
    }
    else if (engine.key.down.pressed)
    {
        move_down(&board->tree);
        board->currentPlayer = !board->currentPlayer;
    }
    else if (engine.key.right.pressed)
    {
        move_right(&board->tree);
    }
    
    // Handle mouse input
    
    Vector2 boardSize = board_size(board);
    Vector2 origin = board_origin(board, boardSize);
    // If the mouse is close enough to a valid move position
    bool foundClosesPos = board_closest_pos(origin, 
                                            &editor.closestBoardPoint.x, 
                                            &editor.closestBoardPoint.y);
    if (foundClosesPos)
    {
        // If the mouse left button is pressed
        if (engine.mouse.left.pressed)
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
                    Stone *connectedEnemies = alloc_array(maxStoneCount, Stone);
                    s32 connectedEnemyCount = 0;
                    board_find_connected_stones(board, stones,
                                                connectedEnemies, &connectedEnemyCount, 
                                                touchingStone->x, touchingStone->y,
                                                touchingStone->player);
                    
                    s32 maxLibertyCount = 4 + connectedEnemyCount*4;
                    Stone *liberties = alloc_array(maxLibertyCount, Stone);
                    
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
                    
                    free(liberties);
                    
                    // Free connected enemies array
                    free(connectedEnemies);
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
    free(stones);
}