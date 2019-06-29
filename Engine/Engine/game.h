#pragma once


//////////////
// INCLUDES //
////////////// 
#include <fstream>
#include <algorithm> 
#include <memory> 
#include <vector> 
#include <Windows.h>
#include <DirectXColors.h>


///////////////////////////////////////////////////////////////////////////////
// BinaryReader
///////////////////////////////////////////////////////////////////////////////
class BinaryReader
{
	std::istream &io;

public:
	BinaryReader(std::istream &io)
		: 
		io(io)
	{}

	std::string GetString()
	{
		std::vector<char> buf(Get<size_t>());
		io.read(&buf[0], buf.size());
		return std::string(buf.begin(), buf.end());
	}

	void Read(void *p, size_t size)
	{
		io.read(reinterpret_cast<char *>(p), size);
	}

	template<typename T>
	T Get()
	{
		T buf;
		io.read(reinterpret_cast<char *>(&buf), sizeof T);
		return buf;
	}
};


///////////////////////////////////////////////////////////////////////////////
// BinaryWriter
///////////////////////////////////////////////////////////////////////////////
class BinaryWriter
{
	std::ostream &io;

public:
	BinaryWriter(std::ostream &io)
		: 
		io(io)
	{}

	void WriteString(const std::string &str)
	{
		size_t len = str.size();
		Write(len);
		io.write(str.c_str(), len);
	}

	template<typename T>
	void Write(T buf)
	{
		io.write(reinterpret_cast<char *>(&buf), sizeof T);
	}
};


////////////////////////////////////////////////////////////////////////////////
// Class name: IGameObject
////////////////////////////////////////////////////////////////////////////////
class IGameObject
{
public:
	virtual ~IGameObject() {}
	virtual void Shutdown() {}
	virtual void Frame() = 0;
	virtual void Render(
		const DirectX::XMMATRIX &,
		const DirectX::XMMATRIX &,
		const DirectX::XMMATRIX &) {}
	virtual void RenderUI(
		const DirectX::XMMATRIX &,
		const DirectX::XMMATRIX &) {}
	virtual void Save(BinaryWriter &) {}
	virtual void Load(BinaryReader &) {}
	virtual void OnClick(const std::vector<bool>, POINT) {}
};


class Settings
{
public:
	enum GameMode { normal, sandbox };
	int money = 100000;
	int Ore;
	int OreConversionRate = 10;
	int OreConversionRateBoost = 1;
	int NumStars = 5;
	int Maintenance = 0;
	int drillCost = 10000;
	int MaintenanceRate = 30;
	int MaintCooldown = 30;
	GameMode gameMode = GameMode::normal;
};

class ui
{
public:
	// how much to scale a design that assumes 96-DPI pixels
	static double scaleX;
	static double scaleY;

	template<typename T>
	inline static constexpr T ScaleX(const T argX)
	{
		return static_cast<T>(argX * scaleX);
	}
};

namespace Geometry
{
	template<typename T>
	struct Rectangle
	{
		T X;
		T left;
		T Y;
		T Top;
		T Width;
		T Height;
		T Right;
		T Bottom;

		Rectangle(T x, T y, T width, T height)
			:
			X(x),
			left(x),
			Y(y),
			Top(y),
			Width(width),
			Height(height),
			Right(x + width),
			Bottom(y + height)
		{}

		bool Contains(T x, T y)
		{
			return X <= x && x < Right && Y <= y && y < Bottom;
		}

		bool Contains(DirectX::XMFLOAT3 value)
		{
			return Contains(value.x, value.y);
		}
	};

	template<typename T>
	struct ColoredRect
	{
		RECT rect;
		DirectX::XMFLOAT4 color;
		bool hidden = false;

		ColoredRect(Rectangle<T> rect)
			:
			//rect({ rect.left, rect.Top, rect.Right, rect.Bottom })
			rect({ rect.left, rect.Top, rect.Width, rect.Height })
		{}

