#include "SandboxModule.hpp"
#include "Application.hpp"
#include <imgui.h>

namespace Cobalt
{

	SandboxModule::SandboxModule()
		: Module("SandboxModule")
	{

		uint32_t a = sizeof(glm::mat4);
		uint32_t b = sizeof(MaterialData);

		float width  = Application::Get()->GetWindow().GetWidth();
		float height = Application::Get()->GetWindow().GetHeight();

		mCameraController = CameraController(width, height);

		mFloorTransform.Scale = glm::vec3(10.0f, 1.0f, 10.0f);
		mCubeTransform.Translation = glm::vec3(0.0f, 0.5f, 0.0f);

		mLightTransform.Translation = glm::vec3(1.2f, 1.0f, 2.0f);
		mLightTransform.Scale = glm::vec3(0.2f, 0.2f, 0.2f);

		mFloorMat.Ambient = glm::vec3(1.0f);
		mFloorMat.Diffuse = glm::vec3(1.0f);
		mFloorMat.Specular = glm::vec3(1.0f);
		mFloorMat.Shininess = 128.0f;

		mCubeMat.Ambient = glm::vec3(1.0f, 0.0f, 0.0f);
		mCubeMat.Diffuse = glm::vec3(1.0f, 0.0f, 0.0f);
		mCubeMat.Specular = glm::vec3(1.0f);
		mCubeMat.Shininess = 256.0f;

		mScene.Camera.CameraTranslation = mCameraController.GetTranslation();
		mScene.Camera.ViewProjectionMatrix = mCameraController.GetViewProjectionMatrix();
		mScene.Light.Color = glm::vec3(1.0f);
		mScene.Light.Position = mLightTransform.Translation;
	}

	SandboxModule::~SandboxModule()
	{
	}

	void SandboxModule::OnInit()
	{
		GLFWwindow* window = Application::Get()->GetWindow().GetWindow();

		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}

	void SandboxModule::OnShutdown()
	{
	}

	void SandboxModule::OnUpdate(float deltaTime)
	{
		GLFWwindow* window = Application::Get()->GetWindow().GetWindow();

		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		{
			mCaptureMouse = false;
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
		else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
		{
			mCaptureMouse = true;
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}

		mCameraController.OnUpdate(deltaTime);
	}

	void SandboxModule::OnRender()
	{
		mScene.Camera.CameraTranslation = mCameraController.GetTranslation();
		mScene.Camera.ViewProjectionMatrix = mCameraController.GetViewProjectionMatrix();
		mScene.Light.Position = mLightTransform.Translation;

		Renderer::BeginScene(mScene);
		
		Renderer::DrawCube(mFloorTransform, mFloorMat);
		Renderer::DrawCube(mCubeTransform, mCubeMat);

		Renderer::EndScene();
	}

	void SandboxModule::OnUIRender()
	{
		if (ImGui::Begin("SandboxModule"))
		{
			if (ImGui::TreeNode("Scene Settings"))
			{
				ImGui::ColorPicker3("Light Color", &mScene.Light.Color.r);
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Cube Transforms"))
			{
				RenderUITransform("Floor", mFloorTransform);
				RenderUITransform("Cube", mCubeTransform);
				RenderUITransform("Light", mLightTransform);
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Cube Materials"))
			{
				RenderUIMaterial("Floor", mFloorMat);
				RenderUIMaterial("Cube", mCubeMat);
				ImGui::TreePop();
			}
		}

		ImGui::End();
	}

	void SandboxModule::OnMouseMove(float x, float y)
	{
		if (mCaptureMouse)
			mCameraController.OnMouseMove(x, y);
	}

	void SandboxModule::RenderUITransform(const char* name, Transform& transform)
	{
		ImGui::DragFloat3(std::format("{} Translation", name).c_str(), &transform.Translation.x, 0.2f, -10.0f, 10.0f);
		ImGui::DragFloat3(std::format("{} Rotation", name).c_str(),    &transform.Rotation.x, 1.0f, 0.0f, 360.0f);
		ImGui::DragFloat3(std::format("{} Scale", name).c_str(),       &transform.Scale.x, 0.2f, -10.0f, 10.0f);

		ImGui::Separator();
	}

	void SandboxModule::RenderUIMaterial(const char* name, MaterialData& material)
	{
		ImGui::DragFloat3(std::format("{} Ambient", name).c_str(),  &material.Ambient.x, 0.05f, 0.0f, 1.0f);
		ImGui::DragFloat3(std::format("{} Diffuse", name).c_str(),  &material.Diffuse.x, 0.05f, 0.0f, 1.0f);
		ImGui::DragFloat3(std::format("{} Specular", name).c_str(), &material.Specular.x, 0.05f, 0.0f, 1.0f);
		ImGui::DragFloat(std::format("{} Specular", name).c_str(),  &material.Shininess, 1.0f, 0.0f, 1024.0f);

		ImGui::Separator();
	}


}