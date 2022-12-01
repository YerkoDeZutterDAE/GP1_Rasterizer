#pragma once
#include <cassert>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Math.h"
#include "Timer.h"

#include <iostream>

namespace dae
{
	struct Camera
	{
		Camera() = default;

		Camera(const Vector3& _origin, float _fovAngle):
			origin{_origin},
			fovAngle{_fovAngle}
		{
		}


		Vector3 origin{};
		float fovAngle{90.f};
		float fov{ tanf((fovAngle * TO_RADIANS) / 2.f) };

		Vector3 forward{Vector3::UnitZ};
		Vector3 up{Vector3::UnitY};
		Vector3 right{Vector3::UnitX};

		float totalPitch{};
		float totalYaw{160};

		Matrix invViewMatrix{};
		Matrix viewMatrix{};

		float moveSpeed{ 1.f };

		void Initialize(float _fovAngle = 90.f, Vector3 _origin = {0.f,0.f,0.f})
		{
			fovAngle = _fovAngle;
			fov = tanf((fovAngle * TO_RADIANS) / 2.f);

			origin = _origin;
		}

		void CalculateViewMatrix()
		{
			//TODO W1
			//ONB => invViewMatrix
			//Inverse(ONB) => ViewMatrix

			//viewMatrix
			viewMatrix = Matrix::CreateLookAtLH(origin, forward, { 0,1,0 });
			//invViewMatrix = { -viewMatrix.GetAxisX(), -viewMatrix.GetAxisY(), -viewMatrix.GetAxisZ(), -viewMatrix.GetTranslation() };
			invViewMatrix = viewMatrix.Inverse();

			//ViewMatrix => Matrix::CreateLookAtLH(...) [not implemented yet]
			//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixlookatlh
		}

		void CalculateProjectionMatrix()
		{
			//TODO W2

			//ProjectionMatrix => Matrix::CreatePerspectiveFovLH(...) [not implemented yet]
			//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixperspectivefovlh
		}

		void Update(Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();

			//Camera Update Logic
			//...

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);


			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			//todo: W2
			//assert(false && "Not Implemented Yet");

			if (pKeyboardState[SDL_SCANCODE_LSHIFT])
				moveSpeed = 6.f;
			else
				moveSpeed = 3.f;

			if (pKeyboardState[SDL_SCANCODE_W])
			{
				origin += forward * deltaTime * moveSpeed;
			}
			if (pKeyboardState[SDL_SCANCODE_S])
			{
				origin -= forward * deltaTime * moveSpeed;
			}
			if (pKeyboardState[SDL_SCANCODE_A])
			{
				origin += right * deltaTime * moveSpeed;
			}
			if (pKeyboardState[SDL_SCANCODE_D])
			{
				origin -= right * deltaTime * moveSpeed;
			}

			if (mouseState == SDL_BUTTON(SDL_BUTTON_LEFT))
			{
				origin += forward * (mouseY * -0.01f) * moveSpeed;

				totalPitch -= mouseX;

				forward.y = cos(totalYaw * 0.01f);
				forward.z = sin(totalYaw * 0.01f);

				forward.z = cos(totalPitch * 0.01f);
				forward.x = sin(totalPitch * 0.01f);

				forward.Normalize();
			}
			else if (mouseState == 5)
			{
				origin += up * (mouseY * -0.01f) * moveSpeed;

				totalYaw -= mouseX;

				forward.y = cos(totalYaw * 0.01f);
				forward.z = sin(totalYaw * 0.01f);

				forward.z = cos(totalPitch * 0.01f);
				forward.x = sin(totalPitch * 0.01f);

				forward.Normalize();
			}
			else if (mouseState == SDL_BUTTON(SDL_BUTTON_RIGHT))
			{
				totalPitch -= mouseX;
				totalYaw += mouseY;

				forward.y = cos(totalYaw * 0.01f);
				forward.z = sin(totalYaw * 0.01f);

				forward.z = cos(totalPitch * 0.01f);
				forward.x = sin(totalPitch * 0.01f);

				forward.Normalize();
			}

			//Update Matrices
			CalculateViewMatrix();
			CalculateProjectionMatrix(); //Try to optimize this - should only be called once or when fov/aspectRatio changes
		}

		Vector3 GetLookatVector(Vector3 input) const
		{

			return Vector3{
				invViewMatrix.GetAxisX().x* input.x + invViewMatrix.GetAxisY().x * input.y + invViewMatrix.GetAxisZ().x * input.z + invViewMatrix.GetTranslation().x,
				invViewMatrix.GetAxisX().y* input.x + invViewMatrix.GetAxisY().y * input.y + invViewMatrix.GetAxisZ().y * input.z + invViewMatrix.GetTranslation().y,
				invViewMatrix.GetAxisX().z* input.x + invViewMatrix.GetAxisY().z * input.y + invViewMatrix.GetAxisZ().z * input.z + invViewMatrix.GetTranslation().z
			};

		}
	};
}
