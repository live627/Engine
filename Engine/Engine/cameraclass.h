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
class CameraClass : public GameObject
{
public:
	CameraClass(InputClass *);
	bool Initialize();
	void Frame();

	bool Render();
	void GetViewMatrix(DirectX::XMMATRIX &) const;

	void Save(std::ofstream &);
	void Load(std::ifstream &);

	void SetPosition(float, float, float);
	void SetRotation(float, float, float);

	DirectX::XMFLOAT3 GetPosition() const;
	DirectX::XMFLOAT3 GetRotation() const;

private:
	float m_positionX, m_positionY, m_positionZ;
	float m_rotationX, m_rotationY, m_rotationZ;
	DirectX::XMMATRIX m_viewMatrix;
	InputClass * m_Input;
};

#endif