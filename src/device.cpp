//
// Created by kurono267 on 17.09.17.
//

// Implementation Metal Device based at Vulkan

#include "mtlpp.hpp"

#include <set>
#include <iostream>

using namespace mtlpp;

#ifdef NDEBUG
const bool enableValidationLayers = true;
#else
const bool enableValidationLayers = true;
#endif

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	VK_KHX_MULTIVIEW_EXTENSION_NAME
};

const std::vector<const char*> validationLayers = {
    "VK_LAYER_LUNARG_standard_validation",
    "VK_LAYER_LUNARG_parameter_validation"
};

std::vector<const char*> getRequiredExtensions() {
	std::vector<const char*> extensions;

	unsigned int glfwExtensionCount = 0;
	const char** glfwExtensions;

	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	for (unsigned int i = 0; i < glfwExtensionCount; i++) {
		extensions.push_back(glfwExtensions[i]);
	}

	if (enableValidationLayers) {
		extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	}

	return extensions;
}

struct QueueFamilyIndices {
	int graphicsFamily = -1;
	int presentFamily = -1;
	int computeFamily = -1;

	bool isComplete() {
		return graphicsFamily >= 0 && presentFamily >= 0 && computeFamily >= 0;
	}
};

QueueFamilyIndices queueFamilies(const vk::PhysicalDevice& device,const vk::SurfaceKHR surface) {
	QueueFamilyIndices indices;

	auto queueFamilies = device.getQueueFamilyProperties();

	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		if (indices.graphicsFamily == -1 && queueFamily.queueCount > 0 && queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
			indices.graphicsFamily = i;
		}

		vk::Bool32 presentSupport = device.getSurfaceSupportKHR(i,surface);

		if(queueFamily.queueCount > 0 && presentSupport){
			indices.presentFamily = i;
		}

		if(queueFamily.queueCount > 0 && queueFamily.queueFlags & vk::QueueFlagBits::eCompute){
			indices.computeFamily = i;
		}

		i++;
	}
	std::cout << "Selected Queues:" << std::endl;
	std::cout << "Graphics " << indices.graphicsFamily << std::endl;
	std::cout << "Present "  << indices.presentFamily << std::endl;
	std::cout << "Compute "  << indices.computeFamily << std::endl;

	return indices;
}

bool checkDeviceExtensionSupport(vk::PhysicalDevice device) {
	auto availableExtensions = device.enumerateDeviceExtensionProperties();

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

struct SwapchainSupportDetails {
	vk::SurfaceCapabilitiesKHR capabilities;
	std::vector<vk::SurfaceFormatKHR> formats;
	std::vector<vk::PresentModeKHR> presentModes;
};

SwapchainSupportDetails swapchainSupport(vk::PhysicalDevice device,vk::SurfaceKHR surface){
	SwapchainSupportDetails details;

	details.capabilities = device.getSurfaceCapabilitiesKHR(surface);
	details.formats      = device.getSurfaceFormatsKHR(surface);
	details.presentModes = device.getSurfacePresentModesKHR(surface);

	return details;
}

bool isDeviceSuitable(const vk::PhysicalDevice& device,vk::SurfaceKHR surface){
	QueueFamilyIndices indices = queueFamilies(device,surface);

	bool extensionsSupported = checkDeviceExtensionSupport(device);
	bool swapChainAdequate = false;
	if(extensionsSupported){
		SwapchainSupportDetails swapChainSupport = swapchainSupport(device,surface);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	vk::PhysicalDeviceFeatures supportedFeatures = device.getFeatures();

	return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

VkResult vkCreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback) {
    auto func = (PFN_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pCallback);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void vkDestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugReportCallbackEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
    if (func != nullptr) {
        func(instance, callback, pAllocator);
    }
}

// Validation layers
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char* layerPrefix, const char* msg, void* userData) {
	std::cerr << "validation layer: " << msg << std::endl;

	return VK_FALSE;
}

vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities,const int& width, const int& height) {
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	} else {
		vk::Extent2D actualExtent(width, height);

		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}

vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) {
	if (availableFormats.size() == 1 && availableFormats[0].format == vk::Format::eUndefined) {
		return {vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear};
	}

	for (const auto& availableFormat : availableFormats) {
		if (availableFormat.format == vk::Format::eB8G8R8A8Unorm && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
			return availableFormat;
		}
	}

	return availableFormats[0];
}

vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> availablePresentModes) {
	for (const auto& availablePresentMode : availablePresentModes) {
		if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
			return availablePresentMode;
		}
	}

	return vk::PresentModeKHR::eFifo;
}

Device Device::CreateSystemDefaultDevice() MTLPP_AVAILABLE(10_11, 8_0) {
	// Create Instance
	vk::ApplicationInfo appInfo("MAGMA_APP",VK_MAKE_VERSION(1, 0, 0),"MAGMA",VK_MAKE_VERSION(1, 0, 0),VK_API_VERSION_1_0);

    vk::InstanceCreateInfo createInfo;
    createInfo.setPApplicationInfo(&appInfo);

    auto extensions = getRequiredExtensions();

    createInfo.setEnabledExtensionCount(extensions.size());
    createInfo.setPpEnabledExtensionNames(extensions.data());

	if (enableValidationLayers) {
		createInfo.setEnabledLayerCount(validationLayers.size());
		createInfo.setPpEnabledLayerNames(validationLayers.data());
	} else {
		createInfo.setEnabledLayerCount(0);
	}

	Device device;
    device._instance = vk::createInstance(createInfo);

	// Setup Validation layers
    if (enableValidationLayers){
		vk::DebugReportCallbackCreateInfoEXT createInfo;
		createInfo.setFlags(vk::DebugReportFlagBitsEXT::eError | vk::DebugReportFlagBitsEXT::eWarning);
		createInfo.setPfnCallback(debugCallback);

		device._callback = device._instance.createDebugReportCallbackEXT(createInfo,nullptr);
    }

    // Create Surface
    if(window == nullptr)
        throw std::runtime_error("GLFW Window doesn't set");

    VkSurfaceKHR surface;
	if (glfwCreateWindowSurface(device._instance, window, nullptr, &surface) != VK_SUCCESS){
		throw std::runtime_error("failed to create window surface!");
	}
	device._surface = surface;

	// Select physical device
	auto devices = device._instance.enumeratePhysicalDevices();

	if (devices.size() == 0) {
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	bool haveDevice = false;
	for (const auto& pDevice : devices) {
		if (isDeviceSuitable(pDevice,device._surface)) {
			device._pDevice = pDevice;
			haveDevice = true;
			break;
		}
	}

	if (!haveDevice) {
		throw std::runtime_error("failed to find a suitable GPU!");
	}

	// Create logical device
	QueueFamilyIndices indices = queueFamilies(device._pDevice,device._surface);

	std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
	std::set<int> uniqueQueueFamilies = {indices.graphicsFamily, indices.presentFamily};

	float queuePriority = 1.0f;
	for(int queueFamily : uniqueQueueFamilies){
		vk::DeviceQueueCreateInfo queueCreateInfo;
		queueCreateInfo.setQueueFamilyIndex(queueFamily);
		queueCreateInfo.setQueueCount(1);
		queueCreateInfo.setPQueuePriorities(&queuePriority);
		queueCreateInfos.push_back(queueCreateInfo);
	}

	vk::PhysicalDeviceFeatures deviceFeatures;
	deviceFeatures.samplerAnisotropy = true;
	deviceFeatures.tessellationShader = true;
	deviceFeatures.multiViewport = true;
	deviceFeatures.geometryShader = true;

	vk::DeviceCreateInfo deviceCreateInfo;

	deviceCreateInfo.setPQueueCreateInfos(queueCreateInfos.data());
	deviceCreateInfo.setQueueCreateInfoCount(queueCreateInfos.size());

	deviceCreateInfo.setPEnabledFeatures(&deviceFeatures);

	deviceCreateInfo.setPpEnabledExtensionNames(deviceExtensions.data());
	deviceCreateInfo.setEnabledExtensionCount(deviceExtensions.size());

	if (enableValidationLayers) {
		deviceCreateInfo.setEnabledLayerCount(validationLayers.size());
		deviceCreateInfo.setPpEnabledLayerNames(validationLayers.data());
	} else {
		deviceCreateInfo.setEnabledLayerCount(0);
	}

	device._device = device._pDevice.createDevice(deviceCreateInfo);
	device._queue = device._device.getQueue(indices.graphicsFamily,0);

	// Create Swapchain

	SwapchainSupportDetails swapChainSupport = swapchainSupport(device._pDevice,device._surface);

	vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	vk::PresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);

	int width;int height;
	glfwGetWindowSize(window,&width,&height);

	vk::Extent2D extent = chooseSwapExtent(swapChainSupport.capabilities,width,height);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	vk::SwapchainCreateInfoKHR swapchainCreateInfo;
	swapchainCreateInfo.setSurface(surface);

	swapchainCreateInfo.setMinImageCount(imageCount);
	swapchainCreateInfo.setImageFormat(surfaceFormat.format);
	swapchainCreateInfo.setImageColorSpace(vk::ColorSpaceKHR::eSrgbNonlinear);
	swapchainCreateInfo.setImageExtent(extent);
	swapchainCreateInfo.setImageArrayLayers(1);
	swapchainCreateInfo.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);

	uint32_t queueFamilyIndices[] = {(uint32_t) indices.graphicsFamily, (uint32_t) indices.presentFamily};

	if (indices.graphicsFamily != indices.presentFamily) {
		swapchainCreateInfo.setImageSharingMode(vk::SharingMode::eConcurrent);
		swapchainCreateInfo.setQueueFamilyIndexCount(2);
		swapchainCreateInfo.setPQueueFamilyIndices(queueFamilyIndices);
	} else {
		swapchainCreateInfo.setImageSharingMode(vk::SharingMode::eExclusive);
	}

	swapchainCreateInfo.setPreTransform(swapChainSupport.capabilities.currentTransform);
	swapchainCreateInfo.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);
	swapchainCreateInfo.setPresentMode(presentMode);
	swapchainCreateInfo.setClipped(VK_TRUE);

	device._swapchain = device._device.createSwapchainKHR(swapchainCreateInfo);
	device._images = device._device.getSwapchainImagesKHR(device._swapchain);
	device._imageFormat = surfaceFormat.format;
	device._extent = extent;
}

