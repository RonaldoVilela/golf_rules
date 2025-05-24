#pragma once

#include <unordered_map>
#include <glad/glad.h>

struct Glyph {
    float x0, y0, x1, y1;       // atla's coordenades
    int width, height;          // width and height in pixels;
    float xoff, yoff;           // letter offset
    float xadvance;             // how much advance for the next character
};

struct FontAtlas {
    unsigned int textureID;
    int width, height;
    float ascent, descent, lineGap;
    std::unordered_map<uint32_t, Glyph> glyphs; // unicode code of each character
};

FontAtlas LoadFont(const char* ttf_fontPath, float fontSize, int atlasWidth = 512, int atlasHeight = 512);
uint32_t DecodeUTF8(const char* &str);