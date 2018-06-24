////////////////////////////////////////////////////////////////////////////////
// Filename: cameraclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _CAMERACLASS_H_
#define _CAMERACLASS_H_


//////////////
// INCLUDES //
//////////////
#include <d3d10_1.h>
#include <d3dx10math.h>
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
	void GetViewMatrix(D3DXMATRIX&);

	void Save(std::ofstream&);
	void Load(std::ifstream&);

	void SetPosition(float, float, float);
	void SetRotation(float, float, float);

	D3DXVECTOR3 GetPosition();
	D3DXVECTOR3 GetRotation();

private:
	float m_positionX, m_positionY, m_positionZ;
	float m_rotationX, m_rotationY, m_rotationZ;
	D3DXMATRIX m_viewMatrix;
	InputClass* m_Input;
};

#endif