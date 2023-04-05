#pragma once

// Compares two moves, returns true if they are equal
function bool
move_compare(Move a, Move b)
{
    if (a.x == b.x && a.y == b.y)
    {
        return true;
    }
    
    return false;
}

function s32
move_tree_height(MoveNode *node, s32 height)
{
    if (node == NULL) return height;
    
    s32 childH = move_tree_height(node->firstChild, height+1);
    s32 rightH = move_tree_height(node->rightSibling, height+1);
    
    return maxf(childH, rightH);
}


// Find a move in the tree if it exists
// Performs depth-first search
function MoveNode *
move_find(MoveNode *root, Move move)
{
    MoveNode *current = root;
    while (current != NULL)
    {
        if (move_compare(current->data, move))
        {
            return current;
        }
        
        MoveNode *found = move_find(current->firstChild, move);
        if (found != NULL)
        {
            return found;
        }
        current = current->rightSibling;
    }
    return NULL;
}

// Allocate a MoveNode data structure
function MoveNode *
move_allocate(u32 x, u32 y, u32 player)
{
    MoveNode *newNode = (MoveNode *)malloc(sizeof(MoveNode));
    memset(newNode, 0, sizeof(MoveNode));
    newNode->data.x = x;
    newNode->data.y = y;
    newNode->data.player = player;
    return newNode;
}

// Moves the point to the parent node if it exists
function bool
move_up(MoveTree *tree)
{
    if (tree->point->parent)
    {
        tree->point = tree->point->parent;
        return true;
    }
    return false;
}

// Moves the point to the left sibling node if it exists
function bool
move_left(MoveTree *tree)
{
    if (tree->point->leftSibling)
    {
        tree->point = tree->point->leftSibling;
        return true;
    }
    return false;
}

// Moves the point to the first child if it exists
function bool
move_down(MoveTree *tree)
{
    if (tree->point->firstChild)
    {
        tree->point = tree->point->firstChild;
        return true;
    }
    return false;
}

// Moves the point to the right sibling if it exists
function bool
move_right(MoveTree *tree)
{
    if (tree->point->rightSibling)
    {
        tree->point = tree->point->rightSibling;
        return true;
    }
    return false;
}

// Place a stone in the board at the point
function void
move_place(MoveTree *tree, u32 x, u32 y, u32 player)
{
    // If there is no root
    if (!tree->root)
    {
        // Allocate a move for root
        tree->root = move_allocate(x, y, player);
        
        // And set the point to point to it
        tree->point = tree->root;
    }
    // Otherwise
    else
    {
        // Start at the point
        MoveNode *current = tree->point;
        
        // Allocate the new node
        MoveNode *newNode = move_allocate(x, y, player);
        
        // Set the current as the parent
        newNode->parent = current;
        
        // If there is no children
        if (!current->firstChild)
        {
            // Just set it as the first child
            current->firstChild = newNode;
        }
        // Otherwise
        else
        {
            // Move to the first child
            current = current->firstChild;
            
            // While there is a right sibling
            while (current->rightSibling)
            {
                // Move to the right sibling
                current = current->rightSibling;
            }
            
            // Now, current variable has the last sibling
            assert(current->rightSibling == 0);
            
            // Set the new node as the right most sibling
            current->rightSibling = newNode;
            
            // Set the left sibling pointer
            newNode->leftSibling = current;
        }
        
        // Update the point
        tree->point = newNode;
    }
}

// Depth-first traversal to draw the tree
function void
move_tree_draw_diagram(MoveTree *tree, MoveNode *root, v2f origin)
{
    if (root == 0) return;
    
    v4f color = {1,1,1,0.4f};
    if (tree->point == root)
    {
        color = (v4f){1,1,1,1};
        editor.nodeY = origin.y;
    }
    
    float stoneHeight = editor.stoneBlack.size.y;
    
    // Draw it
    draw_sprite(&editor.layer1, origin, (v2f){.5f,.5f}, color, 
                root->data.player == 1 ? editor.stoneBlack : editor.stoneWhite);
    
#if 0
    Vector2i value = {root->data.x, root->data.y};
    draw_label_v2i(editor.layer2, &editor.font32, value, 
                   v2(origin.x, origin.y), 
                   10, rgba(1,1,1,0.5f), 1, false);
#endif
    
    move_tree_draw_diagram(tree, root->firstChild, 
                           (v2f){origin.x,origin.y-.5f*stoneHeight});
    
    move_tree_draw_diagram(tree, root->rightSibling, 
                           (v2f){origin.x+.5f*stoneHeight,origin.y});
}
