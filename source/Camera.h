#pragma once
#include "EMath.h"

class Camera
{
public:
	Camera(const Elite::FVector3& position, float aspectRatio, float width, float height, float nearPlane, float farPlane);
	~Camera() = default;

	Elite::FVector3 GetCameraPos() const { return m_LookAtMatrix[3].xyz; }
	Elite::FVector3 GetForwardVec() const { return m_LookAtMatrix[2].xyz; }
	Elite::FMatrix4 GetLookAtMatrix() const { return m_LookAtMatrix; }

	Elite::FMatrix4 GetInverseLookAtMatrix() const { return Elite::Inverse(m_LookAtMatrix); }
	Elite::FMatrix4 GetLeftHandedProjectionMatrix() const { return m_LeftHandedProjectionMatrix; }
	Elite::FMatrix4 GetRightHandedProjectionMatrix() const { return m_RightHandedProjectionMatrix; }

	float GetAspectRatio() const { return m_AspectRatio; }
	float GetFov() const { return m_Fov; }
	void SetFOV(float fov);

	void ToggleHandedMode(/*bool boolean*/) ;

	//key and mouse input converted into camera movement
	void UpdateKeyboardMovement(const Elite::FVector3& normalMovent, float deltaT);
	void UpdateMouseLeftDown(int x, int y, float deltaT);
	void UpdateMouseRightDown(int x, int y, float deltaT);
	void UpdateMouseBothDown(int x, int y, float deltaT);

private:

	Elite::FMatrix4 m_LookAtMatrix;
	Elite::FMatrix4 m_LeftHandedProjectionMatrix;
	Elite::FMatrix4 m_RightHandedProjectionMatrix;
	Elite::FVector3 m_WorldUp;

	float m_AspectRatio;
	float m_Fov;
	float m_MovementScaler;
	float m_RotationScaler;
	float m_Width;
	float m_Height;
	float m_Near;
	float m_Far;

	bool m_RightHandMode;
};

