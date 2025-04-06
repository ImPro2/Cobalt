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

	class Renderer
	{
	public:
		static void Init();
		static void Shutdown();

		static void OnResize();

	public:
		static void BeginScene(const glm::mat4& viewProjectionMatrix, const glm::vec3& cameraTranslation, const glm::vec3& lightPosition, const glm::vec3& lightColor);
		static void EndScene();

		static void DrawCube(const Transform& transform);

	public:
		static VkRenderPass GetMainRenderPass() { return sData->MainRenderPass; }

	private:
		static void CreateOrRecreateDepthTexture();
		static void CreateOrRecreateFramebuffers();

	private:
		struct SceneData
		{
			glm::mat4 ViewProjection;
			alignas(16) glm::vec3 LightPosition;
			alignas(16) glm::vec3 LightColor;
			alignas(16) glm::vec3 CameraPosition;
		};

		struct PushConstants
		{
			glm::mat4 CubeTransform;
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
			VkDescriptorSetLayout SceneDataDescriptorSetLayout;
			VkDescriptorSet SceneDataDescriptorSet;

			SceneData* CurrentSceneData = nullptr;
		};

		inline static RendererData* sData = nullptr;
	};

}