ns::Array<Device> Device::CopyAllDevices() MTLPP_AVAILABLE(10_11, NA) {
	throw std::logic_error("CopyAllDevices Doesn't implement");
}

ns::String Device::GetName() const {
	vk::PhysicalDeviceProperties pdProp = _pDevice.getProperties();
	return pdProp.deviceName;
}

Size       Device::GetMaxThreadsPerThreadgroup() const MTLPP_AVAILABLE(10_11, 9_0) {
	throw std::logic_error("GetMaxThreadsPerThreadgroup Doesn't implement");
}

bool       Device::IsLowPower() const MTLPP_AVAILABLE_MAC(10_11) {
	return false; // By default all device doesn't low power;
}

bool       Device::IsHeadless() const MTLPP_AVAILABLE_MAC(10_11) {
	return false; // By default all device has display
}

uint64_t   Device::GetRecommendedMaxWorkingSetSize() const MTLPP_AVAILABLE_MAC(10_12) {
	vk::PhysicalDeviceProperties pdProp = _pDevice.getProperties();
	return pdProp.limits.maxMemoryAllocationCount; // Max Allocated memory
}

bool       Device::IsDepth24Stencil8PixelFormatSupported() const MTLPP_AVAILABLE_MAC(10_11) {
	vk::FormatProperties props = _pDevice.getFormatProperties(vk::Format::eD24UnormS8Uint);
	if((props.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment) == vk::FormatFeatureFlagBits::eDepthStencilAttachment) {
		return true;
	}
	return false;
}

CommandQueue Device::NewCommandQueue(){

}

CommandQueue Device::NewCommandQueue(uint32_t maxCommandBufferCount){

}

