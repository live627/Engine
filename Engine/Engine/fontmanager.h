#pragma once


///////////////////////////////
// PRE-PROCESSING DIRECTIVES //
///////////////////////////////
// Windows is too helpful sometimes.
#define NOMINMAX


///////////////////////
// INCLUDES //
///////////////////////
#include <wrl\client.h>


///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "textureclass.h"
#include "fontshaderclass.h"
#include "game.h"


class Fonts
{
public:
	Fonts(ID3D11Device * p_device, ID3D11DeviceContext * p_deviceContext)
		:
		m_device(p_device),
		m_deviceContext(p_deviceContext),
		m_textures(std::make_unique<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>[]>(2))
	{
		LoadFontBitmap("data\\Verdana - Copy.dds", "data\\Verdana.dat");
		LoadFontBitmap2("data\\Consolas.dds");
	}
	void LoadFontBitmap(const char *, const char *);
	auto GetTexture(int idx) const { return m_textures[idx].Get(); }
	void BuildVertexArray(void *, const char *, float, float);
	POINT && MeasureString(const char *);
	void LoadFontBitmap2(const char*);
	void BuildVertexArray2(void *, const char *, float, float);
	POINT && MeasureString2(const char *);

private:
	ID3D11Device * m_device;
	ID3D11DeviceContext * m_deviceContext;

	struct CharRecord
	{
		float X;
		float Y;
		float Width;
		float Height;

		float xoffset;
		float yoffset;
		float xadvance;
	};

	std::unique_ptr<CharRecord[]> m_chars;
	std::unique_ptr<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>[]> m_textures;

	struct Character {
		int codePoint;

		float X;
		float Y;
		float Width;
		float Height;

		float xoffset;
		float yoffset;
	};

	struct FontType {
		const char * name;
		int size, bold, italic, width, height, characterCount;
		std::vector<Character> characters;
	};

	FontType m_font_Consolas;
};