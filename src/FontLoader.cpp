#include <iostream>
#include "FontLoader.h"
#include "Renderer.h"
#include <fstream>
#include <vector>

#include <stb/stb_truetype.h>

FontAtlas LoadFont(const char *ttf_fontPath, float fontSize, int atlasWidth, int atlasHeight)
{
    FontAtlas atlas;
    atlas.width = atlasWidth;
    atlas.height = atlasHeight;

// [1] load the font data from the file //
    std::ifstream file;
    file.open(ttf_fontPath, std::ios::binary);

    if(!file.is_open()){
        std::cout << "Could not find the .ttf font file in the path: " << ttf_fontPath << "\n";
        return atlas;
    }

    //font buffer (file_start, file_end);
    std::vector<unsigned char> fontBuffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>()); // the buffer here

    file.close();
// ================== //


// [2] Initializing the font //
    stbtt_fontinfo fontInfo;

    if(!stbtt_InitFont(&fontInfo, fontBuffer.data(), 0)){
        std::cout << "Couldn't initialize the font \n";
        return atlas;
    }
// ================= //


// [3] Calculate the font metrics //
    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&fontInfo, &ascent, &descent, &lineGap);
    float scale = stbtt_ScaleForPixelHeight(&fontInfo, fontSize);

    atlas.ascent = ascent * scale;
    atlas.descent = descent * scale;
    atlas.lineGap = lineGap * scale;
// ================= //


// [4] generate the atlas bitmap //
    std::vector<unsigned char> bitmap(atlasWidth * atlasHeight, 0);

    int x = 0, y = 0, rowHeight = 0;
    // in the ascii table, only characters equal or greater than 32 are acuatlly printable.
    for (uint32_t codepoint = 32; codepoint < 256; ++codepoint) {

        int w, h, xoff, yoff;
        unsigned char* glyphBitmap = stbtt_GetCodepointBitmap(&fontInfo, 0, scale, codepoint, &w, &h, &xoff, &yoff);

        if (x + w >= atlasWidth) {
            x = 0;
            y += rowHeight;
            rowHeight = 0;
        }

        if (y + h >= atlasHeight) {
            stbtt_FreeBitmap(glyphBitmap, nullptr);
            continue; // atlas full
        }

        // copy to atlas bitmap
        for (int i = 0; i < h; ++i) {
            memcpy(&bitmap[(y + (h - 1 - i)) * atlasWidth + x], &glyphBitmap[i * w], w);
        }

        // save the metrics
        int ax;
        stbtt_GetCodepointHMetrics(&fontInfo, codepoint, &ax, nullptr);

        Glyph glyph = {
            (float)x / atlasWidth,
            (float)(y + h) / atlasHeight,
            (float)(x + w) / atlasWidth,
            (float)y / atlasHeight,
            (float)w,
            (float)h,
            (float)xoff,
            (float)yoff,
            ax * scale
        };
        atlas.glyphs[codepoint] = glyph;

        x += w + 1;
        if (h > rowHeight) rowHeight = h;

        stbtt_FreeBitmap(glyphBitmap, nullptr);
    }
 // ================= //

// [5] Generate the OpenGL Atlas texture //
    GLCall(glGenTextures(1, &atlas.textureID));
    GLCall(glBindTexture(GL_TEXTURE_2D, atlas.textureID));
    
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

    GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, atlasWidth, atlasHeight, 0, GL_RED, GL_UNSIGNED_BYTE, bitmap.data()));

// ==================================== //

    return atlas;
}

uint32_t DecodeUTF8(const char *&str)
{
    return 0;
}
