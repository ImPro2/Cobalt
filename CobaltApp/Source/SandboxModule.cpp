#include "copch.hpp"
#include "SandboxModule.hpp"
#include "Application.hpp"
#include "AssetManager.hpp"
#include "Vulkan/MaterialSystem.hpp"
#include "Vulkan/Texture.hpp"
#include "Vulkan/LightingPass.hpp"

#include <imgui.h>

namespace Cobalt
{

	SandboxModule::SandboxModule()
		: Module("SandboxModule")
	{
		CO_PROFILE_FN();

		float width  = Application::Get()->GetWindow().GetWidth();
		float height = Application::Get()->GetWindow().GetHeight();

		mCameraController = CameraController(width, height);

		mScene.Camera.CameraTranslation = mCameraController.GetTranslation();
		mScene.Camera.ViewProjectionMatrix = mCameraController.GetViewProjectionMatrix();

		mScene.DirectionalLight.Direction = glm::vec3(0.0f, -1.0f, 0.0f);
		mScene.DirectionalLight.Intensity = glm::vec3(1.0f);
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

		Model cubeModel("CobaltApp/Assets/Models/Cube.glb");
		Model sphereModel("CobaltApp/Assets/Models/Sphere.glb");
		
		mCubeMesh = AssetManager::GetMesh(cubeModel.GetMeshAssetHandle());
		mSphereMesh = AssetManager::GetMesh(sphereModel.GetMeshAssetHandle());

		auto& materialSystem = Renderer::GetMaterialSystem();

		mCubeMesh->SetMaterial(materialSystem.GetMaterial("PBR"));

		const Texture* defaultTexture = AssetManager::GetTexture("Default");
		Image defaultTextureImage = { defaultTexture->GetSampler(), defaultTexture->GetImageView(), defaultTexture->GetImageLayout() };

		for (uint32_t y = 0; y < sSphereGridSize; y++)
		{
			MaterialInfo sphereMaterialInfo;
			sphereMaterialInfo.SampledTextures = { defaultTextureImage };

			sphereMaterialInfo.ShaderEffectName = "Opaque";
			sphereMaterialInfo.PackedMaterial.BaseColorFactor = { 0.3f, 0.3f, 0.3f, 1.0f };
			sphereMaterialInfo.PackedMaterial.MetallicFactor = float(y) / (sSphereGridSize - 1);

			for (uint32_t x = 0; x < sSphereGridSize; x++)
			{
				sphereMaterialInfo.PackedMaterial.RoughnessFactor = float(x) / (sSphereGridSize - 1);

				std::string materialName = std::format("Sphere material {}:{}", x, y);

				mSphereMaterials[y][x] = materialSystem.BuildMaterial(materialName, sphereMaterialInfo);
			}
		}

		CubemapInfo cubemapInfo;
		//cubemapInfo.FilePaths.BackFace = "CobaltApp/Assets/Textures/Skybox/back.jpg";
		//cubemapInfo.FilePaths.DownFace = "CobaltApp/Assets/Textures/Skybox/bottom.jpg";
		//cubemapInfo.FilePaths.FrontFace = "CobaltApp/Assets/Textures/Skybox/front.jpg";
		//cubemapInfo.FilePaths.LeftFace = "CobaltApp/Assets/Textures/Skybox/left.jpg";
		//cubemapInfo.FilePaths.RightFace = "CobaltApp/Assets/Textures/Skybox/right.jpg";
		//cubemapInfo.FilePaths.UpFace = "CobaltApp/Assets/Textures/Skybox/top.jpg";
		cubemapInfo.FilePaths.EquirectangularMap = "CobaltApp/Assets/Textures/HDR/newport_loft.hdr";
		cubemapInfo.CubeMesh = mCubeMesh;

		Cubemap* skybox = AssetManager::GetCubemap(AssetManager::RegisterCubemap("Skybox", cubemapInfo));

		Renderer::SetSkybox(skybox);
	}

