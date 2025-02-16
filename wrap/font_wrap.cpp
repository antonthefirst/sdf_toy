#include "font_wrap.h"
#include "math/maths.h" // for vec2
#include "core/container.h"
#include <stdio.h>
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

#define STB_RECT_PACK_IMPLEMENTATION
#include "stb/stb_rect_pack.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb/stb_truetype.h"

struct FontMetrics {
	float ascent;
	float descent;
	float line_gap;
};
struct GlyphTableEntry {
	ivec2 atlas_ll, atlas_ur;
	ivec2 center_offs;
	ivec2 dims;
	float advance_width;
	float left_side_bearing;
};
struct GlyphData {
	unsigned char* sdf;
	int w, h, x, y;
};

static int sdf_padding = 16;

#define FONT_BUFFER_BYTESIZE (1 << 25)

struct Font {
	stbtt_fontinfo info;
	unsigned char ttf_buffer[FONT_BUFFER_BYTESIZE]; // from the sample...kinda massive?
	FontMetrics metrics;
	Bunch<GlyphTableEntry> glyph_table;
	Bunch<int> codepoints;   // for mapping codepoint to index (replace with hash?)
	GLuint tex = 0;

	// temporary?
	ivec2 atlas_res = ivec2(1024);
	unsigned char* atlas_data = nullptr;
};

void writeToTex(GLuint* tex, unsigned char* data, int w, int h) {
	glGenTextures(1, tex);
	glBindTexture(GL_TEXTURE_2D, *tex);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);
}

static void makeFontAtlas(Font* F, float pixel_height) {
	F->codepoints.clear();
	// ASCII codes
	for (int i = 32; i <= 126; ++i)
		F->codepoints.push(i);
	stbrp_context context;
	stbrp_node* nodes = (stbrp_node*)malloc(sizeof(stbrp_node) * F->atlas_res.x);
	stbrp_init_target(&context, F->atlas_res.x, F->atlas_res.y, nodes, F->atlas_res.x);

	Bunch<GlyphData> glyphs; // for x,y position within tex
	glyphs.setgarbage(F->codepoints.count);

	Bunch<stbrp_rect> rects; // for the uv coords
	rects.setgarbage(F->codepoints.count);

	float font_scale = stbtt_ScaleForPixelHeight(&F->info, pixel_height);
	
	unsigned char onedge_value = 128;
	//float pixel_dist_scale = 16.0f;
	float pixel_dist_scale = 4.0f;

#if 0 // for testing
	{
		int code = 'H';
		int gi = code - 32;
		GlyphData& g = glyphs[gi];
		memset(&g, 0, sizeof(GlyphData));
		g.sdf = stbtt_GetCodepointSDF(&F->info, font_scale, code, sdf_padding, onedge_value, pixel_dist_scale, &g.w, &g.h, &g.x, &g.y);
		//log("wh (%d, %d), xy (%d, %d), -pad wh (%d, %d)\n", g.w, g.h, g.x, g.y, g.w - sdf_padding *2, g.h - sdf_padding *2);
	}
#endif

	for (int i = 0; i < F->codepoints.count; ++i) {
		GlyphData& g = glyphs[i];
		memset(&g, 0, sizeof(GlyphData));
		g.sdf = stbtt_GetCodepointSDF(&F->info, font_scale, F->codepoints[i], sdf_padding, onedge_value, pixel_dist_scale, &g.w, &g.h, &g.x, &g.y);
		//log("glyph '%c': dims %d,%d pos %d,%d\n", (char)F->codepoints[i], g.w, g.h, g.x, g.y);
		rects[i].w = (unsigned short)g.w;
		rects[i].h = (unsigned short)g.h;
		rects[i].x = (unsigned short)0;
		rects[i].y = (unsigned short)0;
		rects[i].was_packed = 0;
	}
	if (stbrp_pack_rects(&context, rects.ptr, rects.count)) {
		F->atlas_data = (unsigned char*)malloc(F->atlas_res.x * F->atlas_res.y);
		memset(F->atlas_data, 0xff, F->atlas_res.x * F->atlas_res.y);
		for (int i = 0; i < F->codepoints.count; ++i) {
			if (!rects[i].was_packed) {
				printLog("[Font] %d (%d: %c) wasn't packed!\n", i, F->codepoints[i], F->codepoints[i]);
			} else {
				//log("%d %c -> (%d %d) (%d %d)\n", F->codepoints[i], F->codepoints[i], rects[i].x, rects[i].y, rects[i].w, rects[i].h);
			}
			for (int y = 0; y < rects[i].h; ++y) {
				for (int x = 0; x < rects[i].w; ++x) {
					int atlas_x = rects[i].x + x;
					int atlas_y = rects[i].y + y;
					F->atlas_data[atlas_x + atlas_y * F->atlas_res.x] = 255 - glyphs[i].sdf[x + y * rects[i].w];
				}
			}
		}
		writeToTex(&F->tex, F->atlas_data, F->atlas_res.x, F->atlas_res.y);
		free(F->atlas_data);
	} else {
		printLog("[Font] Couldn't pack\n");
	}
	free(nodes);
	for (int i = 0; i < F->codepoints.count; ++i) { // #TODO glyphs.count instead?
		stbtt_FreeSDF(glyphs[i].sdf, 0);
	}
	F->glyph_table.clear();
	F->glyph_table.setgarbage(1024);
	memset(F->glyph_table.ptr, 0, F->glyph_table.bytes());
	for (int i = 0; i < F->codepoints.count; ++i) {
		int code = F->codepoints[i];
		F->glyph_table[code].atlas_ll = ivec2(rects[i].x, rects[i].y);
		F->glyph_table[code].atlas_ur = F->glyph_table[code].atlas_ll + ivec2(rects[i].w, rects[i].h);
		/* xy returned by GetCodepointSDF is shifted up by padding, but we draw at the bounds and pad in shader, so remove that. */
		F->glyph_table[code].center_offs = ivec2(glyphs[i].x, glyphs[i].y) + ivec2(sdf_padding);
		/* wh returned by GetCodepointSDF includes padding, so subtract it here for the real wh (since we pad in shader) */
		F->glyph_table[code].dims = ivec2(rects[i].w, rects[i].h) - ivec2(int(sdf_padding*2.0f));
		int aw, lsb;
		stbtt_GetCodepointHMetrics(&F->info, code, &aw, &lsb);
		F->glyph_table[code].advance_width = aw * font_scale;
		F->glyph_table[code].left_side_bearing = lsb * font_scale;
	}
	int as, de, lg;
	stbtt_GetFontVMetrics(&F->info, &as, &de, &lg);
	F->metrics.ascent = as * font_scale;
	F->metrics.descent = de * font_scale;
	F->metrics.line_gap = lg * font_scale;
}

