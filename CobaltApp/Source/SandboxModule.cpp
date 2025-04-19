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

		mScene.Camera.CameraTranslation = mCameraController.GetTranslation();
		mScene.Camera.ViewProjectionMatrix = mCameraController.GetViewProjectionMatrix();
		mScene.DirectionalLight.Direction = glm::vec3(1.0f);
		mScene.DirectionalLight.Ambient = glm::vec3(0.2f);
		mScene.DirectionalLight.Diffuse = glm::vec3(0.5f);
		mScene.DirectionalLight.Specular = glm::vec3(1.0f);
	}

	SandboxModule::~SandboxModule()
	{
	}

	void SandboxModule::OnInit()
	{
		GLFWwindow* window = Application::Get()->GetWindow().GetWindow();

		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		MaterialData floorMat;
		floorMat.Ambient = glm::vec3(1.0f);
		floorMat.Diffuse = glm::vec3(1.0f);
		floorMat.Specular = glm::vec3(1.0f);
		floorMat.Shininess = 128.0f;

		MaterialData cubeMat;
		cubeMat.Ambient = glm::vec3(1.0f, 0.0f, 0.0f);
		cubeMat.Diffuse = glm::vec3(1.0f, 0.0f, 0.0f);
		cubeMat.Specular = glm::vec3(1.0f);
		cubeMat.Shininess = 256.0f;

		mFloorMatIdx = Renderer::RegisterMaterial(floorMat);
		mCubeMatIdx  = Renderer::RegisterMaterial(cubeMat);
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

		Renderer::BeginScene(mScene);
		
		Renderer::DrawCube(mFloorTransform, mFloorMatIdx);
		Renderer::DrawCube(mCubeTransform, mCubeMatIdx);

		Renderer::EndScene();
	}

	void SandboxModule::OnUIRender()
	{
		if (ImGui::Begin("SandboxModule"))
		{
			if (ImGui::TreeNode("Scene Settings"))
			{
				ImGui::DragFloat3("Directional Light Direction", &mScene.DirectionalLight.Direction.x, 0.1f, -10.0f, 10.0f);
				ImGui::ColorEdit3("Directional Light Ambient", &mScene.DirectionalLight.Ambient.r);
				ImGui::ColorEdit3("Directional Light Diffuse", &mScene.DirectionalLight.Diffuse.r);
				ImGui::ColorEdit3("Directional Light Specular", &mScene.DirectionalLight.Specular.r);
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Transforms"))
			{
				RenderUITransform("Floor", mFloorTransform);
				RenderUITransform("Cube", mCubeTransform);
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Materials"))
			{
				RenderUIMaterial("Floor", mFloorMatIdx);
				RenderUIMaterial("Cube", mCubeMatIdx);
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

	void SandboxModule::RenderUIMaterial(const char* name, uint32_t materialIndex)
	{
		MaterialData& material = Renderer::GetMaterial(materialIndex);

		ImGui::DragFloat3(std::format("{} Ambient", name).c_str(),  &material.Ambient.x, 0.05f, 0.0f, 1.0f);
		ImGui::DragFloat3(std::format("{} Diffuse", name).c_str(),  &material.Diffuse.x, 0.05f, 0.0f, 1.0f);
		ImGui::DragFloat3(std::format("{} Specular", name).c_str(), &material.Specular.x, 0.05f, 0.0f, 1.0f);
		ImGui::DragFloat(std::format("{} Specular", name).c_str(),  &material.Shininess, 1.0f, 0.0f, 1024.0f);

		ImGui::Separator();
	}


}