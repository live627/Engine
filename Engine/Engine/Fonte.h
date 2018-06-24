////////////////////////////////////////////////////////////////////////////////
// Filename: fontmanager.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _FontManager_H_
#define _FontManager_H_


///////////////////////////////
// PRE-PROCESSING DIRECTIVES //
///////////////////////////////
#define _USE_MATH_DEFINES


///////////////////////
// INCLUDES //
///////////////////////
#include <map>
#include <memory>


///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "d3dclass.h"
#include "vec2.h"
#include "ft2build.h"
#include FT_FREETYPE_H
const FT_UInt DEFAULT_FONT_SIZE = 18;//static_cast<int>(ceilf(SystemClass::SCALEX(8)));


//Data used for information about all glyphs
//A glyph is a bitmap with one character in it
struct GlyphData
{
	Vector2 start;
	Vector2 end;
	Vector2 size;

	unsigned int advance;

	int xOffset;
	int yOffset;
};

//A static string whose content can't be modified
class StaticString
{
	friend class FaceMap;
public:
	void Render(int xpos, int ypos);

protected:
	std::tr1::shared_ptr m_pVerts;

	FaceMap* m_pFace;
	Device* m_pDevice;
};

//To be implemented
class DynamicString : public StaticString
{
	friend class FaceMap;
public:

private:

};

//A FaceMap is a class which contains a texture with all of the glyphs, it is also the class used for creating renderable strings of text
class FaceMap
{
	friend class FontManager;
public:
	StaticString BuildStaticString(const std::string& text, const Vec3& color, unsigned int maxWidth = 0);
	DynamicString BuildDynamicString(unsigned int maxStringLength, unsigned int maxWidth = 0, bool lineBreak = false);

	unsigned int GetStringWidth(std::string text);

	unsigned int GetAdvance(char c1, char c2);

	TexResource GetTexture() { return m_TexResource; }

private:
	FaceMap() {}

	FT_Face m_Face;

	Device* m_pDevice;

	unsigned int m_Size;

	TexResource m_TexResource;
	std::map m_Coords;
};

//A font manager can be used for creating FaceMaps
class FontManager
{
public:
	FontManager() : m_LastError(0) {}

	bool Initialize(Device* pDevice);

	bool LoadFont(std::string fontName);

	std::vector GetAvailableFonts() const;

	FaceMap* CreateFace(std::string familyName, std::string styleName, unsigned int size);

private:
	FT_Library m_Library;
	std::multimap m_Faces;
	std::map, FaceMap*> m_FaceMaps;
	Device* m_pDevice;

	const char* m_LastError;
};