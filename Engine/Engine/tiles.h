#pragma once


///////////////////////
// INCLUDES //
///////////////////////
#include <DirectXColors.h>
#include <wrl\client.h>
#include <numeric>
#include <algorithm>
#include <random>


///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "cameraclass.h"
#include "textureclass.h"
#include "fontshaderclass.h"
#include "LargeBitmap.h"
#include "game.h"


////////////////////////////////////////////////////////////////////////////////
// Class name: Tile
////////////////////////////////////////////////////////////////////////////////

class Tiles : public IGameObject
{
	class Tile
	{
	public:
		Tile(const Geometry::Rectangle<int> position, uint8_t texidx, int index) : position(position), texidx(texidx), index(index)
		{
		}

		Geometry::Rectangle<int> position;
		uint8_t texidx;
		int index;
	};

public:
	Tiles(
		ID3D11Device * p_device,
		ID3D11DeviceContext * p_deviceContext,
		ShaderClass * p_FontShader,
		CameraClass * p_Camera,
		Settings * p_settings,
		int screenWidth,
		int screenHeight,
		const int width,
		const int height,
		const int chanceToStartAlive = 44,
		const int smoothingIterations = 4,
		const int octaves = 8,
		const double freq = 8,
		std::uint32_t seed = std::default_random_engine::default_seed)
		:
		m_device(p_device),
		m_deviceContext(p_deviceContext),
		m_Bitmap(p_device, p_deviceContext, p_FontShader, screenWidth, screenHeight, "data/sprite.dds"),
		m_Camera(p_Camera),
		m_settings(p_settings),
		m_screenWidth(screenWidth),
		m_screenHeight(screenHeight),
		width(width),
		height(height),
		size(height * width),
		cellular(width, height, chanceToStartAlive, smoothingIterations, seed),
		perlin(width, height, octaves, freq, seed),
		textureMap({
			{ 0, 0, 128, 128 },
			{ 0, 128, 128, 128 },
			{ 128, 128, 128, 128 },
			{ 256, 0, 128, 128 },
			{ 0, 384, 128 * 3, 128 },
			{ 128 * 7, 128 * 5, 128 * 3, 128 },
			{ 128 * 11, 128 * 5, 128 * 3, 128 },
			{ 0, 128 * 11, 128 * 5, 128 },
			{ 128 * 4, 128 * 5, 128 * 2, 128 },
			{ 0, 128 * 5, 128 * 3, 128 },
			{ 128 * 6, 128 * 11, 128 * 5, 128 },
			{ 128 * 15, 128, 128, 128 * 2 }, // drill well
			{ 128 * 15, 128 * 4 - 24, 128, 128 }, // drill bit
			{ 128 * 12, 128, 128 * 3, 128 }, // reception pod
			{ 128 * 15, 128 * 2, 128, 128 }, // drill step
		})
	{
		TileSize = 128, worldwidth = width * TileSize, worldheight = height * TileSize;
		m_Camera->SetPosition(worldwidth / 2.0f, 0.0f, -1.0f);
		LoadTiles("data\\tiles.dat");
	}
	void LoadTiles(const char *);
	Tile * TileFromWorldPoint(DirectX::XMFLOAT3 &);
	void OnClick(const std::vector<bool>, POINT);
	virtual void Frame() {};
	void Render(const DirectX::XMMATRIX &, const DirectX::XMMATRIX &, const DirectX::XMMATRIX &);
	Tile * GetTile(int idx) { return &m_tiles[idx]; }
	void Save(BinaryWriter &);
	void Load(BinaryReader &);

private:
	ID3D11Device * m_device;
	ID3D11DeviceContext * m_deviceContext;
	int m_screenWidth, m_screenHeight, width, height, size,
		TileSize, worldwidth, worldheight;
	Spritemap m_Bitmap;
	CameraClass * m_Camera;
	Settings * m_settings;
	std::vector<Tile> m_tiles;
	std::vector<RECT> textureMap;

	class Cellular
	{
		std::unique_ptr<bool[]> map;
		int width, height, size, chanceToStartAlive;
		std::default_random_engine generator;
		std::uniform_int_distribution<int> distribution;
	public:
		Cellular(
			const int & width,
			const int & height,
			const int & chanceToStartAlive = 44,
			const int & smoothingIterations = 4,
			const unsigned int & seed = std::default_random_engine::default_seed)
			:
			width(width),
			height(height),
			size(height * width),
			chanceToStartAlive(chanceToStartAlive),
			generator(seed),
			distribution(0, 100)
		{
			map = std::make_unique<bool[]>(size);

			Generate();
			for (int k = 0; k <= smoothingIterations; k++)
				Smooth();
		}
		auto GetMap() const { return map.get(); }

