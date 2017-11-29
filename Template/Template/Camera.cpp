#include "Camera.h"

Camera::Camera()
{
	_pos   = D3DXVECTOR3(0.0f, 7.0f, 0.0f); //determine height of player by changing y value
	_right = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
	_up    = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	_look  = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
}

Camera::~Camera()
{
}

void Camera::getPosition(D3DXVECTOR3* pos)
{
	*pos = _pos;
}

void Camera::setPosition(D3DXVECTOR3* pos)
{
	_pos = *pos;
}

void Camera::getRight(D3DXVECTOR3* right)
{
	*right = _right;
}

void Camera::getUp(D3DXVECTOR3* up)
{
	*up = _up;
}

void Camera::getLook(D3DXVECTOR3* look)
{
	*look = _look;
}

void Camera::walk(float units)
{
	_pos += D3DXVECTOR3(_look.x, 0.0f, _look.z) * units;
}

void Camera::strafe(float units)
{
	_pos += D3DXVECTOR3(_right.x, 0.0f, _right.z) * units;
}

void Camera::yaw(float angle)
{
	D3DXMATRIX T;
	D3DXMatrixRotationY(&T, angle);

	D3DXVec3TransformCoord(&_right,&_right, &T);
	D3DXVec3TransformCoord(&_look,&_look, &T);
}

void Camera::getViewMatrix(D3DXMATRIX* View)
{
	// Keep camera's axes orthogonal to each other
	D3DXVec3Normalize(&_look, &_look);

	D3DXVec3Cross(&_up, &_look, &_right);
	D3DXVec3Normalize(&_up, &_up);

	D3DXVec3Cross(&_right, &_up, &_look);
	D3DXVec3Normalize(&_right, &_right);

	// Build the view matrix:
	float x = -D3DXVec3Dot(&_right, &_pos);
	float y = -D3DXVec3Dot(&_up, &_pos);
	float z = -D3DXVec3Dot(&_look, &_pos);

	(*View)(0,0) = _right.x; (*View)(0, 1) = _up.x; (*View)(0, 2) = _look.x; (*View)(0, 3) = 0.0f;
	(*View)(1,0) = _right.y; (*View)(1, 1) = _up.y; (*View)(1, 2) = _look.y; (*View)(1, 3) = 0.0f;
	(*View)(2,0) = _right.z; (*View)(2, 1) = _up.z; (*View)(2, 2) = _look.z; (*View)(2, 3) = 0.0f;
	(*View)(3,0) = x;        (*View)(3, 1) = y;     (*View)(3, 2) = z;       (*View)(3, 3) = 1.0f;
}