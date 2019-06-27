////////////////////////////////////////////////////////////////////////////////
// Filename: tiles.cpp
////////////////////////////////////////////////////////////////////////////////
#include "tiles.h"


void Tiles::LoadTiles(const char* filename)
{
	try
	{
		//std::ifstream file(filename, std::ios::binary);
		//file.exceptions(std::fstream::failbit | std::fstream::badbit);
		//BinaryReader reader(file);
		int32_t numTiles = 1;//reader.Get<int32_t>();
		m_tiles.reserve(numTiles);
		std::vector<ColoredRect> coloredRects;
		coloredRects.reserve(size);
		std::vector<int> uvrectmap;
		uvrectmap.reserve(size);
	 
		for (int i = 0; i < size; i++)
		{
			try
			{
				//int32_t tileLength = reader.Get<int32_t>();
				//auto buffer = std::make_unique<std::byte[]>(tileLength);
				//reader.Read(buffer.get(), tileLength);
				int x = (i / width), y = (i % height);
				byte mappedTexture = 0;
				RectangleI rectangle(x * TileSize, y * TileSize, TileSize, TileSize);

				// the sky
				if (y < 6)
					mappedTexture = 255;

				// drill bit
				if (x == width / 2)
				{
					if (y < 7)
						mappedTexture = 255;
					if (y == 7)
					{
						//	drillBitIndex = i;
						mappedTexture = 12;
						rectangle.Bottom = 128;
					}

					// drill well
					if (y < 6)
						mappedTexture = 255;
					if (y == 5)
					{
						mappedTexture = 11;
						rectangle.Bottom = 128 * 2;
					}
				}
				// reception pod
				if (y == 5)
				{
					if (x == ((width / 2) - 3))
					{
						mappedTexture = 13;
						rectangle.Right = 128 * 3;
					}
					if (x == ((width / 2) - 2) || x == ((width / 2) - 1))
						mappedTexture = 255;
				}
				m_tiles.emplace_back(rectangle, mappedTexture, i);
				coloredRects.emplace_back(rectangle);
				uvrectmap.emplace_back(mappedTexture);
			}
			catch (std::exception & e)
			{
				throw std::runtime_error(
					FormatString(
						"%s\n\nCould not load tile %d",
						e.what(), i
					).data()
				);
			}
		}
		m_Bitmap.SetRectUvMap(std::move(uvrectmap));
		m_Bitmap.UpdateUvRects(std::move(textureMap));
		m_Bitmap.UpdateColoredRects(std::move(coloredRects));
	}
	catch (std::exception & e)
	{
		throw std::runtime_error(
			FormatString(
				"%s\n\nCould not load tiles",
				e.what()
			).data()
		);
	}
}


Tiles::Tile * Tiles::TileFromWorldPoint(DirectX::XMFLOAT3 & p)
{
	for (int index = 0; index < size; index++)
	{
		auto rect = m_tiles[index].position;
		// Calculate the screen coordinates of the bitmap.
		float
			left = (float)rect.left - (float)(m_screenWidth / 2),
			right = left + (float)rect.Width,
			top = (float)(m_screenHeight / 2) - (float)rect.Top,
			bottom = top - (float)rect.Height;
		if (p.x >= left && p.x < right && p.y <= top && p.y > bottom && m_tiles[index].texidx != 255)
			return &m_tiles[index];
	}
	return nullptr;
}


void Tiles::OnClick(const std::vector<bool> keys, POINT p)
{
	if (keys[VK_LBUTTON])
	{
		auto point = m_Camera->ToWorldPosition(p);
		auto tile = TileFromWorldPoint(point);
		if (tile == nullptr)
			return;
		auto rect = tile->position;
		if (tile->texidx != 255)
			OutputDebugStringA(FormatString("idx %d", tile->index).data());
		switch (tile->texidx)
		{
			case 0:
				tile->texidx = 3;
				break;

			case 12:
				// drill bit was clicked
				int x = tile->index / width, y = tile->index % height;
				int neighbour = x * height + (y + 1);
				int cost = m_settings->drillCost * (y - 6);
				if (m_settings->money < m_settings->drillCost * (y - 7))
					return;
				//	m_DialogBox.Show(Strings.NotEnoughMoney,
				//		FormatString("Drilling down a level requires more money. You only have {0} out of the required {1}",
				//		(m_settings->Money / 100).ToString("C"),
				//			(cost / 100).ToString("C")), new[] { "OK" }, new Action[]{ game.dialogBox.Hide });
				else
				{
					m_settings->money -= cost;
					m_tiles[neighbour].texidx = 12;
					tile->texidx = 14;
					m_Bitmap.UpdateUvRectMap(neighbour, m_tiles[neighbour].texidx);
				}
				break;
		}
		m_Bitmap.UpdateUvRectMap(tile->index, tile->texidx);
		m_Bitmap.UpdateColoredRect(tile->index, tile->position);
	}
}


void Tiles::Render(
	const DirectX::XMMATRIX & worldMatrix,
	const DirectX::XMMATRIX & orthoMatrix,
	const DirectX::XMMATRIX & baseViewMatrix)
{
	m_Bitmap.Render(worldMatrix, orthoMatrix, baseViewMatrix);
}

void Tiles::Save(BinaryWriter & writer)
{
	writer.Write(size);
	writer.Write(width);
	writer.Write(height);
	for (int index = 0; index < size; index++)
		writer.Write(m_tiles[index].texidx);
}

void Tiles::Load(BinaryReader & reader)
{
	size = reader.Get<int>();
	width = reader.Get<int>();
	height = reader.Get<int>();
	std::vector<uint8_t> data(size);
	reader.Read(data.data(), data.capacity());
	std::vector<ColoredRect> coloredRects;
	coloredRects.reserve(size);
	std::vector<int> uvrectmap;
	uvrectmap.reserve(size);
	m_tiles.clear();
	for (int index = 0; index < size; index++)
	{
		int x = (index / width) * TileSize, y = (index % height) * TileSize;
		int v = data[index] == 255 ? 0 : data[index];
		uint8_t mappedTexture = data[index];
		RectangleI rectangle(x, y, textureMap[v].right, textureMap[v].bottom);
		m_tiles.emplace_back(rectangle, mappedTexture, index);
		coloredRects.emplace_back(rectangle);
		uvrectmap.emplace_back(mappedTexture);
	}
	m_Bitmap.SetRectUvMap(std::move(uvrectmap));
	m_Bitmap.UpdateUvRects(std::move(textureMap));
	m_Bitmap.UpdateColoredRects(std::move(coloredRects));
}
