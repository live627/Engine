////////////////////////////////////////////////////////////////////////////////
// Filename: cameraclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _CAMERACLASS_H_
#define _CAMERACLASS_H_


//////////////
// INCLUDES //
//////////////
#include <DirectXMath.h>
#include <iostream>


///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "game.h"
#include "inputclass.h"


////////////////////////////////////////////////////////////////////////////////
// Class name: CameraClass
////////////////////////////////////////////////////////////////////////////////
class CameraClass : public IGameObject
{
public:
	CameraClass(const CameraClass &) = delete;
	CameraClass & operator=(const CameraClass &) = delete;
	CameraClass(InputClass * p_Input, int screenWidth, int screenHeight)
		:
		m_Input(p_Input),
		m_screenWidth(screenWidth),
		m_screenHeight(screenHeight)
	{
		SetPosition(0.0f, 0.0f, -1.0f);
		Render();
	}
	void Frame()
	{
		if (m_Input->IsKeyDown(VK_LEFT))
			m_position.x++;

		if (m_Input->IsKeyDown(VK_RIGHT))
			m_position.x--;

		if (m_Input->IsKeyDown(VK_UP))
			m_position.y--;

		if (m_Input->IsKeyDown(VK_DOWN))
			m_position.y++;

		if (m_Input->IsKeyDown(VK_RBUTTON))
		{
			POINT temp = m_Input->GetMousePositionDelta();
			m_position.x -= temp.x;
			m_position.y += temp.y;
		}
	}
	void Render()
	{
		DirectX::XMVECTOR
			Eye = DirectX::XMVectorSet(m_position.x, m_position.y, -1.0f, 0.0f),
			LookAt = DirectX::XMVectorSet(m_position.x, m_position.y, 0.0f, 0.0f),
			Up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

		m_viewMatrix = DirectX::XMMatrixLookAtLH(Eye, LookAt, Up);
	}

	void SetPosition(float x, float y, float z) { m_position = DirectX::XMFLOAT3(x, y, z); }
	void SetRotation(float x, float y, float z) { m_rotation = DirectX::XMFLOAT3(x, y, z); }

	void Save(BinaryWriter & writer)
	{
		writer.Write(m_position.x);
		writer.Write(m_position.y);
	}

	void Load(BinaryReader & reader)
	{
		m_position.x = reader.Get<float>();
		m_position.y = reader.Get<float>();
	}

	DirectX::XMMATRIX GetViewMatrix() const { return m_viewMatrix; }
	DirectX::XMFLOAT3 GetPosition() const { return m_position; }
	DirectX::XMFLOAT3 GetRotation() const { return m_rotation; }

	void ResizeBuffers(int screenWidth, int screenHeight,
		const DirectX::XMMATRIX & worldMatrix,
		//const DirectX::XMMATRIX & orthoMatrix,
		const DirectX::XMMATRIX & projectionMatrix)
	{
		m_screenWidth = screenWidth;
		m_screenHeight = screenHeight;
		this->worldMatrix = worldMatrix;
		this->projectionMatrix = projectionMatrix;
	}
	
	DirectX::XMFLOAT3 && ToWorldPosition(const DirectX::XMVECTOR & p_position) const
	{
		DirectX::XMVECTOR orig = DirectX::XMVector3Unproject(
			p_position,
			0,
			0,
			m_screenWidth,
			m_screenHeight,
			0.1f,
			1,
			projectionMatrix,
			m_viewMatrix,
			worldMatrix);
		DirectX::XMFLOAT3 v2F;    //the float where we copy the v2 vector members
		DirectX::XMStoreFloat3(&v2F, orig);
	}

	DirectX::XMFLOAT3 && ToWorldPosition(const DirectX::XMFLOAT3 & p_position) const
	{
		return ToWorldPosition(DirectX::XMLoadFloat3(&p_position));
	}

	DirectX::XMFLOAT3 && ToWorldPosition(const POINT & p_position) const
	{
		DirectX::XMMATRIX projection = projectionMatrix;
		DirectX::XMMATRIX view = m_viewMatrix;

		DirectX::XMMATRIX invProjectionView = DirectX::XMMatrixInverse(
			&DirectX::XMMatrixDeterminant(view *  projection), (view *  projection));
		//invViewProjection = invView * invProjection;

		float x = (((2.0f * p_position.x) / m_screenWidth) - 1);
		float y = -(((2.0f * p_position.y) / m_screenHeight) - 1);

		DirectX::XMVECTOR mousePosition = DirectX::XMVectorSet(x, y, 1.0f, 0.0f);

		auto mouseInWorldSpace = DirectX::XMVector3Transform(mousePosition, invProjectionView);
		DirectX::XMFLOAT3 v2F;    //the float where we copy the v2 vector members
		DirectX::XMStoreFloat3(&v2F, mouseInWorldSpace);

		return std::move(v2F);
		//return ToWorldPosition(DirectX::XMVectorSet(p_position.x, p_position.y, 0, 0));
	}

	DirectX::XMFLOAT3 && ToWorldPosition(int x, int y) const
	{
		return ToWorldPosition(DirectX::XMVectorSet(x, y, 0, 0));
	}

private:
	DirectX::XMFLOAT3 m_position, m_rotation;
	DirectX::XMMATRIX m_viewMatrix;
	InputClass * m_Input;
	DirectX::XMMATRIX worldMatrix, projectionMatrix;
	int m_screenWidth, m_screenHeight;
};

#endif