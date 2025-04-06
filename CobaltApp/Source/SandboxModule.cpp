#include "SandboxModule.hpp"
#include <imgui.h>

namespace Cobalt
{

	SandboxModule::SandboxModule()
		: Module("SandboxModule")
	{
		mCamera.Translation = glm::vec3(0.0f, 0.0f, 3.0f);

		mCubeTransform = Transform();
	}

	SandboxModule::~SandboxModule()
	{
	}

	void SandboxModule::OnInit()
	{
	}

	void SandboxModule::OnShutdown()
	{
	}

	void SandboxModule::OnUpdate()
	{

	}

	void SandboxModule::OnRender()
	{
		Renderer::BeginScene(mCamera, mLightPosition, mLightColor);
		Renderer::DrawCube(mCubeTransform);
		Renderer::EndScene();
	}

	void SandboxModule::OnUIRender()
	{
		if (ImGui::Begin("SandboxModule"))
		{
			ImGui::SliderFloat3("Camera Translation", &mCamera.Translation.x, -10.0f, 10.0f);

			ImGui::Separator();

			ImGui::SliderFloat3("Cube Translation", &mCubeTransform.Translation.x, -10.0f, 10.0f);
			ImGui::SliderFloat3("Cube Rotation", &mCubeTransform.Rotation.x, -10.0f, 10.0f);
			ImGui::SliderFloat3("Cube Scale", &mCubeTransform.Scale.x, -10.0f, 10.0f);

			ImGui::Separator();

			ImGui::SliderFloat3("Light Position", &mLightPosition.x, -10.0f, 10.0f);
			ImGui::SliderFloat3("Light Colour", &mLightColor.x, 0.0f, 1.0f);
		}

		ImGui::End();
	}

}