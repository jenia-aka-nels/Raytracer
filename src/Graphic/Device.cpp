#include "Device.h"

std::vector<VkPresentModeKHR>& Device::getSupportedSurfacePresentModes() { return _surfacePresentModes; }

std::vector<VkSurfaceFormatKHR>& Device::getSupportedSurfaceFormats() { return _surfaceFormats; }

VkSurfaceCapabilitiesKHR& Device::getSupportedSurfaceCapabilities() { return _surfaceCapabilities; }

std::optional<uint32_t> Device::getSupportedGraphicsFamilyIndex() { return _graphicsFamily; }

std::optional<uint32_t> Device::getSupportedPresentFamilyIndex() { return _presentFamily; }

std::optional<uint32_t> Device::getSupportedComputeFamilyIndex() { return _computeFamily; }

bool Device::_isDeviceSuitable(VkPhysicalDevice device) {
  // check if queue supported
  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

  int i = 0;
  for (const auto& queueFamily : queueFamilies) {
    if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      _graphicsFamily = i;
    }

    if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) {
      _computeFamily = i;
    }

    VkBool32 presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, _surface->getSurface(), &presentSupport);

    if (presentSupport) {
      _presentFamily = i;
    }

    if (_presentFamily.has_value() && _graphicsFamily.has_value() && _computeFamily.has_value()) {
      break;
    }

    i++;
  }

  // check if device extensions are supported
  uint32_t extensionCount;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

  std::vector<VkExtensionProperties> availableExtensions(extensionCount);
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

  std::set<std::string> requiredExtensions(_deviceExtensions.begin(), _deviceExtensions.end());

  for (const auto& extension : availableExtensions) {
    requiredExtensions.erase(extension.extensionName);
  }

  // check surface capabilities
  bool swapChainAdequate = false;
  if (requiredExtensions.empty()) {
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, _surface->getSurface(), &_surfaceCapabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface->getSurface(), &formatCount, nullptr);

    if (formatCount != 0) {
      _surfaceFormats.resize(formatCount);
      vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface->getSurface(), &formatCount, _surfaceFormats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface->getSurface(), &presentModeCount, nullptr);

    if (presentModeCount != 0) {
      _surfacePresentModes.resize(presentModeCount);
      vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface->getSurface(), &presentModeCount,
                                                _surfacePresentModes.data());
    }

    swapChainAdequate = !_surfaceFormats.empty() && !_surfacePresentModes.empty();
  }

  // check device features
  vkGetPhysicalDeviceFeatures(device, &_supportedFeatures);

  return _presentFamily.has_value() && _graphicsFamily.has_value() && _computeFamily.has_value() &&
         requiredExtensions.empty() && swapChainAdequate && _supportedFeatures.samplerAnisotropy;
}

void Device::_pickPhysicalDevice() {
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(_instance->getInstance(), &deviceCount, nullptr);

  if (deviceCount == 0) {
    throw std::runtime_error("failed to find GPUs with Vulkan support!");
  }

  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(_instance->getInstance(), &deviceCount, devices.data());

  for (const auto& device : devices) {
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(device, &props);
    if (_isDeviceSuitable(device) && props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
      _physicalDevice = device;
      break;
    }
  }

  if (_physicalDevice == VK_NULL_HANDLE) {
    throw std::runtime_error("failed to find a suitable GPU!");
  }
}

void Device::_createLogicalDevice() {
  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
  std::set<uint32_t> uniqueQueueFamilies = {_graphicsFamily.value(), _presentFamily.value(), _computeFamily.value()};

  float queuePriority = 1.0f;
  for (uint32_t queueFamily : uniqueQueueFamilies) {
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = queueFamily;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    queueCreateInfos.push_back(queueCreateInfo);
  }

  VkPhysicalDeviceFeatures deviceFeatures{};
  deviceFeatures.samplerAnisotropy = VK_TRUE;

  VkDeviceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

  createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
  createInfo.pQueueCreateInfos = queueCreateInfos.data();

  createInfo.pEnabledFeatures = &deviceFeatures;

  createInfo.enabledExtensionCount = static_cast<uint32_t>(_deviceExtensions.size());
  createInfo.ppEnabledExtensionNames = _deviceExtensions.data();
  createInfo.enabledLayerCount = 0;
  if (_instance->isValidation()) {
    createInfo.enabledLayerCount = static_cast<uint32_t>(_instance->getValidationLayers().size());
    createInfo.ppEnabledLayerNames = _instance->getValidationLayers().data();
  }

  if (vkCreateDevice(_physicalDevice, &createInfo, nullptr, &_logicalDevice) != VK_SUCCESS) {
    throw std::runtime_error("failed to create logical device!");
  }
}

VkFormat Device::findDepthBufferSupportedFormat(const std::vector<VkFormat>& candidates,
                                                VkImageTiling tiling,
                                                VkFormatFeatureFlags features) {
  for (VkFormat format : candidates) {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(_physicalDevice, format, &props);

    if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
      return format;
    } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
      return format;
    }
  }

  throw std::runtime_error("failed to find supported format!");
}

Device::Device(std::shared_ptr<Surface> surface, std::shared_ptr<Instance> instance) {
  _instance = instance;
  _surface = surface;
  _pickPhysicalDevice();
  _createLogicalDevice();
}

VkDevice& Device::getLogicalDevice() { return _logicalDevice; }

VkPhysicalDevice& Device::getPhysicalDevice() { return _physicalDevice; }

Device::~Device() { vkDestroyDevice(_logicalDevice, nullptr); }