	private:
		inline void Generate()
		{
			for (int index = 0; index < size; index++)
				map[index] = distribution(generator) < chanceToStartAlive;
		}
		// Iterates through every tile in the map and decides if needs to be born, die, or remain unchanged
		inline void Smooth()
		{
			for (int index = 0; index < size; index++)
			{
				int newVal = countAliveNeighbours(index);
				if (map[index])
					map[index] = !(newVal < 3);
				else
					map[index] = newVal > 4;
			}
		}
		// Counts the number of "alive" cells around the target cell
		inline int8_t countAliveNeighbours(const int & index) const
		{
			int8_t count = 0;
			int x = index / width, y = index % width;
			for (int i = -1; i < 2; i++)
			{
				for (int j = -1; j < 2; j++)
				{
					int neighbour = (x + i) * width + (y + j);

					/*
					Count the neighbour as "alive" if:
					- it is within the map boundaries
					- was already deemed "alive"
					- is not the target cell
					*/
					if ((IsInBounds(neighbour) && map[neighbour]) && !(i == 0 && j == 0))
						count++;
				}
			}
			return count;
		}
		// Determines whether a cell is within the map
		inline constexpr bool IsInBounds(const int & index) const
		{
			return index >= 0 && index < size;
		}
	};

	class PerlinNoise
	{
	public:
		PerlinNoise(
			const int width,
			const int height,
			const int octaves,
			const double freq,
			std::uint32_t seed = std::default_random_engine::default_seed)
			:
			width(width),
			height(height),
			size(height * width),
			octaves(octaves),
			fx(width / freq),
			fy(height / freq)
		{
			map = std::make_unique<double[]>(size);
			std::iota(std::begin(p), std::begin(p) + 256, 0);
			std::shuffle(std::begin(p), std::begin(p) + 256, std::default_random_engine(seed));
			std::iota(std::begin(p) + 256, std::end(p), 0);

			for (int index = 0; index < size; index++)
				map[index] = OctaveNoise((index / width) / fx, (index % width) / fy, 0.0);
		}
		auto GetMap() const { return map.get(); }
		friend class Tiles;

	private:
		std::int32_t p[512];
		int width, height, size, octaves;
		double fx, fy;
		std::unique_ptr<double[]> map;

		static inline constexpr double Fade(const double t) noexcept
		{
			return t * t * t * (t * (t * 6 - 15) + 10);
		}

		static inline constexpr double Lerp(const double t, const double a, const double b) noexcept
		{
			return a + t * (b - a);
		}

		static inline constexpr double Grad(std::int32_t hash, const double x, const double y, const double z) noexcept
		{
			const std::int32_t h = hash & 15;
			const double u = h < 8 ? x : y;
			const double v = h < 4 ? y : h == 12 || h == 14 ? x : z;
			return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
		}

		inline double Generate(double x, double y, double z) const
		{
			const std::int32_t X = static_cast<std::int32_t>(std::floor(x)) & 255;
			const std::int32_t Y = static_cast<std::int32_t>(std::floor(y)) & 255;
			const std::int32_t Z = static_cast<std::int32_t>(std::floor(z)) & 255;

			x -= std::floor(x);
			y -= std::floor(y);
			z -= std::floor(z);

			const double u = Fade(x);
			const double v = Fade(y);
			const double w = Fade(z);

			const std::int32_t A = p[X] + Y, AA = p[A] + Z, AB = p[A + 1] + Z;
			const std::int32_t B = p[X + 1] + Y, BA = p[B] + Z, BB = p[B + 1] + Z;

			return Lerp(w, Lerp(v, Lerp(u, Grad(p[AA], x, y, z),
				Grad(p[BA], x - 1, y, z)),
				Lerp(u, Grad(p[AB], x, y - 1, z),
					Grad(p[BB], x - 1, y - 1, z))),
				Lerp(v, Lerp(u, Grad(p[AA + 1], x, y, z - 1),
					Grad(p[BA + 1], x - 1, y, z - 1)),
					Lerp(u, Grad(p[AB + 1], x, y - 1, z - 1),
						Grad(p[BB + 1], x - 1, y - 1, z - 1))));
		}

		double OctaveNoise(double x, double y, double z) const
		{
			double result = 0.0;
			double amp = 1.0;

			for (std::int32_t i = 0; i < octaves; ++i)
			{
				result += Generate(x, y, z) * amp;
				x *= 2.0;
				y *= 2.0;
				z *= 2.0;
				amp *= 0.5;
			}

			return result * 0.5 + 0.5;
		}
	};

	Cellular cellular;
	PerlinNoise perlin;
};