#pragma once

function void
editor_render_init()
{
    /* Initialize Direct3D 11 */
    xd11_initialize((XD11Config){window_proc, 0, 0, L"My Window"});
    
    /* Initialize Input */
    xwin_initialize((XWindowConfig){LoadCursor(NULL, IDC_ARROW), xd11.window_handle});
    
    /* Initialize renderer */
    xrender2d_initialize((v4f){.2f,.2f,.2f,1});
    
    random_seed(0);
    
    // Create the sprites in the texture atlas from png files
    editor.debugFont = xrender2d_sprite_from_png(&xrender2d.texture_atlas,
                                                 L"images/font.png", false);
    
    editor.white = xrender2d_sprite_from_png(&xrender2d.texture_atlas,
                                             L"images/white.png", false);
    
    editor.naruto = xrender2d_sprite_from_png(&xrender2d.texture_atlas, 
                                              L"images/naruto.png", false);
    
    editor.stoneBlack = xrender2d_sprite_from_png(&xrender2d.texture_atlas,
                                                  L"images/stone_black.png", false);
    
    editor.stoneWhite = xrender2d_sprite_from_png(&xrender2d.texture_atlas,
                                                  L"images/stone_white.png", false);
    
    wchar_t fontFullPath[260];
    xwin_path_abs(fontFullPath, 260, 
                  L"fonts\\Inconsolata.ttf");
    
    editor.font32 = xrender2d_font(&xrender2d.texture_atlas,
                                   fontFullPath, L"Inconsolata", 16);
    
    /* Update texture atlas */
    xd11_texture2d_update(xrender2d.texture_atlas.texture,
                          xrender2d.texture_atlas.bytes);
}