		ColoredRect(Rectangle<T> rect, DirectX::XMFLOAT4 color)
			:
			//rect({ rect.left, rect.Top, rect.Right, rect.Bottom }),
			rect({ rect.left, rect.Top, rect.Width, rect.Height }),
			color(color)
		{}

		ColoredRect(T x, T y, T width, T height, DirectX::XMFLOAT4 color)
			:
			//rect({ x, y, x + width, y + height }),
			rect({ x, y, width, height }),
			color(color)
		{}

		ColoredRect(T x, T y, T width, T height, DirectX::XMFLOAT4 color, bool hidden)
			:
			//rect({ x, y, x + width, y + height }),
			rect({ x, y, width, height }),
			color(color),
			hidden(hidden)
		{}
	};
};

// Helper class for COM exceptions
class com_exception : public std::runtime_error
{
public:
	com_exception(const HRESULT hr, const char * msg)
		:
		std::runtime_error(msg),
		result(hr)
	{}

	virtual const char* what() const override
	{
		LPSTR errorText = NULL;
		FormatMessageA(
			// use system message tables to retrieve error text
			FORMAT_MESSAGE_FROM_SYSTEM
			// allocate buffer on local heap for error text
			| FORMAT_MESSAGE_ALLOCATE_BUFFER
			// Important! will fail otherwise, since we're not 
			// (and CANNOT) pass insertion parameters
			| FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,    // unused with FORMAT_MESSAGE_FROM_SYSTEM
			result,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPSTR)&errorText,  // output 
			0, // minimum size for output buffer
			NULL);   // arguments - see note 

		static char s_str[1064] = {};
		if (NULL != errorText)
		{
			sprintf_s(s_str, "Failure with HRESULT of 0X%08X:\n\n%s\n\n%s",
				static_cast<unsigned int>(result),
				errorText,
				std::runtime_error::what()
			);

			// release memory allocated by FormatMessage()
			LocalFree(errorText);
			errorText = NULL;
		}
		else
		{
			sprintf_s(s_str, "Failure with HRESULT of 0X%08X:\n\n%s",
				static_cast<unsigned int>(result),
				std::runtime_error::what()
			);
		}

		return s_str;
	}

private:
	HRESULT result;
};

// Helper utility converts D3D API failures into exceptions.
inline void ThrowIfFailed(const HRESULT hr, const char * msg)
{
	if (FAILED(hr))
		throw com_exception(hr, msg);
}

// snprintf with automatic string size measurement.
template<typename... Args>
auto FormatString(const char * fmt, Args... args)
{
	size_t sz = std::snprintf(nullptr, 0, fmt, args...);
	std::vector<char> buf(sz + 1); // note +1 for null terminator
	std::snprintf(&buf[0], buf.size(), fmt, args...);

	return std::move(buf);
}
class Formatting
{
public:
	/*add commas between groups of 3 digits with remainder on left side*/
	static auto CommaFormat(std::vector<char> & in)
	{
		int length = in.size();
		if (length > 3)
		{
			int i = length % 3, count = 0;
			for (char c : in)
				count += in[i] == '.';
			length = count == 0 ? length : count;
			if (i == 0)
				i = 3;
			in.reserve(length + length / 3);
			for (int inserted = 0; i < length + inserted; i += 4)
			{
				in.insert(in.begin() + i, ',');
				inserted++;
			}
		}
		return in;
	}
};

// Taken verbatim from DirectXColors.h - these use actual floats instead of vectors.
namespace Colors
{
	// Added by me
	XMGLOBALCONST DirectX::XMFLOAT4 MyBlack = { 0.000000000f, 0.000000000f, 0.000000000f, 0.400000000f };