Font* fontMake(const char* pathfile, float pixel_height) {
	Font* F = NULL;
	FILE* file;
	fopen_s(&file, pathfile, "rb");
	if (file) {
		F = new Font();
		fread(F->ttf_buffer, 1, FONT_BUFFER_BYTESIZE, file);
		stbtt_InitFont(&F->info, F->ttf_buffer, stbtt_GetFontOffsetForIndex(F->ttf_buffer, 0));
		makeFontAtlas(F, pixel_height);
		fclose(file);
	}
	return F;
}
void fontDestroy(Font* F) {
	delete F;
}
unsigned int fontTex(Font* F, float* padding) {
	if (padding) *padding = (float)sdf_padding;
	return F->tex;
}
void fontGlyph(Font* F, int codepoint, GlyphInfo* glyph)
{
	/* TEMP, replace with a hash lookup. */
	if (codepoint < 0 || codepoint >= F->glyph_table.count) return;
	const GlyphTableEntry& G = F->glyph_table[codepoint];

	/* This is silly, should use GlyphInfo internally too. */
	glyph->x = (float)G.center_offs.x;
	glyph->y = (float)G.center_offs.y;
	glyph->w = (float)G.dims.x;
	glyph->h = (float)G.dims.y;
	glyph->advance = (float)G.advance_width;
	glyph->uvx0 = G.atlas_ll.x / float(F->atlas_res.x);
	glyph->uvy0 = G.atlas_ll.y / float(F->atlas_res.y);
	glyph->uvx1 = G.atlas_ur.x / float(F->atlas_res.x);
	glyph->uvy1 = G.atlas_ur.y / float(F->atlas_res.y);
}
void fontMetrics(Font* F, float* ascent, float* descent, float* line_gap, float* cap_height, float* x_height) {
	if (ascent) *ascent = F->metrics.ascent;
	if (descent) *descent = F->metrics.descent;
	if (line_gap) *line_gap = F->metrics.line_gap;
	/* Real cap and x height are in the OS2 table, and not all fonts may have them, so might as well follow the doc and approximate ourselves. */
	/* https://docs.microsoft.com/en-us/typography/opentype/spec/os2#scapheight */
	if (cap_height) *cap_height = (float)F->glyph_table[(int)'H'].dims.y;
	if (x_height) *x_height = (float)F->glyph_table[(int)'x'].dims.y;
}

#include "imgui/imgui.h"
void fontDebug(Font* F) {
	gui::Begin("Font Debug");
	gui::Image((ImTextureID)F->tex, vec2(F->atlas_res));
	gui::End();
}