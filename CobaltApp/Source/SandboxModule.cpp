#include "copch.hpp"
#include "SandboxModule.hpp"
#include "Application.hpp"
#include <imgui.h>

namespace Cobalt
{

	SandboxModule::SandboxModule()
		: Module("SandboxModule")
	{
		CO_PROFILE_FN();

		uint32_t a = sizeof(glm::mat4);
		uint32_t b = sizeof(MaterialData);

		float width  = Application::Get()->GetWindow().GetWidth();
		float height = Application::Get()->GetWindow().GetHeight();

		mCameraController = CameraController(width, height);

		//mFloorTransform.Scale = glm::vec3(10.0f, 1.0f, 10.0f);
		//mCubeTransform.Translation = glm::vec3(0.0f, 1.0f, 0.0f);

		mScene.Camera.CameraTranslation = mCameraController.GetTranslation();
		mScene.Camera.ViewProjectionMatrix = mCameraController.GetViewProjectionMatrix();

		mScene.DirectionalLight.Direction = glm::vec3(0.0f, -1.0f, 0.0f);
		mScene.DirectionalLight.Ambient = glm::vec3(0.2f);
		mScene.DirectionalLight.Diffuse = glm::vec3(0.5f);
		mScene.DirectionalLight.Specular = glm::vec3(1.0f);

		mScene.PointLightCount = 0;

		mScene.PointLights[0].Position = glm::vec3(0.0f, 3.0f, 0.0f);
		mScene.PointLights[0].Ambient = glm::vec3(0.1f);
		mScene.PointLights[0].Diffuse = glm::vec3(1.0f, 0.0f, 0.0f);
		mScene.PointLights[0].Specular = glm::vec3(1.0f);
		mScene.PointLights[0].Constant = 1.0f;
		mScene.PointLights[0].Linear = 0.7f;
		mScene.PointLights[0].Quadratic = 1.8f;

		mScene.PointLights[1].Position = glm::vec3(0.0f, 3.0f, 0.0f);
		mScene.PointLights[1].Ambient = glm::vec3(0.1f);
		mScene.PointLights[1].Diffuse = glm::vec3(0.0f, 1.0f, 0.0f);
		mScene.PointLights[1].Specular = glm::vec3(1.0f);
		mScene.PointLights[1].Constant = 1.0f;
		mScene.PointLights[1].Linear = 0.7f;
		mScene.PointLights[1].Quadratic = 1.8f;

		mScene.PointLights[2].Position = glm::vec3(0.0f, 3.0f, 0.0f);
		mScene.PointLights[2].Ambient = glm::vec3(0.1f);
		mScene.PointLights[2].Diffuse = glm::vec3(0.0f, 0.0f, 1.0f);
		mScene.PointLights[2].Specular = glm::vec3(1.0f);
		mScene.PointLights[2].Constant = 1.0f;
		mScene.PointLights[2].Linear = 0.7f;
		mScene.PointLights[2].Quadratic = 1.8f;

		mFloorTransform.Translation = glm::vec3(0.0f, -0.5f, 0.0f);
		mFloorTransform.Rotation = glm::vec3(0.0f);
		mFloorTransform.Scale = glm::vec3(10.0f, 1.0f, 10.0f);

		mSphereTransforms.resize(10 * 10);

		uint32_t i = 0;

		for (float y = 2.0f; y <= 20.0f; y += 2.0f)
		{
			for (float x = -8.0f; x <= 10.0f; x += 2.0f)
			{
				mSphereTransforms[i] = Transform();
				mSphereTransforms[i].Translation = { x, y, 5.0f };

				i++;
			}
		}
	}

	SandboxModule::~SandboxModule()
	{
		CO_PROFILE_FN();
	}

	void SandboxModule::OnInit()
	{
		CO_PROFILE_FN();

		GLFWwindow* window = Application::Get()->GetWindow().GetWindow();

		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		mSphereModel = std::make_unique<Model>("CobaltApp/Assets/Models/sphere.obj");
		mCubeModel   = std::make_unique<Model>("CobaltApp/Assets/Models/cube.obj");
	}

	void SandboxModule::OnShutdown()
	{
		CO_PROFILE_FN();
	}

