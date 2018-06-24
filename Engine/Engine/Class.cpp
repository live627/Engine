#include "Class.h"



Class::Class()
{

	// TODO: Place code here.
	TTF::Font font("some_font.ttf");

	TTF::FontMetrics font_metrics = font.GetFontMetrics();
	
	font.TriangulateGlyph(TTF::CodePoint(65), triangulator);

	for (auto tri : triangulator) {
		TTF::vec2t v0 = triangulator[tri.i0];
		TTF::vec2t v1 = triangulator[tri.i1];
		TTF::vec2t v2 = triangulator[tri.i2];
		// store in a buffer, or do something with it from here... up to you really
	}
}


Class::~Class()
{
}