	inline DirectX::XMFLOAT4 Lerp(const DirectX::XMFLOAT4 & c1, const DirectX::XMFLOAT4 & c2, float t)
	{
		using namespace DirectX;
		XMVECTOR C0 = XMLoadFloat4(&c1);
		XMVECTOR C1 = XMLoadFloat4(&c2);

		DirectX::XMFLOAT4 result;
		XMStoreFloat4(&result, XMVectorLerp(C0, C1, t));
		return result;
	}
	// Standard colors (Red/Green/Blue/Alpha)
	XMGLOBALCONST DirectX::XMFLOAT4 AliceBlue = { 0.941176534f, 0.972549081f, 1.000000000f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 AntiqueWhite = { 0.980392218f, 0.921568692f, 0.843137324f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 Aqua = { 0.000000000f, 1.000000000f, 1.000000000f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 Aquamarine = { 0.498039246f, 1.000000000f, 0.831372619f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 Azure = { 0.941176534f, 1.000000000f, 1.000000000f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 Beige = { 0.960784376f, 0.960784376f, 0.862745166f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 Bisque = { 1.000000000f, 0.894117713f, 0.768627524f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 Black = { 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 BlanchedAlmond = { 1.000000000f, 0.921568692f, 0.803921640f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 Blue = { 0.000000000f, 0.000000000f, 1.000000000f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 BlueViolet = { 0.541176498f, 0.168627456f, 0.886274576f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 Brown = { 0.647058845f, 0.164705887f, 0.164705887f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 BurlyWood = { 0.870588303f, 0.721568644f, 0.529411793f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 CadetBlue = { 0.372549027f, 0.619607866f, 0.627451003f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 Chartreuse = { 0.498039246f, 1.000000000f, 0.000000000f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 Chocolate = { 0.823529482f, 0.411764741f, 0.117647067f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 Coral = { 1.000000000f, 0.498039246f, 0.313725501f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 CornflowerBlue = { 0.392156899f, 0.584313750f, 0.929411829f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 Cornsilk = { 1.000000000f, 0.972549081f, 0.862745166f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 Crimson = { 0.862745166f, 0.078431375f, 0.235294133f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 Cyan = { 0.000000000f, 1.000000000f, 1.000000000f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 DarkBlue = { 0.000000000f, 0.000000000f, 0.545098066f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 DarkCyan = { 0.000000000f, 0.545098066f, 0.545098066f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 DarkGoldenrod = { 0.721568644f, 0.525490224f, 0.043137256f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 DarkGray = { 0.662745118f, 0.662745118f, 0.662745118f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 DarkGreen = { 0.000000000f, 0.392156899f, 0.000000000f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 DarkKhaki = { 0.741176486f, 0.717647076f, 0.419607878f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 DarkMagenta = { 0.545098066f, 0.000000000f, 0.545098066f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 DarkOliveGreen = { 0.333333343f, 0.419607878f, 0.184313729f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 DarkOrange = { 1.000000000f, 0.549019635f, 0.000000000f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 DarkOrchid = { 0.600000024f, 0.196078449f, 0.800000072f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 DarkRed = { 0.545098066f, 0.000000000f, 0.000000000f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 DarkSalmon = { 0.913725555f, 0.588235319f, 0.478431404f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 DarkSeaGreen = { 0.560784340f, 0.737254918f, 0.545098066f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 DarkSlateBlue = { 0.282352954f, 0.239215702f, 0.545098066f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 DarkSlateGray = { 0.184313729f, 0.309803933f, 0.309803933f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 DarkTurquoise = { 0.000000000f, 0.807843208f, 0.819607913f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 DarkViolet = { 0.580392182f, 0.000000000f, 0.827451050f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 DeepPink = { 1.000000000f, 0.078431375f, 0.576470613f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 DeepSkyBlue = { 0.000000000f, 0.749019623f, 1.000000000f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 DimGray = { 0.411764741f, 0.411764741f, 0.411764741f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 DodgerBlue = { 0.117647067f, 0.564705908f, 1.000000000f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 Firebrick = { 0.698039234f, 0.133333340f, 0.133333340f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 FloralWhite = { 1.000000000f, 0.980392218f, 0.941176534f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 ForestGreen = { 0.133333340f, 0.545098066f, 0.133333340f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 Fuchsia = { 1.000000000f, 0.000000000f, 1.000000000f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 Gainsboro = { 0.862745166f, 0.862745166f, 0.862745166f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 GhostWhite = { 0.972549081f, 0.972549081f, 1.000000000f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 Gold = { 1.000000000f, 0.843137324f, 0.000000000f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 Goldenrod = { 0.854902029f, 0.647058845f, 0.125490203f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 Gray = { 0.501960814f, 0.501960814f, 0.501960814f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 Green = { 0.000000000f, 0.501960814f, 0.000000000f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 GreenYellow = { 0.678431392f, 1.000000000f, 0.184313729f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 Honeydew = { 0.941176534f, 1.000000000f, 0.941176534f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 HotPink = { 1.000000000f, 0.411764741f, 0.705882370f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 IndianRed = { 0.803921640f, 0.360784322f, 0.360784322f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 Indigo = { 0.294117659f, 0.000000000f, 0.509803951f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 Ivory = { 1.000000000f, 1.000000000f, 0.941176534f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 Khaki = { 0.941176534f, 0.901960850f, 0.549019635f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 Lavender = { 0.901960850f, 0.901960850f, 0.980392218f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 LavenderBlush = { 1.000000000f, 0.941176534f, 0.960784376f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 LawnGreen = { 0.486274540f, 0.988235354f, 0.000000000f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 LemonChiffon = { 1.000000000f, 0.980392218f, 0.803921640f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 LightBlue = { 0.678431392f, 0.847058892f, 0.901960850f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 LightCoral = { 0.941176534f, 0.501960814f, 0.501960814f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 LightCyan = { 0.878431439f, 1.000000000f, 1.000000000f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 LightGoldenrodYellow = { 0.980392218f, 0.980392218f, 0.823529482f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 LightGreen = { 0.564705908f, 0.933333397f, 0.564705908f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 LightGray = { 0.827451050f, 0.827451050f, 0.827451050f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 LightPink = { 1.000000000f, 0.713725507f, 0.756862819f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 LightSalmon = { 1.000000000f, 0.627451003f, 0.478431404f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 LightSeaGreen = { 0.125490203f, 0.698039234f, 0.666666687f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 LightSkyBlue = { 0.529411793f, 0.807843208f, 0.980392218f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 LightSlateGray = { 0.466666698f, 0.533333361f, 0.600000024f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 LightSteelBlue = { 0.690196097f, 0.768627524f, 0.870588303f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 LightYellow = { 1.000000000f, 1.000000000f, 0.878431439f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 Lime = { 0.000000000f, 1.000000000f, 0.000000000f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 LimeGreen = { 0.196078449f, 0.803921640f, 0.196078449f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 Linen = { 0.980392218f, 0.941176534f, 0.901960850f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 Magenta = { 1.000000000f, 0.000000000f, 1.000000000f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 Maroon = { 0.501960814f, 0.000000000f, 0.000000000f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 MediumAquamarine = { 0.400000036f, 0.803921640f, 0.666666687f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 MediumBlue = { 0.000000000f, 0.000000000f, 0.803921640f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 MediumOrchid = { 0.729411781f, 0.333333343f, 0.827451050f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 MediumPurple = { 0.576470613f, 0.439215720f, 0.858823597f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 MediumSeaGreen = { 0.235294133f, 0.701960802f, 0.443137288f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 MediumSlateBlue = { 0.482352972f, 0.407843173f, 0.933333397f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 MediumSpringGreen = { 0.000000000f, 0.980392218f, 0.603921592f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 MediumTurquoise = { 0.282352954f, 0.819607913f, 0.800000072f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 MediumVioletRed = { 0.780392230f, 0.082352944f, 0.521568656f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 MidnightBlue = { 0.098039225f, 0.098039225f, 0.439215720f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 MintCream = { 0.960784376f, 1.000000000f, 0.980392218f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 MistyRose = { 1.000000000f, 0.894117713f, 0.882353008f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 Moccasin = { 1.000000000f, 0.894117713f, 0.709803939f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 NavajoWhite = { 1.000000000f, 0.870588303f, 0.678431392f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 Navy = { 0.000000000f, 0.000000000f, 0.501960814f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 OldLace = { 0.992156923f, 0.960784376f, 0.901960850f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 Olive = { 0.501960814f, 0.501960814f, 0.000000000f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 OliveDrab = { 0.419607878f, 0.556862772f, 0.137254909f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 Orange = { 1.000000000f, 0.647058845f, 0.000000000f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 OrangeRed = { 1.000000000f, 0.270588249f, 0.000000000f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 Orchid = { 0.854902029f, 0.439215720f, 0.839215755f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 PaleGoldenrod = { 0.933333397f, 0.909803987f, 0.666666687f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 PaleGreen = { 0.596078455f, 0.984313786f, 0.596078455f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 PaleTurquoise = { 0.686274529f, 0.933333397f, 0.933333397f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 PaleVioletRed = { 0.858823597f, 0.439215720f, 0.576470613f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 PapayaWhip = { 1.000000000f, 0.937254965f, 0.835294187f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 PeachPuff = { 1.000000000f, 0.854902029f, 0.725490212f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 Peru = { 0.803921640f, 0.521568656f, 0.247058839f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 Pink = { 1.000000000f, 0.752941251f, 0.796078503f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 Plum = { 0.866666734f, 0.627451003f, 0.866666734f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 PowderBlue = { 0.690196097f, 0.878431439f, 0.901960850f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 Purple = { 0.501960814f, 0.000000000f, 0.501960814f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 Red = { 1.000000000f, 0.000000000f, 0.000000000f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 RosyBrown = { 0.737254918f, 0.560784340f, 0.560784340f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 RoyalBlue = { 0.254901975f, 0.411764741f, 0.882353008f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 SaddleBrown = { 0.545098066f, 0.270588249f, 0.074509807f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 Salmon = { 0.980392218f, 0.501960814f, 0.447058856f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 SandyBrown = { 0.956862807f, 0.643137276f, 0.376470625f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 SeaGreen = { 0.180392161f, 0.545098066f, 0.341176480f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 SeaShell = { 1.000000000f, 0.960784376f, 0.933333397f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 Sienna = { 0.627451003f, 0.321568638f, 0.176470593f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 Silver = { 0.752941251f, 0.752941251f, 0.752941251f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 SkyBlue = { 0.529411793f, 0.807843208f, 0.921568692f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 SlateBlue = { 0.415686309f, 0.352941185f, 0.803921640f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 SlateGray = { 0.439215720f, 0.501960814f, 0.564705908f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 Snow = { 1.000000000f, 0.980392218f, 0.980392218f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 SpringGreen = { 0.000000000f, 1.000000000f, 0.498039246f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 SteelBlue = { 0.274509817f, 0.509803951f, 0.705882370f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 Tan = { 0.823529482f, 0.705882370f, 0.549019635f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 Teal = { 0.000000000f, 0.501960814f, 0.501960814f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 Thistle = { 0.847058892f, 0.749019623f, 0.847058892f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 Tomato = { 1.000000000f, 0.388235331f, 0.278431386f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 Transparent = { 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 Turquoise = { 0.250980407f, 0.878431439f, 0.815686345f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 Violet = { 0.933333397f, 0.509803951f, 0.933333397f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 Wheat = { 0.960784376f, 0.870588303f, 0.701960802f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 White = { 1.000000000f, 1.000000000f, 1.000000000f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 WhiteSmoke = { 0.960784376f, 0.960784376f, 0.960784376f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 Yellow = { 1.000000000f, 1.000000000f, 0.000000000f, 1.000000000f };
	XMGLOBALCONST DirectX::XMFLOAT4 YellowGreen = { 0.603921592f, 0.803921640f, 0.196078449f, 1.000000000f };
}; // namespace Colors