////////////////////////////////////////////////////////////////////////////////
// Filename: cameraclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "cameraclass.h"


CameraClass::CameraClass(InputClass * p_Input)
{
	m_positionX = 0.0f;
	m_positionY = 0.0f;
	m_positionZ = 0.0f;

	m_rotationX = 0.0f;
	m_rotationY = 0.0f;
	m_rotationZ = 0.0f;

	m_Input = p_Input;
}


bool CameraClass::Initialize()
{
	SetPosition(0.0f, 0.0f, -1.0f);

	return Render();
}


void CameraClass::Frame()
{
	float x = 0, y = 0, z = -1;


	if (m_Input->IsKeyDown(VK_LEFT))
		m_positionX++;

	if (m_Input->IsKeyDown(VK_RIGHT))
		m_positionX--;

	if (m_Input->IsKeyDown(VK_UP))
		m_positionY--;

	if (m_Input->IsKeyDown(VK_DOWN))
		m_positionY++;
}


void CameraClass::SetPosition(float x, float y, float z)
{
	m_positionX = x;
	m_positionY = y;
	m_positionZ = z;
}


void CameraClass::SetRotation(float x, float y, float z)
{
	m_rotationX = x;
	m_rotationY = y;
	m_rotationZ = z;
}


DirectX::XMFLOAT3 CameraClass::GetPosition() const
{
	return DirectX::XMFLOAT3(m_positionX, m_positionY, m_positionZ);
}


DirectX::XMFLOAT3 CameraClass::GetRotation() const
{
	return DirectX::XMFLOAT3(m_rotationX, m_rotationY, m_rotationZ);
}


bool CameraClass::Render()
{
	DirectX::XMVECTOR
		Eye = DirectX::XMVectorSet(m_positionX, m_positionY, -1.0f, 0.0f),
		LookAt = DirectX::XMVectorSet(m_positionX, m_positionY, 0.0f, 0.0f),
		Up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	m_viewMatrix = DirectX::XMMatrixLookAtLH(Eye, LookAt, Up);

	return true;
}


void CameraClass::GetViewMatrix(DirectX::XMMATRIX& viewMatrix) const
{
	viewMatrix = m_viewMatrix;
}

void CameraClass::Save(std::ofstream& file)
{
	int16_t x = m_positionX;
	file.write(reinterpret_cast<const char*>(&x), sizeof x);

	x = m_positionY;
	file.write(reinterpret_cast<const char*>(&x), sizeof x);
}

void CameraClass::Load(std::ifstream& file)
{
	int16_t x = 0;
	file.read(reinterpret_cast<char*>(&x), sizeof x);
	m_positionX = x;

	file.read(reinterpret_cast<char*>(&x), sizeof x);
	m_positionY = x;
}
