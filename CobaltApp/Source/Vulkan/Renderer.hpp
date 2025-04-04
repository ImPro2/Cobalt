#pragma once
#include "GraphicsContext.hpp"

namespace Cobalt
{

	class Renderer
	{
	public:
		Renderer(const GraphicsContext& graphicsContext);
		~Renderer();

	public:
		void Init();
		void Shutdown();

	public:
		void BeginScene();
		void EndScene();

	private:
		const GraphicsContext& mGraphicsContext;
	};

}
