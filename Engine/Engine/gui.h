#pragma once

#include <DirectXColors.h>
#include "game.h"
#include "textclass.h"

	class BasicPanel
	{
	protected:
		RectangleI rect;
		DirectX::XMFLOAT4 backgroundColor;
		DirectX::XMFLOAT4 borderColor;

		BasicPanel(RectangleI rect, DirectX::XMFLOAT4 backgroundColor, DirectX::XMFLOAT4 borderColor)
			:
			rect(rect),
			backgroundColor(backgroundColor),
			borderColor(borderColor)
		{
			BuildPanel(rect, backgroundColor, borderColor);
		}

		void BuildPanel(RectangleI rect, DirectX::XMFLOAT4 backgroundColor)
		{
			sprites.emplace_back(rect, backgroundColor);
		}

		void BuildPanel(RectangleI rect, DirectX::XMFLOAT4 backgroundColor, DirectX::XMFLOAT4 borderColor)
		{
			sprites.emplace_back(rect, backgroundColor);
			sprites.emplace_back(rect.left - 1, rect.Top, 1, rect.Height, borderColor); // left
			sprites.emplace_back(rect.Right, rect.Top, 1, rect.Height, borderColor); // Right
			sprites.emplace_back(rect.left, rect.Top, rect.Width, 1, borderColor); // Top
			sprites.emplace_back(rect.left, rect.Bottom, rect.Width, 1, borderColor); // Bottom
		}

		void BuildTopPanel(RectangleI rect, DirectX::XMFLOAT4 backgroundColor,
			DirectX::XMFLOAT4 borderColor, int height, int padding)
		{
			sprites.emplace_back(
				rect.X, rect.Y + 1, rect.Width,
				padding + height + padding, backgroundColor);
			sprites.emplace_back(
				rect.X, rect.Y + padding + height + padding,
				rect.Width, 1, borderColor);
		}

		void BuildBottomPanel(RectangleI rect, DirectX::XMFLOAT4 backgroundColor,
			DirectX::XMFLOAT4 borderColor, int height)
		{
			sprites.emplace_back(
				rect.X, rect.Y + rect.Height - (ui::ScaleX(20) + height), rect.Width,
				ui::ScaleX(20) + height, backgroundColor);
			sprites.emplace_back(
				rect.X, rect.Y + rect.Height - (ui::ScaleX(20) + height),
				rect.Width, 1, borderColor);
		}

	public:
		bool IsMouseOver(DirectX::XMFLOAT3 transformedMousePosition)
		{
			return rect.Contains(transformedMousePosition);
		}

		virtual void Frame(DirectX::XMFLOAT3 &) = 0;
		virtual std::vector<ColoredRect> GetSprites() = 0;
		std::vector<ColoredRect> sprites;
	};

	class Button : BasicPanel
	{
		DirectX::XMFLOAT4 color = Colors::White;
		DirectX::XMFLOAT4 hoverColor = Colors::Wheat;

	public:
		Button(RectangleI rect, DirectX::XMFLOAT4 backgroundColor,
			DirectX::XMFLOAT4 borderColor, const char * text)
			:
			BasicPanel(rect, backgroundColor, borderColor)
		{}

		void Frame(DirectX::XMFLOAT3 transformedMousePosition)
		{
			if (IsMouseOver(transformedMousePosition))
				sprites[0].color = hoverColor;
			else
				sprites[0].color = color;
		}
	};

	class ListView : BasicPanel
	{
		DirectX::XMFLOAT4 color = Colors::White;
		DirectX::XMFLOAT4 hoverColor = Colors::Wheat;
		std::vector<RectangleI> rc;
		std::vector<std::tuple<const char *, float, float>> txtPos;
		int selectedIndex = INT16_MAX, totalItemHeight = 0;
		int scrollbarHeight;
		float scrollTop;
		float clampedScrollTop;
		float matrixScrollTop;
		float clampedMatrixScrollTop;
		bool scrollDrag;

	public:
		ListView(RectangleI rect, DirectX::XMFLOAT4 backgroundColor,
			DirectX::XMFLOAT4 borderColor)
			:
			BasicPanel(rect, backgroundColor, borderColor)
		{}

		std::vector<ColoredRect> GetSprites()
		{
			return sprites;
		}

		auto UpdateItems(std::vector<const char *> items, Fonts * fonts)
		{
			int scrollbarWidth = ui::ScaleX(18);
			for (int i = 0; i < items.size(); i++)
			{
				auto size = fonts->MeasureString(items[i]);
				int itemHeight = size.y + size.y / 2;
				rc.emplace_back(rect.left, rect.Top + totalItemHeight, rect.Width - scrollbarWidth, itemHeight);
				totalItemHeight += itemHeight;
				float y1 = rc[i].Top + ((rc[i].Height - size.y) / 2.0f);
				txtPos.emplace_back(std::make_tuple(items[i], rc[i].left + 10.0f, y1));
			}
			for (int i = 0; i < items.size(); i++)
			{
				int w = rc[i].Width;
				if (totalItemHeight < rect.Height)
					w = rect.Width;

				sprites.emplace_back(rc[i].left, rc[i].Top, w, rc[i].Height, Colors::WhiteSmoke);
				sprites.emplace_back(rc[i].left, rc[i].Top, w, 1, Colors::Lerp(Colors::AntiqueWhite, Colors::DarkGray, 0.5));
			}

			/*
			sprites2.Clear();
			sprites2.Add(new Sprite(pixel, Rect, DirectX::XMFLOAT4.Lerp(DirectX::XMFLOAT4.AntiqueWhite, DirectX::XMFLOAT4.White, 0.5f)));
			scrollbarHeight = Math.Max(scrollbarWidth, Math.Min(Rect.Height, (int)MathHelper.Lerp(1, Rect.Height, Rect.Height / TotalItemHeight)));
			sprites2.emplace_back(Rect.Right - scrollbarWidth, Rect.Top, scrollbarWidth, Rect.Height), DirectX::XMFLOAT4.Lerp(DirectX::XMFLOAT4.AntiqueWhite, DirectX::XMFLOAT4.LightGray, 0.5f)));
			sprites2.emplace_back(Rect.Right - scrollbarWidth, Rect.Top, scrollbarWidth, scrollbarHeight), DirectX::XMFLOAT4.DarkGray));
			*/

			return &txtPos;
		}

		void Frame(DirectX::XMFLOAT3 & transformedMousePosition)
		{
			for (int i = 0; i < rc.size(); i++)
			{
				// Is an item just now clicked? Select it!
				if (
					rc[i].Contains(transformedMousePosition)
					&& rect.Contains(transformedMousePosition)
					)
					selectedIndex = i;

				int idx = 4 + 2 * i;
				if (i != selectedIndex)
					sprites[idx].color = Colors::White;
				else
					sprites[idx].color = Colors::Wheat;
			}

			/*
			// Scrolling logic shall be skipped if not enough items in list.
			if (TotalItemHeight < Rect.Height)
				return;

			// Dragging the scrollbar
			if (
				sprites2[2].RectangleI.Value.Contains(inputManager.MousePosition.ToVector2())
				&& inputManager.IsHeld(MouseInput.leftButton)
				)
				scrollDrag = true;

			if (inputManager.JustReleased(MouseInput.leftButton))
				scrollDrag = false;

			if (scrollDrag)
			{
				var pos = inputManager.PreviousMouseState.Position.Y - inputManager.CurrentMouseState.Position.Y;
				matrixScrollTop += pos;
				scrollTop += pos * (Rect.Height / TotalItemHeight);
			}

			matrixScrollTop += Math.Sign(inputManager.MouseScroll) * 20;
			clampedMatrixScrollTop = MathHelper.Clamp(matrixScrollTop, Rect.Height - TotalItemHeight, 0);
			itemsMatrix.Translation = new Vector3(0, clampedMatrixScrollTop, 0);
			matrixScrollTop = clampedMatrixScrollTop;
			scrollTop += Math.Sign(inputManager.MouseScroll) * (20 * (Rect.Height / TotalItemHeight));
			clampedScrollTop = MathHelper.Clamp(scrollTop, (Rect.Height - TotalItemHeight) * Rect.Height / TotalItemHeight, 0);
			sprites2[2].Position = new Vector2(sprites2[2].Position.X, Rect.Top - clampedScrollTop);
			scrollTop = clampedScrollTop;
			*/
		}
	};