SizeAndAlign Device::HeapTextureSizeAndAlign(const TextureDescriptor& desc) MTLPP_AVAILABLE(NA, 10_0) {

}

SizeAndAlign Device::HeapBufferSizeAndAlign(uint32_t length, ResourceOptions options) MTLPP_AVAILABLE(NA, 10_0) {

}

Heap Device::NewHeap(const HeapDescriptor& descriptor) MTLPP_AVAILABLE(NA, 10_0) {

}

Buffer Device::NewBuffer(uint32_t length, ResourceOptions options) {

}

Buffer Device::NewBuffer(const void* pointer, uint32_t length, ResourceOptions options) {

}

Buffer Device::NewBuffer(void* pointer, uint32_t length, ResourceOptions options, std::function<void (void* pointer, uint32_t length)> deallocator) {

}

DepthStencilState Device::NewDepthStencilState(const DepthStencilDescriptor& descriptor) {

}

Texture Device::NewTexture(const TextureDescriptor& descriptor) {

}

//- (id <MTLTexture>)newTextureWithDescriptor:(MTLTextureDescriptor *)descriptor iosurface:(IOSurfaceRef)iosurface plane:(NSUInteger)plane NS_AVAILABLE_MAC(10_11);
SamplerState Device::NewSamplerState(const SamplerDescriptor& descriptor) {

}

Library Device::NewDefaultLibrary() {

}
//- (nullable id <MTLLibrary>)newDefaultLibraryWithBundle:(NSBundle *)bundle error:(__autoreleasing NSError **)error NS_AVAILABLE(10_12, 10_0);
Library Device::NewLibrary(const ns::String& filepath, ns::Error* error) {

}

Library Device::NewLibrary(const char* source, const CompileOptions& options, ns::Error* error) {

}

void Device::NewLibrary(const char* source, const CompileOptions& options, std::function<void(const Library&, const ns::Error&)> completionHandler) {

}

RenderPipelineState Device::NewRenderPipelineState(const RenderPipelineDescriptor& descriptor, ns::Error* error) {

}

RenderPipelineState Device::NewRenderPipelineState(const RenderPipelineDescriptor& descriptor, PipelineOption options, RenderPipelineReflection* outReflection, ns::Error* error) {

}

void Device::NewRenderPipelineState(const RenderPipelineDescriptor& descriptor, std::function<void(const RenderPipelineState&, const ns::Error&)> completionHandler) {

}

void Device::NewRenderPipelineState(const RenderPipelineDescriptor& descriptor, PipelineOption options, std::function<void(const RenderPipelineState&, const RenderPipelineReflection&, const ns::Error&)> completionHandler) {

}

ComputePipelineState Device::NewComputePipelineState(const Function& computeFunction, ns::Error* error) {

}

ComputePipelineState Device::NewComputePipelineState(const Function& computeFunction, PipelineOption options, ComputePipelineReflection& outReflection, ns::Error* error) {

}

void Device::NewComputePipelineState(const Function& computeFunction, std::function<void(const ComputePipelineState&, const ns::Error&)> completionHandler) {

}

void Device::NewComputePipelineState(const Function& computeFunction, PipelineOption options, std::function<void(const ComputePipelineState&, const ComputePipelineReflection&, const ns::Error&)> completionHandler) {

}

ComputePipelineState Device::NewComputePipelineState(const ComputePipelineDescriptor& descriptor, PipelineOption options, ComputePipelineReflection* outReflection, ns::Error* error) {

}

void Device::NewComputePipelineState(const ComputePipelineDescriptor& descriptor, PipelineOption options, std::function<void(const ComputePipelineState&, const ComputePipelineReflection&, const ns::Error&)> completionHandler) MTLPP_AVAILABLE(10_11, 9_0) {

}

Fence Device::NewFence() MTLPP_AVAILABLE(NA, 10_0) {

}

bool Device::SupportsFeatureSet(FeatureSet featureSet) const {
	return true; // Basic support all features set
}

bool Device::SupportsTextureSampleCount(uint32_t sampleCount) const MTLPP_AVAILABLE(10_11, 9_0) {
	return true; // Basic support all sample count
}
