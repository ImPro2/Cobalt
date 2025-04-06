#include "SandboxModule.hpp"
#include "Application.hpp"
#include <imgui.h>

namespace Cobalt
{

	SandboxModule::SandboxModule()
		: Module("SandboxModule")
	{
		float width  = Application::Get()->GetWindow().GetWidth();
		float height = Application::Get()->GetWindow().GetHeight();

		mCameraController = CameraController(width, height);

		mFloorTransform.Scale = glm::vec3(10.0f, 1.0f, 10.0f);
		mCubeTransform.Translation = glm::vec3(0.0f, 0.5f, 0.0f);

		mLightTransform.Translation = glm::vec3(1.2f, 1.0f, 2.0f);
		mLightTransform.Scale = glm::vec3(0.2f, 0.2f, 0.2f);
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
		Renderer::BeginScene(mCameraController.GetViewProjectionMatrix(), mCameraController.GetTranslation(), mLightTransform.Translation, mLightColor);
		Renderer::DrawCube(mCubeTransform);
		Renderer::DrawCube(mFloorTransform);
		Renderer::EndScene();
	}

	void SandboxModule::OnUIRender()
	{
		if (ImGui::Begin("SandboxModule"))
		{
			RenderUITransform("Floor", mFloorTransform);
			RenderUITransform("Cube", mCubeTransform);
			RenderUITransform("Light", mLightTransform);

			ImGui::ColorPicker3("Light Color", &mLightColor.x);
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

}