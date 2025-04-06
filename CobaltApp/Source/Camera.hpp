#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_inverse.hpp>

namespace Cobalt
{

	class CameraController
	{
	public:
		CameraController() = default;
		CameraController(float viewportWidth, float viewportHeight, float fov = 45.0f, float nearClip = 0.1f, float farClip = 1000.0f);

	public:
		void OnUpdate(float deltaTime);
		void OnResize(float viewportWidth, float viewportHeight);
		void OnMouseMove(float x, float y);

	public:
		const glm::vec3& GetTranslation() const { return mTranslation; }
		const glm::mat4& GetViewProjectionMatrix() const { return mViewProjectionMatrix; }

	private:
		void RecalculateViewMatrix();
		void RecalculateProjectionMatrix();

	private:
		float mAspectRatio;
		float mFOV = 45.0f;
		float mNearClip = 0.1f;
		float mFarClip = 1000.0f;

		float mViewportWidth, mViewportHeight;

		float mCameraSpeed = 10.0f;
		float mMouseSensitivity = 0.1f;

		glm::vec3 mTranslation = glm::vec3(0.0f, 0.0f, 3.0f);
		glm::vec3 mForwardDir = glm::vec3(0.0f, 0.0f, -1.0f);
		glm::vec3 mUpDir = glm::vec3(0.0f, 1.0f, 0.0f);

		glm::mat4 mViewMatrix, mProjectionMatrix;
		glm::mat4 mViewProjectionMatrix;

		float mPitch = 0.0f, mYaw = 0.0f;

		float mLastMouseX = 0.0f;
		float mLastMouseY = 0.0f;
	};

}