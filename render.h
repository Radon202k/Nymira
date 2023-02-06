#pragma once

function void
base_draw_char(RenderGroup *group, Sprite font,
               char c, float x, float y, float size,
               float r, float g, float b, float a,
               float layer)
{
    float uvWidth = font.uv.max.x - font.uv.min.x;
    float uvHeight = font.uv.max.y - font.uv.min.y;
    
    float charUVwidth = (uvWidth / 16);
    float charUVheight = (uvHeight / 16);
    
    int charX = ((int)c) % 16;
    int charY = ((int)c) / 16;
    
    base_draw_rect(group,
                   x, y,
                   size, size,
                   font.uv.min.x + charX*charUVwidth, font.uv.min.y + charY*charUVheight,
                   font.uv.min.x + (charX+1)*charUVwidth, font.uv.min.y + (charY+1)*charUVheight,
                   r,g,b,a,
                   layer);
}

function void
draw_char(RenderGroup *group, Sprite font,
          char c, Vector2 pos, float size, Color col, float layer)
{
    base_draw_char(group, font,
                   c, 
                   pos.x, pos.y,
                   size,
                   col.r, col.g, col.b, col.a,
                   layer);
}

function void
draw_string(RenderGroup *group, Sprite font, 
            char *string, Vector2 pos, float size, Color col, float layer)
{
    char *at = string;
    while (*at)
    {
        draw_char(group, font, *at++, pos, size, col, layer);
        pos.x += size;
    }
}

function void
draw_label_int(RenderGroup *group, Sprite font,
               s32 value, Vector2 pos, float size, Color col, float layer,
               bool centered)
{
    // Get a string for the number using _itoa_s
    char number[32] = {0};
    _itoa_s(value, number, 32, 10);
    
    // Claculate the char size and position
    Vector2 charPos = pos;
    
    if (centered)
    {
        charPos.x -= strlen(number) * 0.5f*size;
    }
    
    draw_string(group, font,
                number, v2(charPos.x, charPos.y - 16.0f), size, col, layer);
}

function void
draw_label_v2i(RenderGroup *group, Sprite font,
               Vector2i value, Vector2 pos, float size, Color col, float layer,
               bool centered)
{
    // Get a string for the number using _itoa_s
    char xs[15] = {0};
    _itoa_s(value.x, xs, 16, 10);
    
    char ys[15] = {0};
    _itoa_s(value.y, ys, 16, 10);
    
    char number[32] = {0};
    strcat_s(number, 32, xs);
    strcat_s(number, 32, ", ");
    strcat_s(number, 32, ys);
    
    // Calculate the char size and position
    Vector2 charPos = pos;
    
    if (centered)
    {
        charPos.x -= strlen(number)*0.5f*size;
    }
    
    draw_string(group, font, number, 
                v2(charPos.x, charPos.y - 16.0f), size, col, layer);
    
}