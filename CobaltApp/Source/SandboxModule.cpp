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
		mCubeTransform = Transform();
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
		Renderer::BeginScene(mCameraController.GetViewProjectionMatrix(), mCameraController.GetTranslation(), mLightPosition, mLightColor);
		Renderer::DrawCube(mCubeTransform);
		Renderer::EndScene();
	}

	void SandboxModule::OnUIRender()
	{
		if (ImGui::Begin("SandboxModule"))
		{
			//ImGui::DragFloat3("Camera Translation", &mCamera.Translation.x, -10.0f, 10.0f);

			//ImGui::Separator();

			ImGui::DragFloat3("Cube Translation", &mCubeTransform.Translation.x, -10.0f, 10.0f);
			ImGui::DragFloat3("Cube Rotation", &mCubeTransform.Rotation.x, -10.0f, 10.0f);
			ImGui::DragFloat3("Cube Scale", &mCubeTransform.Scale.x, -10.0f, 10.0f);

			ImGui::Separator();

			ImGui::DragFloat3("Light Position", &mLightPosition.x, -10.0f, 10.0f);
			ImGui::DragFloat3("Light Colour", &mLightColor.x, 0.0f, 1.0f);
		}

		ImGui::End();
	}

	void SandboxModule::OnMouseMove(float x, float y)
	{
		if (mCaptureMouse)
			mCameraController.OnMouseMove(x, y);
	}

}