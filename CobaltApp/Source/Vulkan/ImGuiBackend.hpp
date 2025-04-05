#pragma once
#include <imgui.h>

namespace Cobalt
{

	class ImGuiBackend
	{
	public:
		static void Init();
		static void Shutdown();

		static void BeginFrame();
		static void EndFrame();

	private:

	};

}