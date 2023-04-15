#pragma once;
#include <vector>

class ValidationLayersInfo {
public:
	bool enableValidationLayers = true;

	const std::vector<const char*> validationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};
};