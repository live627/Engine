#pragma once


///////////////////////////////
// PRE-PROCESSING DIRECTIVES //
///////////////////////////////
// Windows is too helpful sometimes.
#define NOMINMAX


///////////////////////
// INCLUDES //
///////////////////////
#include "ft2build.h"
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include <wrl\client.h>


///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "textureclass.h"
#include "fontshaderclass.h"
#include "game.h"


/////////////
// LINKING //
/////////////
#pragma comment(lib, "libfreetype.lib")


////////////////////////////////////////////////////////////////////////////////
// Class name: Fonts
////////////////////////////////////////////////////////////////////////////////
class Font
{
public:
	Font() = default;
	Font(ID3D11Device * p_device, ID3D11DeviceContext * p_deviceContext)
	:
		m_device(p_device),
		m_deviceContext(p_deviceContext),
		m_numGlyphs(127 - 32)
	{}

	bool LoadTTF(FT_Library, FT_Byte *, FT_Long);
	auto GetTexture() const { return m_texture.Get(); }
	void BuildVertexArray(void *, const char *, float, float);
	POINT && MeasureString(const char *);

private:
	struct GlyphInfo 
	{
		uint32_t 
			ax, // advance.x
			ay, // advance.y

			bw, // bitmap.width
			bh, // bitmap.rows

			x, y; // Position of glyph on texture map in pixels.

		float left, right; // UV coords of glyph on texture map.
		FT_BBox bbox;
	};
	FT_Short
		height,
		max_advance_width;

	void StitchGlyph(const std::byte *, const GlyphInfo &, uint32_t, uint32_t, std::byte *);
	
private:
	ID3D11Device * m_device;
	ID3D11DeviceContext * m_deviceContext;
	FT_Face m_face;
	std::unique_ptr<GlyphInfo[]> m_glyphSlots;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_texture;
	size_t
		m_width,
		m_height = 0,
		m_numGlyphs;
};

class Fonts
{
public:
	Fonts(ID3D11Device * p_device, ID3D11DeviceContext * p_deviceContext)
		:
		m_device(p_device),
		m_deviceContext(p_deviceContext)
	{
		FT_Init_FreeType(&m_library);
		LoadFonts("data\\fonts.dat");
	}
	~Fonts() { FT_Done_FreeType(m_library); }
	void LoadFonts(const char *);
	void LoadFont(FT_Byte *, int32_t, int);
	Font * GetFont(int idx) { return &m_fonts[idx]; }

private:
	ID3D11Device * m_device;
	ID3D11DeviceContext * m_deviceContext;
	FT_Library m_library;
	std::unique_ptr<Font[]> m_fonts;
};