#pragma once
#include "GraphicsContext.hpp"
#include "Pipeline.hpp"
#include "VulkanBuffer.hpp"
#include "Texture.hpp"
#include "Material.hpp"
#include "Mesh.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include <array>

#define CO_MAX_POINT_LIGHT_COUNT 16

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

		float Constant;
		float Linear;
		float Quadratic;

		float __padding0;
	};

	struct SceneData
	{
		CameraData Camera;
		DirectionalLightData DirectionalLight;
		PointLightData PointLights[CO_MAX_POINT_LIGHT_COUNT];
		uint32_t PointLightCount;
	};


	struct ObjectData
	{
		glm::mat4 Transform;
		alignas(16) MaterialHandle MaterialHandle;
	};

	class Renderer
	{
	public:
		static void Init();
		static void Shutdown();

		static void OnResize();

	public:
		static TextureHandle CreateTexture(const TextureInfo& textureInfo);
		static MaterialHandle CreateMaterial(const MaterialData& materialData);

		static Texture& GetTexture(TextureHandle textureHandle);
		static Material& GetMaterial(MaterialHandle materialHandle);

		static void BeginScene(const SceneData& scene);
		static void EndScene();

		static void DrawCube(const Transform& transform, MaterialHandle material);

		static void DrawMesh(const Transform& transform, Mesh mesh);

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

			std::unique_ptr<VulkanBuffer> SceneDataUniformBuffer;
			std::unique_ptr<VulkanBuffer> MaterialDataStorageBuffer;
			std::unique_ptr<VulkanBuffer> ObjectDataStorageBuffer;

			std::unique_ptr<Texture> DepthTexture;

			static constexpr uint32_t sGlobalDescriptorSetIndex   = 0;
			static constexpr uint32_t sMaterialDescriptorSetIndex = 1;
			static constexpr uint32_t sObjectDescriptorSetIndex   = 2;

			VulkanDescriptorSet* GlobalDescriptorSet = nullptr;
			VulkanDescriptorSet* MaterialDescriptorSet = nullptr;
			VulkanDescriptorSet* ObjectDescriptorSet = nullptr;

			//SceneData* MappedSceneData = nullptr;
			//MaterialData* MappedMaterialData = nullptr;
			//ObjectData* MappedObjectData = nullptr;

			static constexpr uint32_t MaxObjectCount = 10000;
			static constexpr uint32_t MaxMaterialCount = 100;

			std::vector<std::unique_ptr<Texture>> Textures;
			std::vector<std::unique_ptr<Material>> Materials;

			std::array<ObjectData, MaxObjectCount> Objects;
			std::array<MaterialData, MaxMaterialCount> MaterialDatas;
			uint32_t ObjectIndex = 0;
			uint32_t MaterialIndex = 0;
			uint32_t BindlessTextureIndex = 0;

			static constexpr const char* sShaderFilePath = "CobaltApp/Assets/Shaders/CubeShader.glsl";
		};

		inline static RendererData* sData = nullptr;
	};

}
