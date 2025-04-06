#pragma once
#include "Module.hpp"
#include "Vulkan/Renderer.hpp"

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

		virtual void OnUpdate() override;
		virtual void OnRender() override;

		virtual void OnUIRender() override;

	private:
		Camera mCamera;
		Transform mCubeTransform;

		glm::vec3 mLightPosition = glm::vec3(1.2f, 1.0f, 2.0f);
		glm::vec3 mLightColor = glm::vec3(1.0f, 1.0f, 1.0f);
	};


}