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
		void RenderUIMaterial(const char* name, uint32_t materialIndex);

	private:
		CameraController mCameraController;

		Transform mFloorTransform = Transform();
		Transform mCubeTransform = Transform();

		uint32_t mFloorMatIdx, mCubeMatIdx;

		SceneData mScene;

		bool mCaptureMouse = true;
	};

}