#pragma once
#include "Module.hpp"
#include "Vulkan/Renderer.hpp"
#include "Camera.hpp"

namespace Cobalt
{

	class SandboxModule : public Module
	{
	public:
		SandboxModule();
		~SandboxModule();

	public:
		virtual void OnInit() override;
		virtual void OnShutdown() override;

		virtual void OnUpdate(float deltaTime) override;
		virtual void OnRender() override;

		virtual void OnUIRender() override;

		virtual void OnMouseMove(float x, float y) override;

	private:
		void RenderUITransform(const char* name, Transform& transform);

	private:
		CameraController mCameraController;

		Transform mFloorTransform = Transform();
		Transform mCubeTransform = Transform();
		Transform mLightTransform = Transform();

		glm::vec3 mLightColor = glm::vec3(1.0f, 1.0f, 1.0f);

		bool mCaptureMouse = true;
	};


}