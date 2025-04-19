project "spv_reflect"
	kind "StaticLib"

	files
	{
		"./spirv_reflect.c",
		"./spirv_reflect.h",
		"./include/spirv/unified1/spirv.h"
	}