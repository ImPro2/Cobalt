#pragma once
#include <string>

namespace Cobalt
{

	class Module
	{
	public:
		Module(const char* name)
			: mName(name)
		{
		}

		virtual ~Module() = default;

	public:
		virtual void OnInit() {}
		virtual void OnShutdown() {}

		virtual void OnUpdate(float deltaTime) {}

		virtual void OnRender() {}
		virtual void OnUIRender() {}

		virtual void OnMouseMove(float x, float y) {}

	public:
		const std::string& GetName() const { return mName; }

	private:
		std::string mName;
	};
	
}