	void SandboxModule::OnShutdown()
	{
		CO_PROFILE_FN();
	}

	void SandboxModule::OnUpdate(float deltaTime)
	{
		CO_PROFILE_FN();

		const Application* application = Application::Get();
		
		static float lastTime = glfwGetTime();
		float currentTime = glfwGetTime();

		if (currentTime - lastTime > 0.5f)
		{
			lastTime = currentTime;
			mLastDeltaTime = deltaTime;
			mLastFPS = 1.0f / mLastDeltaTime;

			if (!application->GetInfo().EnableImGui)
			{
				std::cout << "Frame time: " << mLastDeltaTime * 1000.0f << "ms\, FPS: " << mLastFPS << "fps\n";
			}
		}

		GLFWwindow* window = application->GetWindow().GetWindow();

		static bool captureMouse = true;

		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		{
			captureMouse = false;
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
		else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
		{
			captureMouse = true;
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

		Transform cubeTransform;
		cubeTransform.Scale = { 20.0f, 0.2f, 20.0f };

		Renderer::BeginScene(mScene, mCameraController.GetProjectionMatrix(), mCameraController.GetViewMatrix());
		Renderer::DrawMesh(cubeTransform, mCubeMesh);

		float left = -10.0f;
		float right = 10.0f;
		float bottom = 2.0f;
		float top = 22.0f;

		Transform sphereTransform;
		sphereTransform.Scale = glm::vec3(0.8f);

		for (uint32_t i = 0; i < sSphereGridSize; i++)
		{
			float y = bottom + (top - bottom) * i / float(sSphereGridSize);

			for (uint32_t j = 0; j < sSphereGridSize; j++)
			{
				float x = left + (right - left) * j / float(sSphereGridSize);

				sphereTransform.Translation.x = x;
				sphereTransform.Translation.y = y;

				mSphereMesh->SetMaterial(mSphereMaterials[i][j]);
				Renderer::DrawMesh(sphereTransform, mSphereMesh);
			}
		}

		Renderer::EndScene();
	}

	void SandboxModule::OnUIRender()
	{
		CO_PROFILE_FN();

		ImGui::Begin("Debug");

		ImGui::Text("Delta Time: %fms", mLastDeltaTime * 1000.0f);
		ImGui::Text("FPS: %f", mLastFPS);
		ImGui::DragFloat3("Directional Light", &mScene.DirectionalLight.Direction.x, 0.1f, -1.0f, 1.0f);

		static int sphereIndices[2] = {};
		ImGui::DragInt2("Sphere", sphereIndices, 1.0f, 0, sSphereGridSize);

		Material* material = mSphereMaterials[sphereIndices[0]][sphereIndices[1]];
		MaterialInfo materialInfo = material->GetMaterialInfo();

		bool materialChanged = false;

		materialChanged |= ImGui::DragFloat3("Sphere Base Color", &materialInfo.PackedMaterial.BaseColorFactor.x, 0.01f, 0.0f, 1.0f);
		materialChanged |= ImGui::DragFloat("Sphere Roughness Factor", &materialInfo.PackedMaterial.RoughnessFactor, 0.01f, 0.0f, 1.0f);
		materialChanged |= ImGui::DragFloat("Sphere Metallic Factor", &materialInfo.PackedMaterial.MetallicFactor, 0.01f, 0.0f, 1.0f);

		if (materialChanged)
		{
			Renderer::GetMaterialSystem().UpdateMaterial(material->GetMaterialHandle(), materialInfo);
		}

		ImGui::End();
	}

	void SandboxModule::OnMouseMove(float x, float y)
	{
		CO_PROFILE_FN();

		mCameraController.OnMouseMove(x, y);
	}

	void SandboxModule::OnResize(uint32_t width, uint32_t height)
	{
		CO_PROFILE_FN();

		mCameraController.OnResize(width, height);
	}

}