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
		mCubeTransform.Translation = glm::vec3(0.0f, 1.0f, 0.0f);

		mScene.Camera.CameraTranslation = mCameraController.GetTranslation();
		mScene.Camera.ViewProjectionMatrix = mCameraController.GetViewProjectionMatrix();
		mScene.DirectionalLight.Direction = glm::vec3(0.0f, -1.0f, 0.0f);
		mScene.DirectionalLight.Ambient = glm::vec3(0.2f);
		mScene.DirectionalLight.Diffuse = glm::vec3(0.5f);
		mScene.DirectionalLight.Specular = glm::vec3(1.0f);
		mScene.PointLight.Position = glm::vec3(0.0f, 3.0f, 0.0f);
		mScene.PointLight.Ambient = glm::vec3(0.1f);
		mScene.PointLight.Diffuse = glm::vec3(1.0f);
		mScene.PointLight.Specular = glm::vec3(1.0f);
		mScene.PointLight.Constant = 1.0f;
		mScene.PointLight.Linear = 0.7f;
		mScene.PointLight.Quadratic = 1.8f;
	}

	SandboxModule::~SandboxModule()
	{
	}

	void SandboxModule::OnInit()
	{
		GLFWwindow* window = Application::Get()->GetWindow().GetWindow();

		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		mDiffuseTexture  = Texture::CreateFromFile("CobaltApp/Assets/Textures/container_diffuse.png");
		mSpecularTexture = Texture::CreateFromFile("CobaltApp/Assets/Textures/container_specular.png");

		MaterialData cubeMat;
		cubeMat.DiffuseMapHandle = Renderer::RegisterTexture(mDiffuseTexture.get());
		cubeMat.SpecularMapHandle = Renderer::RegisterTexture(mSpecularTexture.get());
		cubeMat.Shininess = 256.0f;

		MaterialData floorMat;
		floorMat.DiffuseMapHandle  = cubeMat.DiffuseMapHandle;
		floorMat.SpecularMapHandle = cubeMat.SpecularMapHandle;
		floorMat.Shininess = 128.0f;

		mCubeMat  = Renderer::RegisterMaterial(cubeMat);
		mFloorMat = Renderer::RegisterMaterial(floorMat);
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

		mScene.DirectionalLight.Direction = glm::normalize(mScene.DirectionalLight.Direction);

		Renderer::BeginScene(mScene);
		
		Renderer::DrawCube(mCubeTransform, mCubeMat);
		Renderer::DrawCube(mFloorTransform, mFloorMat);

		Renderer::EndScene();
	}

	void SandboxModule::OnUIRender()
	{
		if (ImGui::Begin("SandboxModule"))
		{
			if (ImGui::TreeNode("Scene Settings"))
			{
				RenderPointLight("Point Light", mScene.PointLight);

				ImGui::DragFloat3("Directional Light Direction", &mScene.DirectionalLight.Direction.x, 0.1f, -10.0f, 10.0f);
				ImGui::ColorEdit3("Directional Light Ambient", &mScene.DirectionalLight.Ambient.r);
				ImGui::ColorEdit3("Directional Light Diffuse", &mScene.DirectionalLight.Diffuse.r);
				ImGui::ColorEdit3("Directional Light Specular", &mScene.DirectionalLight.Specular.r);
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Transforms"))
			{
				RenderUITransform("Cube", mCubeTransform);
				RenderUITransform("Floor", mFloorTransform);
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Materials"))
			{
				RenderUIMaterial("Cube", mCubeMat);
				RenderUIMaterial("Floor", mFloorMat);
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

	void SandboxModule::RenderPointLight(const char* name, PointLightData& pointLight)
	{
		ImGui::Text("Pointlight: %s", name);
		
		ImGui::DragFloat3("  Position",    &pointLight.Position.x, 0.2f, -10.0f, 10.0f);
		ImGui::ColorEdit3("  Ambient Color", &pointLight.Ambient.r);
		ImGui::ColorEdit3("  Diffuse Color", &pointLight.Diffuse.r);
		ImGui::ColorEdit3("  Specular Color ", &pointLight.Specular.r);
		ImGui::DragFloat ("  Constant Coefficient", &pointLight.Constant, 0.001f, 0.0f, 2.0f);
		ImGui::DragFloat ("  Linear Coefficient", &pointLight.Linear, 0.001f, 0.0f, 2.0f);
		ImGui::DragFloat ("  Quadratic Coefficient", &pointLight.Quadratic, 0.001f, 0.0f, 2.0f);

		ImGui::Separator();
	}

	void SandboxModule::RenderUITransform(const char* name, Transform& transform)
	{
		ImGui::Text("Transform: %s", name);
		ImGui::DragFloat3("  Translation", &transform.Translation.x, 0.2f, -10.0f, 10.0f);
		ImGui::DragFloat3("  Rotation",    &transform.Rotation.x, 1.0f, 0.0f, 360.0f);
		ImGui::DragFloat3("  Scale",       &transform.Scale.x, 0.2f, -10.0f, 10.0f);

		ImGui::Separator();
	}

	void SandboxModule::RenderUIMaterial(const char* name, MaterialHandle material)
	{
		MaterialData& materialData = Renderer::GetMaterial(material);

		ImGui::Text("Material: %s", name);
		ImGui::DragFloat(" Specular", &materialData.Shininess, 1.0f, 0.0f, 1024.0f);

		ImGui::Separator();
	}

}