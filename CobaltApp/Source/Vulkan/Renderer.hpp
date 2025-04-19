#pragma once
#include "GraphicsContext.hpp"
#include "Pipeline.hpp"
#include "VulkanBuffer.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include <array>

namespace Cobalt
{

	struct Transform
	{
		glm::vec3 Translation;
		glm::vec3 Rotation;
		glm::vec3 Scale = glm::vec3(1.0f);

		glm::mat4 GetTransform() const
		{
			glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));

			return glm::translate(glm::mat4(1.0f), Translation) * rotation * glm::scale(glm::mat4(1.0f), Scale);
		}
	};

	struct MaterialData
	{
		alignas(16) glm::vec3 Ambient;
		alignas(16) glm::vec3 Diffuse;
		alignas(16) glm::vec3 Specular;
		float Shininess;
	};

	struct CameraData
	{
		glm::mat4 ViewProjectionMatrix;
		alignas(16) glm::vec3 CameraTranslation;
	};

	struct DirectionalLightData
	{
		alignas(16) glm::vec3 Direction;

		alignas(16) glm::vec3 Ambient;
		alignas(16) glm::vec3 Diffuse;
		alignas(16) glm::vec3 Specular;
	};

	struct PointLightData
	{
		alignas(16) glm::vec3 Position;

		alignas(16) glm::vec3 Ambient;
		alignas(16) glm::vec3 Diffuse;
		alignas(16) glm::vec3 Specular;

		//float Constant;
		//float Linear;
		//float Quadratic;
	};

	struct SceneData
	{
		CameraData Camera;
		DirectionalLightData DirectionalLight;
		PointLightData PointLight;
	};

	struct ObjectData
	{
		glm::mat4 Transform;
		alignas(16) uint32_t MaterialIndex;
	};

	class Renderer
	{
	public:
		static void Init();
		static void Shutdown();

		static void OnResize();

	public:
		static uint32_t RegisterMaterial(const MaterialData& materialData);
		static MaterialData& GetMaterial(uint32_t materialIndex);

		static void BeginScene(const SceneData& scene);
		static void EndScene();

		static void DrawCube(const Transform& transform, uint32_t materialIndex);

	public:
		static VkRenderPass GetMainRenderPass() { return sData->MainRenderPass; }

	private:
		static void CreateOrRecreateDepthTexture();
		static void CreateOrRecreateFramebuffers();

	private:
		struct PushConstants
		{
			//glm::mat4 CubeTransform;
		};

		struct RendererData
		{
			VkRenderPass MainRenderPass;
			std::shared_ptr<Pipeline> CubePipeline;
			std::vector<VkFramebuffer> Framebuffers;
			std::unique_ptr<VulkanBuffer> VertexBuffer, IndexBuffer;

			VkImage DepthTexture;
			VkImageView DepthTextureView;
			VkDeviceMemory DepthTextureMemory;

			std::unique_ptr<VulkanBuffer> SceneDataUniformBuffer;
			std::unique_ptr<VulkanBuffer> MaterialDataStorageBuffer;
			std::unique_ptr<VulkanBuffer> ObjectDataStorageBuffer;

			VulkanDescriptorSet* SceneDataDescriptorSet = nullptr;
			VulkanDescriptorSet* MaterialDataDescriptorSet = nullptr;
			VulkanDescriptorSet* ObjectDataDescriptorSet = nullptr;

			SceneData* MappedSceneData = nullptr;
			MaterialData* MappedMaterialData = nullptr;
			ObjectData* MappedObjectData = nullptr;

			static constexpr uint32_t MaxObjectCount = 10000;
			static constexpr uint32_t MaxMaterialCount = 100;

			std::array<ObjectData, MaxObjectCount> Objects;
			uint32_t ObjectIndex = 0;

			std::array<MaterialData, MaxMaterialCount> Materials;
			uint32_t MaterialIndex = 0;
		};

		inline static RendererData* sData = nullptr;
	};

}
