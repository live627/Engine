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
	CameraClass(InputClass * p_Input)
		:
		m_Input(p_Input)
	{
		SetPosition(0.0f, 0.0f, -1.0f);
		Render();
	}
	bool Initialize() { return true; }
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
	bool Render()
	{
		DirectX::XMVECTOR
			Eye = DirectX::XMVectorSet(m_position.x, m_position.y, -1.0f, 0.0f),
			LookAt = DirectX::XMVectorSet(m_position.x, m_position.y, 0.0f, 0.0f),
			Up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

		m_viewMatrix = DirectX::XMMatrixLookAtLH(Eye, LookAt, Up);

		return true;
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

private:
	DirectX::XMFLOAT3 m_position, m_rotation;
	DirectX::XMMATRIX m_viewMatrix;
	InputClass * m_Input;
};

#endif