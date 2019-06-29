#pragma once

#include <DirectXColors.h>
#include <numeric>
#include "fontmanager.h"
#include "fontshaderclass.h"

////////////////////////////////////////////////////////////////////////////////
// Class name: LargeBitmap
////////////////////////////////////////////////////////////////////////////////
class LargeBitmap
{
private:

public:
	LargeBitmap(ID3D11Device *, ID3D11DeviceContext *, ShaderClass *, int, int, const char *);
	LargeBitmap(ID3D11Device *, ID3D11DeviceContext *, ShaderClass *, int, int);
	void UpdateColoredRects(const std::vector<Geometry::ColoredRect<int>> &&);
	void UpdateColoredRect(int, const Geometry::ColoredRect<int> &);
	void UpdateColoredRect(int, RECT);
	void UpdateColoredRect(int, bool);
	void Render(const DirectX::XMMATRIX &, const DirectX::XMMATRIX &, const DirectX::XMMATRIX &);
	void ResizeBuffers(int, int);

protected:
	void CreateBuffers();
	void UpdateBuffers();
	virtual void BuildVertexArray(void *);
	virtual void RenderBuffers();
	virtual size_t GetVertexCount();
	virtual size_t GetIndexCount();
	virtual std::vector<unsigned long> BuildIndexArray();

	ID3D11Device * device;
	ID3D11DeviceContext * deviceContext;
	ShaderClass * m_FontShader;
	int m_screenWidth, m_screenHeight;
	std::vector<Geometry::ColoredRect<int>> m_rects;
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer, indexBuffer;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_texture;
	size_t vertexCount, indexCount;
};

////////////////////////////////////////////////////////////////////////////////
// Class name: Spritemap
////////////////////////////////////////////////////////////////////////////////
class Spritemap : public LargeBitmap
{
public:
	Spritemap(ID3D11Device *, ID3D11DeviceContext *, ShaderClass *, int, int, const char *);
	void UpdateUvRects(const std::vector<RECT> &&);
	void SetRectUvMap(const std::vector<int> &&);
	void UpdateUvRectMap(int, int);

private:
	std::vector<RECT> m_uvrects;
	std::vector<int> m_uvrectmap;
	void BuildVertexArray(void *) override;
};

////////////////////////////////////////////////////////////////////////////////
// Class name: PieChart
////////////////////////////////////////////////////////////////////////////////
class PieChart : public LargeBitmap
{
public:
	PieChart(ID3D11Device *, ID3D11DeviceContext *, ShaderClass *, int, int);
	void MakeChart(POINT, std::vector<float>);

private:
	void BuildVertexArray(void *) override;
	void RenderBuffers();
	size_t GetVertexCount() override;
	size_t GetIndexCount() override;
	std::vector<unsigned long> BuildIndexArray();

	double Lerp(double a, double b, double t)
	{
		return a + t * (b - a);
	}
};