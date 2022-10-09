#include "pch.h"
#include "Camera.h"

Camera::Camera(const Elite::FVector3& position, float fieldOfView, float width, float height, float nearPlane, float farPlane)
	: m_LookAtMatrix{}
	, m_LeftHandedProjectionMatrix{}
	, m_RightHandedProjectionMatrix{}
	, m_WorldUp{ 0,1,0 }
	, m_AspectRatio{ width / height }
	, m_Fov{ tanf(Elite::ToRadians(fieldOfView) / 2.f) }
	, m_MovementScaler{ 25 }
	, m_RotationScaler{ 5 }
	, m_Width{ width }
	, m_Height{ height }
	, m_Near{ nearPlane }
	, m_Far{ farPlane }
	, m_RightHandMode{ false }
{
	m_LookAtMatrix = m_LookAtMatrix.Identity();
	m_LookAtMatrix[3].xyz = { position.xy, -position.z };

	//making the LEFT handed projectionMatrix
	m_LeftHandedProjectionMatrix[0][0] = { 1 / (m_AspectRatio * m_Fov) };
	m_LeftHandedProjectionMatrix[1][1] = { 1 / m_Fov };
	float A{ m_Far / (m_Far - m_Near) };
	float B{ (-1 * (m_Far * m_Near)) / (m_Far - m_Near) };
	m_LeftHandedProjectionMatrix[2] = { 0, 0, A, 1 };
	m_LeftHandedProjectionMatrix[3][2] = { B };

	//making the RIGHT handed projectionMatrix
	m_RightHandedProjectionMatrix[0][0] = { 1 / (m_AspectRatio * m_Fov) };
	m_RightHandedProjectionMatrix[1][1] = { 1 / m_Fov };
	A = { m_Far / (m_Near - m_Far) };
	B = { (m_Far * m_Near) / (m_Near - m_Far) };
	m_RightHandedProjectionMatrix[2] = { 0, 0, A, -1 };
	m_RightHandedProjectionMatrix[3][2] = { B };
}

void Camera::SetFOV(float fov)
{
	m_Fov = tanf(Elite::ToRadians(fov) / 2.f);

	//updating LEFT Handed projectionMatrix
	m_LeftHandedProjectionMatrix[0][0] = { 1 / (m_AspectRatio * m_Fov) };
	m_LeftHandedProjectionMatrix[1][1] = { 1 / m_Fov };
	//updating RIGHT Handed projectionMatrix
	m_RightHandedProjectionMatrix[0][0] = { 1 / (m_AspectRatio * m_Fov) };
	m_RightHandedProjectionMatrix[1][1] = { 1 / m_Fov };
}

void Camera::ToggleHandedMode()
{
	//toggle bool and invert z position
	m_RightHandMode = !m_RightHandMode;
	m_LookAtMatrix[3].z = -m_LookAtMatrix[3].z;

	m_LookAtMatrix[2].x = -m_LookAtMatrix[2].x;
	m_LookAtMatrix[2].y = -m_LookAtMatrix[2].y;

	Elite::FVector3 right = { Elite::GetNormalized(Elite::Cross(m_WorldUp, m_LookAtMatrix[2].xyz)) };
	Elite::FVector3 up = { Elite::GetNormalized(Elite::Cross(m_LookAtMatrix[2].xyz, right)) };

	m_LookAtMatrix[0] = { right,0 };
	m_LookAtMatrix[1] = { up,0 };
}

void Camera::UpdateKeyboardMovement(const Elite::FVector3& normalMovent, float deltaT)
{
	Elite::FMatrix4 translate{ };
	translate = translate.Identity();

	translate[3].xyz = { normalMovent.x * m_MovementScaler * deltaT,
			normalMovent.y * m_MovementScaler * deltaT,
			normalMovent.z * m_MovementScaler * deltaT };
	if (!m_RightHandMode)
	{
		translate[3].z = -translate[3].z;
	}

	m_LookAtMatrix *= translate;
}

void Camera::UpdateMouseLeftDown(int x, int y, float deltaT)
{
	Elite::FMatrix4 transalte{ };
	transalte = transalte.Identity();
	float thetay{};

	if (m_RightHandMode)
	{
		transalte[3].z += y / m_RotationScaler;
		thetay = { -Elite::ToRadians(x / m_RotationScaler) };
	}
	else
	{
		transalte[3].z -= y / m_RotationScaler;
		thetay = { Elite::ToRadians(x / m_RotationScaler) };
	}

	m_LookAtMatrix *= transalte;

	Elite::FMatrix4 yrotationMatrix{ Elite::MakeRotationY(thetay) };

	m_LookAtMatrix *= yrotationMatrix;

	Elite::FVector3 right = { Elite::GetNormalized(Elite::Cross(m_WorldUp, m_LookAtMatrix[2].xyz)) };
	Elite::FVector3 up{ Elite::GetNormalized(Elite::Cross(m_LookAtMatrix[2].xyz, right)) };

	m_LookAtMatrix[0] = { right,0 };
	m_LookAtMatrix[1] = { up,0 };
}

void Camera::UpdateMouseRightDown(int x, int y, float deltaT)
{
	if (x == 0 && y == 0) { return; }

	float thetax{};
	float thetay{};

	if (m_RightHandMode)
	{
		thetax = { -Elite::ToRadians(x / m_RotationScaler) };
		thetay = { -Elite::ToRadians(y / m_RotationScaler) };
	}
	else
	{
		thetax = { Elite::ToRadians(x / m_RotationScaler) };
		thetay = { Elite::ToRadians(y / m_RotationScaler) };
	}

	Elite::FMatrix4 yrotationMatrix{ Elite::MakeRotationX(thetay) };
	Elite::FMatrix4 xrotationMatrix{ Elite::MakeRotationY(thetax) };

	m_LookAtMatrix *= yrotationMatrix * xrotationMatrix;

	Elite::FVector3 right = { Elite::GetNormalized(Elite::Cross(m_WorldUp, m_LookAtMatrix[2].xyz)) };
	Elite::FVector3 up = { Elite::GetNormalized(Elite::Cross(m_LookAtMatrix[2].xyz, right)) };

	m_LookAtMatrix[0] = { right,0 };
	m_LookAtMatrix[1] = { up,0 };
}

void Camera::UpdateMouseBothDown(int x, int y, float deltaT)
{
	Elite::FMatrix4 transalte{ };
	transalte = transalte.Identity();
	if (y > 0) { transalte[3].y += m_MovementScaler * deltaT; }
	else if (y < 0) { transalte[3].y -= m_MovementScaler * deltaT; }
	m_LookAtMatrix *= transalte;
}