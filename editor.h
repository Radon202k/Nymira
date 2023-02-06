#pragma once

function void
editor_render_init()
{
    // Create the sprites in the texture atlas from png files
    editor.font = sprite_create("images/font.png");
    editor.white = sprite_create("images/white.png");
    editor.naruto = sprite_create("images/naruto.png");
    editor.stoneBlack = sprite_create("images/stone_black.png");
    editor.stoneWhite = sprite_create("images/stone_white.png");
    
    // Create render groups with different sorting layers
    editor.layer1 = render_group_push_layer(1);
    editor.layer2 = render_group_push_layer(2);
}