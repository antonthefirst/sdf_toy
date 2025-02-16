#pragma once

struct Font;

/* Make/Destroy */
Font* fontMake(const char* pathfile, float pixel_height = 32.0f);
void  fontDestroy(Font*);

/* Usage */
unsigned int fontTex(Font*, float* padding);
struct GlyphInfo {
	// Typesetting info
	float w,h; // width and height of the glyph
	float x,y; // offset of glyph from origin
	float advance; // advance to next glyph (without kerning)

	// Rendering info
	float uvx0, uvy0, uvx1, uvy1; // uv coordinates within the font texture, including padding
};
void fontGlyph(Font*, int codepoint, GlyphInfo* glyph);
void fontMetrics(Font*, float* ascent, float* descent, float* line_gap, float* cap_height, float* x_height);

/* Debug */
void fontDebug(Font*);