	void SandboxModule::OnUpdate(float deltaTime)
	{
		CO_PROFILE_FN();
		
		static float lastTime = glfwGetTime();
		float currentTime = glfwGetTime();

		if (currentTime - lastTime > 1.0f)
		{
			mDeltaTime = deltaTime;
			lastTime = currentTime;

			if (!Application::Get()->GetInfo().EnableImGui)
				std::cout << "Frame Time: " << deltaTime * 1000.0f << "ms" << std::endl << "FPS: " << 1.0f / deltaTime << std::endl;
		}

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
		CO_PROFILE_FN();

		mScene.Camera.CameraTranslation = mCameraController.GetTranslation();
		mScene.Camera.ViewProjectionMatrix = mCameraController.GetViewProjectionMatrix();

		mScene.DirectionalLight.Direction = glm::normalize(mScene.DirectionalLight.Direction);

		Renderer::BeginScene(mScene);

		const auto& sphereMesh = mSphereModel->GetMeshes()[0];

		for (const Transform& transform : mSphereTransforms)
			Renderer::DrawMesh(transform, sphereMesh.get());

		Renderer::DrawMesh(mFloorTransform, mCubeModel->GetMeshes()[0].get());

		Renderer::EndScene();
	}

	void SandboxModule::OnUIRender()
	{
		CO_PROFILE_FN();

		if (ImGui::Begin("SandboxModule"))
		{
			ImGui::Text("Frame Time: %fms", mDeltaTime * 1000.0f);
			ImGui::Text("FPS: %f", 1.0f / mDeltaTime);
			
			if (ImGui::TreeNode("Scene Settings"))
			{
				for (uint32_t i = 0; i < mScene.PointLightCount; i++)
					RenderPointLight(std::format("Point Light {}", i).c_str(), mScene.PointLights[i]);

				ImGui::DragFloat3("Directional Light Direction", &mScene.DirectionalLight.Direction.x, 0.1f, -10.0f, 10.0f);
				ImGui::ColorEdit3("Directional Light Ambient", &mScene.DirectionalLight.Ambient.r);
				ImGui::ColorEdit3("Directional Light Diffuse", &mScene.DirectionalLight.Diffuse.r);
				ImGui::ColorEdit3("Directional Light Specular", &mScene.DirectionalLight.Specular.r);
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Transforms"))
			{
				//RenderUITransform("Sphere", mSphereTransform);
				RenderUITransform("Floor", mFloorTransform);
				ImGui::TreePop();
			}

			/*if (ImGui::TreeNode("Materials"))
			{
				//RenderUIMaterial("Cube", mCubeMat);
				//RenderUIMaterial("Floor", mFloorMat);
				ImGui::TreePop();
			}*/
		}

		ImGui::End();
	}

	void SandboxModule::OnMouseMove(float x, float y)
	{
		CO_PROFILE_FN();

		if (mCaptureMouse)
			mCameraController.OnMouseMove(x, y);
	}

	void SandboxModule::RenderPointLight(const char* name, PointLightData& pointLight)
	{
		CO_PROFILE_FN();

		ImGui::Text("Pointlight: %s", name);
		
		ImGui::DragFloat3(std::format("{} Position", name).c_str(), &pointLight.Position.x, 0.2f, -10.0f, 10.0f);
		ImGui::ColorEdit3(std::format("{} Ambient Color", name).c_str(), &pointLight.Ambient.r);
		ImGui::ColorEdit3(std::format("{} Diffuse Color", name).c_str(), &pointLight.Diffuse.r);
		ImGui::ColorEdit3(std::format("{} Specular Color ", name).c_str(), &pointLight.Specular.r);
		ImGui::DragFloat (std::format("{} Constant Coefficient", name).c_str(), &pointLight.Constant, 0.001f, 0.0f, 2.0f);
		ImGui::DragFloat (std::format("{} Linear Coefficient", name).c_str(), &pointLight.Linear, 0.001f, 0.0f, 2.0f);
		ImGui::DragFloat (std::format("{} Quadratic Coefficient", name).c_str(), &pointLight.Quadratic, 0.001f, 0.0f, 2.0f);

		ImGui::Separator();
	}

	void SandboxModule::RenderUITransform(const char* name, Transform& transform)
	{
		CO_PROFILE_FN();

		ImGui::Text("Transform: %s", name);
		ImGui::DragFloat3(std::format("{} Translation", name).c_str(), &transform.Translation.x, 0.2f, -10.0f, 10.0f);
		ImGui::DragFloat3(std::format("{} Rotation", name).c_str(),    &transform.Rotation.x, 1.0f, 0.0f, 360.0f);
		ImGui::DragFloat3(std::format("{} Scale", name).c_str(),       &transform.Scale.x, 0.2f, -10.0f, 10.0f);

		ImGui::Separator();
	}

	void SandboxModule::RenderUIMaterial(const char* name, MaterialHandle material)
	{
		CO_PROFILE_FN();

		//MaterialData& materialData = Renderer::GetMaterial(material);

		//ImGui::Text("Material: %s", name);
		//ImGui::DragFloat(std::format("{}  Specular", name).c_str(), &materialData.Shininess, 1.0f, 0.0f, 1024.0f);

		//ImGui::Separator();
	}

}