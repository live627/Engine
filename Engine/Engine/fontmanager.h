#pragma once


///////////////////////
// INCLUDES //
///////////////////////
#include <wrl\client.h>


///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "fontshaderclass.h"
#include "game.h"


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

	bool LoadTTF(uint8_t *, uint64_t);
	auto GetTexture() { return m_texture.Get(); }
	void BuildVertexArray(void *, const char *, float, float);

private:
	struct GlyphInfo 
	{
		unsigned int 
			ax, // advance.x
			ay, // advance.y

			bw, // bitmap.width
			bh, // bitmap.rows

			x, y; // Position of glyph on texture map in pixels.

		float left, right; // UV coords of glyph on texture map.
	};
	
	void CreateShaderResourceView(uint, uint, uint, const byte *);
	
	ID3D11Device * m_device;
	ID3D11DeviceContext * m_deviceContext;
	std::vector<GlyphInfo> m_glyphSlots;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_texture;
	size_t
		m_width = 0,
		m_height = 0,
		m_numGlyphs = 0;
};

class Fonts
{
public:
	Fonts(ID3D11Device * p_device, ID3D11DeviceContext * p_deviceContext)
		:
		m_device(p_device),
		m_deviceContext(p_deviceContext)
	{
		LoadFonts("data\\fonts.dat");
	}
	void LoadFonts(const char *);
	void LoadFont(uint8_t *, uint64_t, int);
	Font * GetFont(int idx) { return &m_fonts[idx]; }

private:
	ID3D11Device * m_device;
	ID3D11DeviceContext * m_deviceContext;
	std::unique_ptr<Font[]> m_fonts;
};