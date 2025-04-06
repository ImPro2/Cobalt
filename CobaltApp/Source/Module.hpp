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
		virtual void OnUpdate() {}
		virtual void OnRender() {}
		virtual void OnUIRender() {}

	public:
		const std::string& GetName() const { return mName; }

	private:
		std::string mName;
	};
	
}