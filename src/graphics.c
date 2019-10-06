#include "graphics.h"

//NOTE: Instance extensions vs device extensions
#define VK_ERROR_SURFACE_NOT_SUPPORTED 1000

const char* surface_extension = "VK_KHR_surface";
#ifdef _WIN32
const char* platform_surface_extension = "VK_KHR_win32_surface";
#endif
const char* debug_extension = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;

//To add/remove extensions, add entries here
const char* extensions[] =
{
	surface_extension,
	platform_surface_extension,
	debug_extension
};
const uint32_t extension_count = 3;

//To enable/disable validation layers, add entries here
const char* layer_extensions[] =
{
	"VK_LAYER_KHRONOS_validation"
};
const uint32_t layer_extension_count = 1;

const char* device_extensions[] =
{
	"VK_KHR_swapchain" //Parameter
};
const uint32_t device_extension_count = 1;

void print_available_vulkan_extensions()
{
	uint32_t extension_count = 0;
	VkExtensionProperties extensions[16] = {};
	
	vkEnumerateInstanceExtensionProperties(NULL, &extension_count, NULL);
	printf("Extensions = %d\n", extension_count);
	
	vkEnumerateInstanceExtensionProperties(NULL, &extension_count, extensions);
	for(int i = 0; i < extension_count; i++)
	{
		printf("Extension %d: %s\n", i, extensions[i].extensionName);
	}
}

void _print_vulkan_error(const char* function, VkResult error_code)
{
	const char* error;
	switch(error_code)
	{
		case VK_ERROR_OUT_OF_HOST_MEMORY:
			error = "Out of host memory";
			break;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:
			error = "Out of device memory";
			break;
		case VK_ERROR_INITIALIZATION_FAILED:
			error = "Initialization failed";
			break;
		case VK_ERROR_LAYER_NOT_PRESENT:
			error = "Layer not present";
			break;
		case VK_ERROR_EXTENSION_NOT_PRESENT:
			error = "Extension not present";
			break;
		case VK_ERROR_FEATURE_NOT_PRESENT:
			error = "Feature not present";
			break;
		case VK_ERROR_SURFACE_NOT_SUPPORTED:
			error = "Surface not supported";
			break;
		case VK_ERROR_DEVICE_LOST:
			error = "Device lost";
			break;
		case VK_ERROR_SURFACE_LOST_KHR:
			error = "Surface lost";
			break;
		case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
			error = "Surface in use";
			break;
		default:
			error = "Unknown error";
			break;
	}
	printf("Vulkan failed in %s: (%d) %s\n", function, error_code, error);
}

#define print_vulkan_error(str) _print_vulkan_error(__func__, str)

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT message_type, const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void* user_data)
{
	const char* severity_str;
	const char* type_str;
	switch(severity)
	{
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
			severity_str = "diagnostic";
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
			severity_str = "info";
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			severity_str = "warning";
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			severity_str = "error";
			break;
	}
	switch(message_type)
	{
		case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
			type_str = "General";
			break;
		case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
			type_str = "Validation";
			break;
		case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
			type_str = "Performance";
			break;
		default:
			type_str = "N/A";
			break;
	}
	printf("<%s> message with severity <%s>: %s\n", type_str, severity_str, callback_data->pMessage);
}

VkResult create_vulkan_instance(VkInstance* instance)
{
	//CREATE INSTANCE
	//IDEA: Toggle extensions
	VkInstanceCreateInfo create_info = {};

	create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	create_info.enabledExtensionCount = extension_count;
	create_info.ppEnabledExtensionNames = extensions;
	create_info.enabledLayerCount = layer_extension_count;
	create_info.ppEnabledLayerNames = layer_extensions;

	VkApplicationInfo application_info = {};
	application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	application_info.pNext = NULL;
	application_info.apiVersion = VK_API_VERSION_1_1; //Parameter
	create_info.pApplicationInfo = &application_info;

	VkResult result = vkCreateInstance(&create_info, NULL, instance);
	if(result != VK_SUCCESS) print_vulkan_error(result);

	return result;
}

VkResult create_debug_messenger(VkDebugUtilsMessengerEXT* debug_messenger, VkInstance instance)
{
	PFN_vkCreateDebugUtilsMessengerEXT vk_func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if(vk_func != NULL)
	{
		VkDebugUtilsMessengerCreateInfoEXT messenger_info = {};
		messenger_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		messenger_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		messenger_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		messenger_info.pfnUserCallback = debug_callback;
		messenger_info.pUserData = NULL;
		return vk_func(instance, &messenger_info, NULL, debug_messenger);
	}
	else return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void destroy_debug_messenger(VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger)
{
	PFN_vkDestroyDebugUtilsMessengerEXT vk_func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if(vk_func) vk_func(instance, debug_messenger, NULL);
}

VkResult query_for_physical_device(VkPhysicalDevice* device, VkInstance instance)
{
	//QUERY PHYSICAL DEVICE
	uint32_t device_count = 0;
	//Get number of devices
	vkEnumeratePhysicalDevices(instance, &device_count, NULL);
	if(device_count == 0)
	{
		printf("Queried for physical devices, found none\n");
		return VK_ERROR_INITIALIZATION_FAILED;
	}

	VkResult result = vkEnumeratePhysicalDevices(instance, &device_count, device);
	if(result != VK_SUCCESS) print_vulkan_error(result);
	return result;
}

VkResult create_window_surface(VkSurfaceKHR* surface, HWND window, HINSTANCE hinstance, VkInstance instance, VkPhysicalDevice device, int queue_index)
{
	//CREATE WINDOW SURFACE (PLATFORM SPECIFIC)
	VkWin32SurfaceCreateInfoKHR surface_create_info = {};
	surface_create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR; //Parameter (platform specific)
	surface_create_info.hwnd = window;
	surface_create_info.hinstance = hinstance;
	VkResult result = vkCreateWin32SurfaceKHR(instance, &surface_create_info, NULL, surface);
	if(result != VK_SUCCESS) print_vulkan_error(result);
	else
	{
		VkBool32 supported;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, queue_index, *surface, &supported); //This needs to be moved to a proper place (since currently it is here to suppress a warning from validation layer)
		result = (supported) ? VK_SUCCESS : (VkResult)VK_ERROR_SURFACE_NOT_SUPPORTED;
		if(result != VK_SUCCESS) print_vulkan_error(result);
		return result;
	}
	return result;
}

void get_device_queue_family_properties(VkPhysicalDevice device, uint32_t* queue_family_count, VkQueueFamilyProperties* queue_families)
{
	vkGetPhysicalDeviceQueueFamilyProperties(device, queue_family_count, NULL);
	vkGetPhysicalDeviceQueueFamilyProperties(device, queue_family_count, queue_families);
}

uint32_t find_graphics_queue(VkPhysicalDevice device)
{
	uint32_t queue_family_count = 0;
	VkQueueFamilyProperties queue_families[8] = {};
	get_device_queue_family_properties(device, &queue_family_count, queue_families);
	for(uint32_t i = 0; i < queue_family_count; i++)
	{
		if(queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) return i;
	}
}

uint32_t find_transfer_queue(VkPhysicalDevice device)
{
	uint32_t queue_family_count = 0;
	VkQueueFamilyProperties queue_families[8] = {};
	get_device_queue_family_properties(device, &queue_family_count, queue_families);
	for(uint32_t i = 0; i < queue_family_count; i++)
	{
		if((queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT) && !(queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && !(queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT)) return i;
	}
}

VkResult create_logical_device(VkDevice* logical_device, uint32_t* graphics_queue_index, VkQueue* graphics_queue, uint32_t* transfer_queue_index, VkQueue* transfer_queue, VkPhysicalDevice device)
{
	//IDEA: Move queue creation/queries to its own functions 
	//QUERY DEVICE QUEUES
	*graphics_queue_index = find_graphics_queue(device);
	*transfer_queue_index = find_transfer_queue(device);

	//Need to specify number of queues when creating the logical device
	//Logical device is essentially an interface to the physical device
	float priority = 1.0f; 

	VkDeviceQueueCreateInfo queue_create_info = {};
	queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_create_info.queueFamilyIndex = *graphics_queue_index; 
	queue_create_info.queueCount = 1; 
	queue_create_info.pQueuePriorities = &priority; //Parameter - dependent on queue count
	VkDeviceQueueCreateInfo transfer_queue_create_info = {};
	transfer_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	transfer_queue_create_info.queueFamilyIndex = *transfer_queue_index;
	transfer_queue_create_info.queueCount = 1;
	transfer_queue_create_info.pQueuePriorities = &priority;

	//QUERY PHYSICAL DEVICE FEATURES
	VkPhysicalDeviceFeatures device_features = {};
	
	VkDeviceQueueCreateInfo queue_create_infos[] = {queue_create_info, transfer_queue_create_info};
	//CREATE LOGICAL DEVICE
	VkDeviceCreateInfo device_create_info = {};
	device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_create_info.pQueueCreateInfos = queue_create_infos;
	device_create_info.queueCreateInfoCount = 2;
	device_create_info.pEnabledFeatures = &device_features;
	
	device_create_info.enabledExtensionCount = device_extension_count;
	device_create_info.ppEnabledExtensionNames = device_extensions;

	VkResult result = vkCreateDevice(device, &device_create_info, NULL, logical_device);
	if(result != VK_SUCCESS) print_vulkan_error(result);
	//CREATE GRAPHICS QUEUE REFERENCE
	else 
	{
		vkGetDeviceQueue(*logical_device, *graphics_queue_index, 0, graphics_queue);
		vkGetDeviceQueue(*logical_device, *transfer_queue_index, 0, transfer_queue);
	}
	return result;
}

bool swapchain_format_supported(VkPhysicalDevice physical_device, VkSurfaceKHR surface, VkFormat format)
{
	uint32_t format_count;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, NULL);
	VkSurfaceFormatKHR formats[128] = {};
	vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, formats);
	for(uint32_t i = 0; i < format_count; i++) if(formats[i].format == format) return true;
	return false;
}

//NOTE: Should swapchain creation be dependent specifically on surface capabilities
//Do I even need to make swapchain creation more generic? My requirements for it seem static
VkResult create_swapchain(VkSwapchainKHR* swapchain, VkExtent2D swapchain_extent, uint32_t swapchain_image_count, VkPhysicalDevice device, VkDevice logical_device, VkSurfaceKHR surface, VkSurfaceTransformFlagBitsKHR pre_transform, VkFormat swapchain_image_format)
{
	//CREATE SWAPCHAIN
	VkSwapchainCreateInfoKHR swapchain_create_info = {};
	swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchain_create_info.surface = surface;
	swapchain_create_info.minImageCount = swapchain_image_count; //Parameter
	swapchain_create_info.imageFormat = swapchain_image_format;
	swapchain_create_info.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	swapchain_create_info.imageExtent = swapchain_extent;
	swapchain_create_info.imageArrayLayers = 1; //Only > 1 when on multiview/stereo surface
	swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchain_create_info.preTransform = pre_transform;
	swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchain_create_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
	swapchain_create_info.clipped = VK_TRUE;
	swapchain_create_info.oldSwapchain = VK_NULL_HANDLE;

	VkResult result = vkCreateSwapchainKHR(logical_device, &swapchain_create_info, NULL, swapchain);
	if(result != VK_SUCCESS) print_vulkan_error(result);
	return result;
}

VkResult create_render_pass(VkRenderPass* render_pass, VkDevice logical_device, VkFormat swapchain_image_format)
{
	VkAttachmentDescription colour_attachment = {};
	colour_attachment.format = swapchain_image_format;
	colour_attachment.samples = VK_SAMPLE_COUNT_1_BIT; //Parameter (may want to multisample/aa)
	colour_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colour_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colour_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colour_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colour_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colour_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	//DESCRIBE SUBPASS
	VkAttachmentReference colour_attachment_ref = {};
	colour_attachment_ref.attachment = 0;
	colour_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colour_attachment_ref;

	VkRenderPassCreateInfo render_pass_info = {};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_info.attachmentCount = 1;
	render_pass_info.pAttachments = &colour_attachment;
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass;
	
	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	render_pass_info.dependencyCount = 1;
	render_pass_info.pDependencies = &dependency;
	VkResult result = vkCreateRenderPass(logical_device, &render_pass_info, NULL, render_pass);
	if(result != VK_SUCCESS) print_vulkan_error(result);
	return result;
}

VkResult create_swapchain_image_view(VkImageView* swapchain_image_view, VkImage* swapchain_image, VkDevice logical_device, VkFormat swapchain_image_format)
{
	VkImageViewCreateInfo image_view_create_info = {};
	image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	image_view_create_info.image = *swapchain_image;
	image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	image_view_create_info.format = swapchain_image_format;
	image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	image_view_create_info.subresourceRange.baseMipLevel = 0;
	image_view_create_info.subresourceRange.levelCount = 1;
	image_view_create_info.subresourceRange.baseArrayLayer = 0;
	image_view_create_info.subresourceRange.layerCount = 1;
	VkResult result = vkCreateImageView(logical_device, &image_view_create_info, NULL, swapchain_image_view);
	if(result != VK_SUCCESS) print_vulkan_error(result);
	return result;
}

VkResult create_framebuffer(VkFramebuffer* swapchain_framebuffer, VkDevice* logical_device, VkRenderPass* render_pass, VkImageView* swapchain_image_view, VkExtent2D swapchain_extent)
{
	VkFramebufferCreateInfo framebuffer_info = {};
	framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebuffer_info.renderPass = *render_pass;
	framebuffer_info.attachmentCount = 1;
	framebuffer_info.pAttachments = swapchain_image_view;
	framebuffer_info.width = swapchain_extent.width;
	framebuffer_info.height = swapchain_extent.height;
	framebuffer_info.layers = 1;

	VkResult result = vkCreateFramebuffer(*logical_device, &framebuffer_info, NULL, swapchain_framebuffer);
	if(result != VK_SUCCESS) print_vulkan_error(result);
	return result;
}

VkResult create_command_pool(VkCommandPool* command_pool, VkDevice* logical_device, uint32_t graphics_queue_index)
{
	VkCommandPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_info.queueFamilyIndex = graphics_queue_index;
	pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	
	VkResult result = vkCreateCommandPool(*logical_device, &pool_info, NULL, command_pool);
	if(result != VK_SUCCESS) print_vulkan_error(result);
	return result;
}

VkResult create_command_buffers(VkCommandBuffer* command_buffers, VkDevice* logical_device, VkCommandPool* command_pool, uint32_t command_buffer_count)
{
	VkCommandBufferAllocateInfo command_alloc_info = {};
	command_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	command_alloc_info.commandPool = *command_pool;
	command_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	command_alloc_info.commandBufferCount = command_buffer_count;

	VkResult result = vkAllocateCommandBuffers(*logical_device, &command_alloc_info, command_buffers);
	if(result != VK_SUCCESS) print_vulkan_error(result);
	return result;
}

VkShaderModule create_shader_module(VkDevice logical_device, const char* path)
{
	FILE* f = fopen(path, "rb");
	if(!f) printf("File I/O blows\n");
	fseek(f, 0, SEEK_END);
	int size = ftell(f);
	fseek(f, 0, SEEK_SET);

	char* file_contents = (char*)malloc(size+1);
	fread(file_contents, 1, size, f);
	fclose(f);
	file_contents[size] = 0;
	
	VkShaderModuleCreateInfo shader_module_create_info = {};
	shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shader_module_create_info.codeSize = size;
	shader_module_create_info.pCode = (const uint32_t*)file_contents;
	
	VkShaderModule shader_module;
	if(vkCreateShaderModule(logical_device, &shader_module_create_info, NULL, &shader_module) != VK_SUCCESS)
	{
		printf("Shaders suck\n");
	}
	free(file_contents);
	return shader_module;
}

VkResult create_graphics_pipeline_layout(VkPipelineLayout* pipeline_layout, VkDevice logical_device, VkDescriptorSetLayout descriptor_set_layout)
{
	//CREATE PUSH CONSTANT
	VkPushConstantRange push_constant_range = {};
	push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	push_constant_range.offset = 0;
	push_constant_range.size = 16*sizeof(float);

	//CREATE PIPELINE LAYOUT
	VkPipelineLayoutCreateInfo pipeline_layout_info = {};
	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.setLayoutCount = 1;
	pipeline_layout_info.pSetLayouts = &descriptor_set_layout;
	pipeline_layout_info.pushConstantRangeCount = 1;
	pipeline_layout_info.pPushConstantRanges = &push_constant_range;
	
	VkResult result = vkCreatePipelineLayout(logical_device, &pipeline_layout_info, NULL, pipeline_layout);
	if(result != VK_SUCCESS)
	{
		printf("Pipeline layout creation failed\n");
		print_vulkan_error(result);
		return result;
	}
	return result;
}

VkVertexInputBindingDescription vertex_input_binding_description(uint32_t binding, uint32_t stride, VkVertexInputRate input_rate = VK_VERTEX_INPUT_RATE_VERTEX)
{
	VkVertexInputBindingDescription vertex_binding_description = {};
	vertex_binding_description.binding = binding;
	vertex_binding_description.stride = stride;
	vertex_binding_description.inputRate = input_rate;
	return vertex_binding_description;
}

VkVertexInputAttributeDescription vertex_attribute(uint32_t binding, uint32_t location, VkFormat format, uint32_t offset)
{
	VkVertexInputAttributeDescription vertex_attribute_description = {};
	vertex_attribute_description.binding = binding;
	vertex_attribute_description.location = location;
	vertex_attribute_description.format = format;
	vertex_attribute_description.offset = offset;
	return vertex_attribute_description;
}

VkResult create_graphics_pipeline(VkPipeline* graphics_pipeline, VkDevice logical_device, VkExtent2D swapchain_extent, VkRenderPass render_pass, VkDescriptorSetLayout descriptor_set_layout, VkPipelineLayout pipeline_layout, VkPrimitiveTopology topology, VkVertexInputBindingDescription* vertex_binding_descriptions, uint32_t vertex_binding_description_count, VkVertexInputAttributeDescription* vertex_attributes, uint32_t vertex_attribute_count, const char* vertex_shader_path, const char* fragment_shader_path)
{
	//CREATE SHADER MODULES AND PIPELINE STAGES
	VkShaderModule vertex_shader_module = create_shader_module(logical_device, vertex_shader_path);
	VkShaderModule fragment_shader_module = create_shader_module(logical_device, fragment_shader_path);
	VkPipelineShaderStageCreateInfo vertex_shader_stage_info = {};
	vertex_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertex_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertex_shader_stage_info.module = vertex_shader_module;
	vertex_shader_stage_info.pName = "main";

	VkPipelineShaderStageCreateInfo fragment_shader_stage_info = {};
	fragment_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragment_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragment_shader_stage_info.module = fragment_shader_module;
	fragment_shader_stage_info.pName = "main";

	VkPipelineShaderStageCreateInfo shader_stages[2] = {vertex_shader_stage_info, fragment_shader_stage_info};


	//DESCRIBE VERTEX SHADER INPUT
	VkPipelineVertexInputStateCreateInfo vertex_input_info = {};

	vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_input_info.vertexBindingDescriptionCount = vertex_binding_description_count;
	vertex_input_info.pVertexBindingDescriptions = vertex_binding_descriptions;
	vertex_input_info.vertexAttributeDescriptionCount = vertex_attribute_count;
	vertex_input_info.pVertexAttributeDescriptions = vertex_attributes;

	//DESCRIBE TOPOLOGY OF VERTEX SHADER INPUT
	VkPipelineInputAssemblyStateCreateInfo input_assembly_info = {};
	input_assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly_info.topology = topology;
	input_assembly_info.primitiveRestartEnable = VK_FALSE;
	
	//DESCRIBE VIEWPORT
	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = (float)swapchain_extent.height;
	viewport.width = (float)swapchain_extent.width;
	viewport.height = -(float)swapchain_extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = {0, 0};
	scissor.extent = swapchain_extent;

	//CREATE VIEWPORT
	VkPipelineViewportStateCreateInfo viewport_state_info = {};
	viewport_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state_info.viewportCount = 1;
	viewport_state_info.pViewports = &viewport;
	viewport_state_info.scissorCount = 1;
	viewport_state_info.pScissors = &scissor;

	//DESCRIBE RASTERISER
	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	//DESCRIBE MULTISAMPLING
	VkPipelineMultisampleStateCreateInfo multisample_info = {};
	multisample_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisample_info.sampleShadingEnable = VK_FALSE;
	multisample_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	//DESCRIBE COLOUR BLENDING
	VkPipelineColorBlendAttachmentState colour_blend_attach_info = {};
	colour_blend_attach_info.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colour_blend_attach_info.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colour_blending_info = {};
	colour_blending_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colour_blending_info.logicOpEnable = VK_FALSE;
	colour_blending_info.logicOp = VK_LOGIC_OP_COPY;
	colour_blending_info.attachmentCount = 1;
	colour_blending_info.pAttachments = &colour_blend_attach_info;


	//CONSTRUCT GRAPHICS PIPELINE
	VkGraphicsPipelineCreateInfo pipeline_info = {};
	pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_info.stageCount = 2;
	pipeline_info.pStages = shader_stages;
	pipeline_info.pVertexInputState = &vertex_input_info;
	pipeline_info.pInputAssemblyState = &input_assembly_info;
	pipeline_info.pViewportState = &viewport_state_info;
	pipeline_info.pRasterizationState = &rasterizer;
	pipeline_info.pMultisampleState = &multisample_info;
	pipeline_info.pColorBlendState = &colour_blending_info;
	pipeline_info.layout = pipeline_layout;
	pipeline_info.renderPass = render_pass;
	pipeline_info.subpass = 0;

	VkResult result = vkCreateGraphicsPipelines(logical_device, VK_NULL_HANDLE, 1, &pipeline_info, NULL, graphics_pipeline);
	if(result != VK_SUCCESS)
	{
		print_vulkan_error(result);
	}

	vkDestroyShaderModule(logical_device, fragment_shader_module, NULL);
	vkDestroyShaderModule(logical_device, vertex_shader_module, NULL);

	return result;
}

VkResult create_uniform_descriptor_pool(VkDescriptorPool* descriptor_pool, VkDevice* logical_device, int descriptor_count)
{
	VkDescriptorPoolSize pool_size = {};
	pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	pool_size.descriptorCount = descriptor_count;

	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.poolSizeCount = 1;
	pool_info.pPoolSizes = &pool_size;
	pool_info.maxSets = descriptor_count;

	VkResult result = vkCreateDescriptorPool(*logical_device, &pool_info, NULL, descriptor_pool);
	if(result != VK_SUCCESS) print_vulkan_error(result);
	return result;
}

VkResult create_descriptor_sets(VkDescriptorSet* descriptor_sets, VkDevice* logical_device, VkDescriptorPool* descriptor_pool, VkBuffer* descriptor_set_buffers, VkDescriptorSetLayout descriptor_set_layout, int descriptor_set_count, int descriptor_set_buffer_range)
{
	VkDescriptorSetLayout descriptor_set_layouts[4] = {};
	for(int i = 0; i < descriptor_set_count; i++) descriptor_set_layouts[i] = descriptor_set_layout;

	VkDescriptorSetAllocateInfo descriptor_set_alloc_info = {};
	descriptor_set_alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptor_set_alloc_info.descriptorPool = *descriptor_pool;
	descriptor_set_alloc_info.descriptorSetCount = descriptor_set_count;
	descriptor_set_alloc_info.pSetLayouts = descriptor_set_layouts;
	VkResult result = vkAllocateDescriptorSets(*logical_device, &descriptor_set_alloc_info, descriptor_sets);
	if(result != VK_SUCCESS)
	{
		print_vulkan_error(result);
		return result;
	}
	for(int i = 0; i < descriptor_set_count; i++)
	{
		VkDescriptorBufferInfo descriptor_buffer_info = {};
		descriptor_buffer_info.buffer = descriptor_set_buffers[i];
		descriptor_buffer_info.offset = 0;
		descriptor_buffer_info.range = descriptor_set_buffer_range;

		VkWriteDescriptorSet descriptor_set_write = {};
		descriptor_set_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptor_set_write.dstSet = descriptor_sets[i];
		descriptor_set_write.dstBinding = 0;
		descriptor_set_write.dstArrayElement = 0;
		descriptor_set_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptor_set_write.descriptorCount = 1;
		descriptor_set_write.pBufferInfo = &descriptor_buffer_info;
		vkUpdateDescriptorSets(*logical_device, 1, &descriptor_set_write, 0, NULL);
	}
	return result;
}

VkResult create_semaphore(VkSemaphore* semaphore, VkDevice* logical_device)
{
	VkSemaphoreCreateInfo semaphore_info = {};
	semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkResult result = vkCreateSemaphore(*logical_device, &semaphore_info, NULL, semaphore);
	if(result != VK_SUCCESS) print_vulkan_error(result);
	return result;
}

VkResult create_fence(VkFence* fence, VkDevice* logical_device)
{
	VkFenceCreateInfo fence_info = {};
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	
	VkResult result = vkCreateFence(*logical_device, &fence_info, NULL, fence);
	if(result != VK_SUCCESS) print_vulkan_error(result);
	return result;
}

VkDescriptorSetLayout create_world_matrix_descriptor_set_layout(VkDevice logical_device)
{
	VkDescriptorSetLayoutBinding world_matrices_layout_binding = {};
	world_matrices_layout_binding.binding = 0;
	world_matrices_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	world_matrices_layout_binding.descriptorCount = 1;
	world_matrices_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutCreateInfo descriptor_set_layout_info = {};
	descriptor_set_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptor_set_layout_info.bindingCount = 1;
	descriptor_set_layout_info.pBindings = &world_matrices_layout_binding;

	VkDescriptorSetLayout set_layout = {};
	if(vkCreateDescriptorSetLayout(logical_device, &descriptor_set_layout_info, NULL, &set_layout) != VK_SUCCESS)
	{
		printf("Can't create descriptor set layout\n");
	}
	return set_layout;
}

uint32_t find_memory_type(VkPhysicalDevice* device, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties mem_properties;
	vkGetPhysicalDeviceMemoryProperties(*device, &mem_properties);
	for(uint32_t i = 0; i < mem_properties.memoryTypeCount; i++)
	{
		if((mem_properties.memoryTypes[i].propertyFlags & properties) == properties) 
		{
			return i;
		}
	}
	return -1;
}

VkResult create_buffer(VkBuffer* buffer, VkDeviceMemory* buffer_memory, VkDevice* logical_device, uint32_t buffer_size, uint32_t buffer_offset, VkBufferUsageFlags usage, const uint32_t* queue_families, uint32_t queue_family_count)
{
	VkBufferCreateInfo buffer_create_info = {};
	buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_create_info.size = buffer_size;
	buffer_create_info.usage = usage;
	buffer_create_info.sharingMode = (queue_family_count > 1) ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
	buffer_create_info.queueFamilyIndexCount = queue_family_count;
	buffer_create_info.pQueueFamilyIndices = queue_families;
	
	VkResult result = vkCreateBuffer(*logical_device, &buffer_create_info, NULL, buffer);
	if(result != VK_SUCCESS) 
	{
		print_vulkan_error(result);
		return result;
	}
	VkMemoryRequirements mem_requirements = {};
	vkGetBufferMemoryRequirements(*logical_device, *buffer, &mem_requirements);
	vkBindBufferMemory(*logical_device, *buffer, *buffer_memory, buffer_offset);
	return result;
}

VkResult allocate_buffer_memory(VkDeviceMemory* memory, VkPhysicalDevice* physical_device, VkDevice* logical_device, uint32_t size, VkMemoryPropertyFlags memory_type)
{
	VkMemoryAllocateInfo allocate_info = {};
	allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocate_info.allocationSize = size;
	allocate_info.memoryTypeIndex = find_memory_type(physical_device, memory_type);
	
	VkResult result = vkAllocateMemory(*logical_device, &allocate_info, NULL, memory);
	if(result != VK_SUCCESS) print_vulkan_error(result);
	return result;
}

void copy_data_to_buffer(VkDevice logical_device, VkDeviceMemory buffer_memory, size_t size_of_buffer, void* data_to_copy)
{
	void* data;
	vkMapMemory(logical_device, buffer_memory, 0, size_of_buffer, 0, &data);
	memcpy(data, data_to_copy, size_of_buffer);
	vkUnmapMemory(logical_device, buffer_memory);
}

void copy_data_between_buffers(VkBuffer* src_buffer, VkBuffer* dst_buffer, uint32_t copy_size, VkCommandBuffer* transfer_command_buffer, VkQueue* transfer_queue)
{
	VkCommandBufferBeginInfo transfer_begin_info = {};
	transfer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	transfer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(*transfer_command_buffer, &transfer_begin_info);
	VkBufferCopy copy_region = {};
	copy_region.srcOffset = 0;
	copy_region.dstOffset = 0;
	copy_region.size = copy_size;
	vkCmdCopyBuffer(*transfer_command_buffer, *src_buffer, *dst_buffer, 1, &copy_region);
	vkEndCommandBuffer(*transfer_command_buffer);

	VkSubmitInfo submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = transfer_command_buffer;
	vkQueueSubmit(*transfer_queue, 1, &submit_info, VK_NULL_HANDLE);
	vkQueueWaitIdle(*transfer_queue);
}

graphical_data_buffer buffer_graphical_data(vulkan_state* vulkan, vertex* vertices, int vertex_count)
{
	uint32_t queue_families[] = {vulkan->graphics_queue_index, vulkan->transfer_queue_index};
	size_t size_of_vertex_buffer = vertex_count*sizeof(vertex);
	VkBuffer vertex_buffer;

	VkResult result = create_buffer(&vertex_buffer, &vulkan->gpu_memory, &vulkan->logical_device, size_of_vertex_buffer, vulkan->device_local_mem_in_use, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, queue_families, 2);
	if(result != VK_SUCCESS) printf("Vertex buffer not created\n");
	vulkan->device_local_mem_in_use += size_of_vertex_buffer;
	
	copy_data_to_buffer(vulkan->logical_device, vulkan->staging_buffer_memory, size_of_vertex_buffer, vertices);
	copy_data_between_buffers(&vulkan->staging_buffer, &vertex_buffer, size_of_vertex_buffer, &vulkan->transfer_command_buffer, &vulkan->transfer_queue);
	graphical_data_buffer buffer = {vertex_buffer, VkBuffer{}, vertex_count, 0};
	return buffer;
}

graphical_data_buffer buffer_graphical_data(vulkan_state* vulkan, vertex* vertices, int vertex_count, uint16_t* indices, int index_count)
{
	uint32_t queue_families[] = {vulkan->graphics_queue_index, vulkan->transfer_queue_index};
	size_t size_of_vertex_buffer = vertex_count*sizeof(vertex);
	VkBuffer vertex_buffer;
	
	VkResult result = create_buffer(&vertex_buffer, &vulkan->gpu_memory, &vulkan->logical_device, size_of_vertex_buffer, vulkan->device_local_mem_in_use, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, queue_families, 2);
	if(result != VK_SUCCESS) printf("Vertex buffer not created\n");
	vulkan->device_local_mem_in_use += size_of_vertex_buffer;

	size_t size_of_index_buffer = index_count*sizeof(uint16_t);
	VkBuffer index_buffer;

	result = create_buffer(&index_buffer, &vulkan->gpu_memory, &vulkan->logical_device, size_of_index_buffer, vulkan->device_local_mem_in_use, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, queue_families, 2);
	if(result != VK_SUCCESS) printf("Index buffer not created\n");
	vulkan->device_local_mem_in_use += size_of_index_buffer;

	copy_data_to_buffer(vulkan->logical_device, vulkan->staging_buffer_memory, size_of_vertex_buffer, vertices);
	copy_data_between_buffers(&vulkan->staging_buffer, &vertex_buffer, size_of_vertex_buffer, &vulkan->transfer_command_buffer, &vulkan->transfer_queue);
	copy_data_to_buffer(vulkan->logical_device, vulkan->staging_buffer_memory, size_of_index_buffer, indices);
	copy_data_between_buffers(&vulkan->staging_buffer, &index_buffer, size_of_index_buffer, &vulkan->transfer_command_buffer, &vulkan->transfer_queue);

	graphical_data_buffer buffer = {vertex_buffer, index_buffer, vertex_count, index_count};
	return buffer;
}

void destroy_graphical_data(vulkan_state* vulkan, graphical_data_buffer* data)
{
	if(data->index_count > 0) vkDestroyBuffer(vulkan->logical_device, data->index_buffer, NULL);
	vkDestroyBuffer(vulkan->logical_device, data->vertex_buffer, NULL);
}

uint32_t create_swapchain_dependent_components(vulkan_state* vulkan)
{
	VkSurfaceCapabilitiesKHR surface_capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vulkan->physical_device, vulkan->surface, &surface_capabilities);

	VkFormat swapchain_image_format = VK_FORMAT_R8G8B8A8_UNORM;
	if(!swapchain_format_supported(vulkan->physical_device, vulkan->surface, swapchain_image_format))
	{
		printf("Given format not available\n"); 
		return 1;
	}

	vulkan->swapchain_image_count = 3;
	vulkan->swapchain_extent = surface_capabilities.currentExtent;

	//*CREATE SWAPCHAIN
	VkResult vulkan_procedure_result = create_swapchain(&vulkan->swapchain, vulkan->swapchain_extent, vulkan->swapchain_image_count, vulkan->physical_device, vulkan->logical_device, vulkan->surface, surface_capabilities.currentTransform, swapchain_image_format);
	if(vulkan_procedure_result != VK_SUCCESS) return 5;

	//*CREATE SWAPCHAIN IMAGE VIEWS
	VkImage swapchain_images[4] = {};
	uint32_t swapchain_image_count;
	vkGetSwapchainImagesKHR(vulkan->logical_device, vulkan->swapchain, &swapchain_image_count, NULL);
	printf("Retrieved swapchain image count = %d\n", swapchain_image_count);
	vkGetSwapchainImagesKHR(vulkan->logical_device, vulkan->swapchain, &vulkan->swapchain_image_count, swapchain_images);
	for(int i = 0; i < vulkan->swapchain_image_count; i++)
	{
		vulkan_procedure_result = create_swapchain_image_view(&vulkan->swapchain_image_views[i], &swapchain_images[i], vulkan->logical_device, swapchain_image_format);
		if(vulkan_procedure_result != VK_SUCCESS)
		{
			printf("Failed on swapchain image %d\n", i);
			return 6;
		}
	}


	//*CREATE FRAMEBUFFERS
	for(int i = 0; i < vulkan->swapchain_image_count; i++)
	{
		vulkan_procedure_result = create_framebuffer(&vulkan->swapchain_framebuffers[i], &vulkan->logical_device, &vulkan->render_pass, &vulkan->swapchain_image_views[i], vulkan->swapchain_extent);
		if(vulkan_procedure_result != VK_SUCCESS) return 11;
	}

	//*CREATE GRAPHICS PIPELINE
	VkVertexInputBindingDescription vertex_binding_description = vertex_input_binding_description(0, sizeof(vertex));

	VkVertexInputAttributeDescription vertex_attributes[] = 
	{
		vertex_attribute(0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(vertex, position)),
		vertex_attribute(0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(vertex, colour))
	};
	vulkan_procedure_result = create_graphics_pipeline(&vulkan->graphics_pipeline, vulkan->logical_device, surface_capabilities.currentExtent, vulkan->render_pass, vulkan->uniform_descriptor_set_layout, vulkan->pipeline_layout, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, &vertex_binding_description, 1, vertex_attributes, 2, "..\\src\\vert.spv", "..\\src\\frag.spv");
	if(vulkan_procedure_result != VK_SUCCESS) return 9;

	vulkan_procedure_result = create_graphics_pipeline(&vulkan->line_graphics_pipeline, vulkan->logical_device, surface_capabilities.currentExtent, vulkan->render_pass, vulkan->uniform_descriptor_set_layout, vulkan->pipeline_layout, VK_PRIMITIVE_TOPOLOGY_LINE_LIST, &vertex_binding_description, 1, vertex_attributes, 2, "..\\src\\line_vert.spv", "..\\src\\line_frag.spv");
	if(vulkan_procedure_result != VK_SUCCESS) return 10;
}

void resize_window(vulkan_state* vulkan)
{
	for(int i = 0; i < vulkan->swapchain_image_count; i++) vkDestroyFramebuffer(vulkan->logical_device, vulkan->swapchain_framebuffers[i], NULL); //*
	vkDestroyPipeline(vulkan->logical_device, vulkan->line_graphics_pipeline, NULL); //*
	vkDestroyPipeline(vulkan->logical_device, vulkan->graphics_pipeline, NULL); //*
	for(int i = 0; i < vulkan->swapchain_image_count; i++) vkDestroyImageView(vulkan->logical_device, vulkan->swapchain_image_views[i], NULL); //*
	vkDestroySwapchainKHR(vulkan->logical_device, vulkan->swapchain, NULL); //*

	VkSurfaceCapabilitiesKHR surface_capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vulkan->physical_device, vulkan->surface, &surface_capabilities);

	VkFormat swapchain_image_format = VK_FORMAT_R8G8B8A8_UNORM;
	vulkan->swapchain_image_count = 3;
	vulkan->swapchain_extent = surface_capabilities.currentExtent;

	create_swapchain_dependent_components(vulkan);
}

//What I want:
//	- If function succeeded or not
//	- If it didn't, why not
//	- So that the program can be fixed/made to handle problem
//	- Consistency in function calls/returns/structures
uint8_t startup_vulkan(vulkan_state* vulkan, HWND window, HINSTANCE hinstance)
{
	//Vulkan stuff
	print_available_vulkan_extensions();
	VkResult vulkan_procedure_result = create_vulkan_instance(&vulkan->instance);
	if(vulkan_procedure_result != VK_SUCCESS) return 1; //Failed

	vulkan_procedure_result = create_debug_messenger(&vulkan->debug_messenger, vulkan->instance);
	if(vulkan_procedure_result != VK_SUCCESS) return 100;

	vulkan_procedure_result = query_for_physical_device(&vulkan->physical_device, vulkan->instance);
	if(vulkan_procedure_result != VK_SUCCESS) return 2;
	
	//Create logical device with queue(s)
	vulkan_procedure_result = create_logical_device(&vulkan->logical_device, &vulkan->graphics_queue_index, &vulkan->graphics_queue, &vulkan->transfer_queue_index, &vulkan->transfer_queue, vulkan->physical_device);
	if(vulkan_procedure_result != VK_SUCCESS) return 3;

	//Create surface
	vulkan_procedure_result = create_window_surface(&vulkan->surface, window, hinstance, vulkan->instance, vulkan->physical_device, vulkan->graphics_queue_index);
	if(vulkan_procedure_result != VK_SUCCESS) return 4;
	
	VkSurfaceCapabilitiesKHR surface_capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vulkan->physical_device, vulkan->surface, &surface_capabilities);

	VkFormat swapchain_image_format = VK_FORMAT_R8G8B8A8_UNORM;
	vulkan->swapchain_image_count = 3;
	vulkan->swapchain_extent = surface_capabilities.currentExtent;

	//CREATE RENDER PASS
	vulkan_procedure_result = create_render_pass(&vulkan->render_pass, vulkan->logical_device, swapchain_image_format);

	if(vulkan_procedure_result != VK_SUCCESS) return 7;

	//CREATE STAGING BUFFER
	vulkan_procedure_result = allocate_buffer_memory(&vulkan->staging_buffer_memory, &vulkan->physical_device, &vulkan->logical_device, KILOBYTES(256), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	if(vulkan_procedure_result != VK_SUCCESS)
	{
		printf("Unable to allocate staging buffer memory\n");
		return 24;
	}
	
	vulkan_procedure_result = create_buffer(&vulkan->staging_buffer, &vulkan->staging_buffer_memory, &vulkan->logical_device, KILOBYTES(256), 0, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, &vulkan->transfer_queue_index, 1);
	if(vulkan_procedure_result != VK_SUCCESS) return 25;

	//CREATE GPU LOCAL BUFFER
	vulkan_procedure_result = allocate_buffer_memory(&vulkan->gpu_memory, &vulkan->physical_device, &vulkan->logical_device, MEGABYTES(256), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	if(vulkan_procedure_result != VK_SUCCESS)
	{
		printf("Unable to allocate device local memory\n");
		return 26;
	}
	vulkan->device_local_mem_in_use = 0;

	//CREATE UNIFORM DESCRIPTOR SET
	vulkan->uniform_descriptor_set_layout = create_world_matrix_descriptor_set_layout(vulkan->logical_device);

	//CREATE UNIFORM BUFFERS
	VkDeviceSize uniform_buffer_size = sizeof(world_matrices);
	
	for(int i = 0; i < vulkan->swapchain_image_count; i++)
	{
		uint32_t queue_families[] = {vulkan->graphics_queue_index, vulkan->transfer_queue_index};
		vulkan_procedure_result = create_buffer(&vulkan->world_matrix_buffers[i], &vulkan->gpu_memory, &vulkan->logical_device, uniform_buffer_size, vulkan->device_local_mem_in_use, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, queue_families, 2);
		vulkan->device_local_mem_in_use += uniform_buffer_size;
		if(vulkan_procedure_result != VK_SUCCESS) return 22;
	}

	//CREATE UNIFORM DESCRIPTOR POOL
	vulkan_procedure_result = create_uniform_descriptor_pool(&vulkan->descriptor_pool, &vulkan->logical_device, vulkan->swapchain_image_count);
	if(vulkan_procedure_result != VK_SUCCESS) return 12;


	//CREATE UNIFORM DESCRIPTOR SETS
	vulkan_procedure_result = create_descriptor_sets(vulkan->descriptor_sets, &vulkan->logical_device, &vulkan->descriptor_pool, vulkan->world_matrix_buffers, vulkan->uniform_descriptor_set_layout, vulkan->swapchain_image_count, sizeof(world_matrices));
	if(vulkan_procedure_result != VK_SUCCESS) return 13;
	
	//CREATE GRAPHICS PIPELINE LAYOUT
	vulkan_procedure_result = create_graphics_pipeline_layout(&vulkan->pipeline_layout, vulkan->logical_device, vulkan->uniform_descriptor_set_layout);
	if(vulkan_procedure_result != VK_SUCCESS) return 8;

	//CREATE SWAPCHAIN DEPENDENT COMPONENTS
	uint32_t swapchain_creation_result = create_swapchain_dependent_components(vulkan);
	if(swapchain_creation_result) return 101;

	//CREATE COMMAND POOL AND BUFFERS
	vulkan_procedure_result = create_command_pool(&vulkan->command_pool, &vulkan->logical_device, vulkan->graphics_queue_index);
	if(vulkan_procedure_result != VK_SUCCESS) return 14;

	vulkan_procedure_result = create_command_pool(&vulkan->transfer_command_pool, &vulkan->logical_device, vulkan->transfer_queue_index);
	if(vulkan_procedure_result != VK_SUCCESS) return 19;

	vulkan_procedure_result = create_command_buffers(vulkan->command_buffers, &vulkan->logical_device, &vulkan->command_pool, vulkan->swapchain_image_count);
	if(vulkan_procedure_result != VK_SUCCESS) return 15;

	vulkan_procedure_result = create_command_buffers(&vulkan->transfer_command_buffer, &vulkan->logical_device, &vulkan->transfer_command_pool, 1);
	if(vulkan_procedure_result != VK_SUCCESS) return 20;

	//CREATE SEMAPHORES AND FENCES
	for(int i = 0; i < MAX_FRAMES_COMPUTED_AT_ONCE; i++)
	{
		vulkan_procedure_result = create_semaphore(&vulkan->image_available_semaphores[i], &vulkan->logical_device);
		if(vulkan_procedure_result != VK_SUCCESS)
		{
			printf("Failed to create image available semaphore %d\n", i);
			return 16;
		}
		
		vulkan_procedure_result = create_semaphore(&vulkan->render_finished_semaphores[i], &vulkan->logical_device);
		if(vulkan_procedure_result != VK_SUCCESS)
		{
			printf("Failed to create render finished semaphore %d\n", i);
			return 17;
		}

		vulkan_procedure_result = create_fence(&vulkan->framebuffer_in_use_fences[i], &vulkan->logical_device);
		if(vulkan_procedure_result != VK_SUCCESS)
		{
			printf("Failed to create framebuffer in use fence %d\n", i);
			return 18;
		}
	}
	vulkan->current_frame = 0;

	vulkan->swapchain_image_index = 0;
	return 0;
}

void shutdown_vulkan(vulkan_state* vulkan)
{
	vkDestroyBuffer(vulkan->logical_device, vulkan->staging_buffer, NULL);
	vkFreeMemory(vulkan->logical_device, vulkan->staging_buffer_memory, NULL);
	vkFreeMemory(vulkan->logical_device, vulkan->gpu_memory, NULL);
	for(int i = 0; i < vulkan->swapchain_image_count; i++)
	{
		vkDestroyBuffer(vulkan->logical_device, vulkan->world_matrix_buffers[i], NULL);
	}
	vkDestroyDescriptorPool(vulkan->logical_device, vulkan->descriptor_pool, NULL);
	for(int i = 0; i < MAX_FRAMES_COMPUTED_AT_ONCE; i++)
	{
		vkDestroySemaphore(vulkan->logical_device, vulkan->render_finished_semaphores[i], NULL);
		vkDestroySemaphore(vulkan->logical_device, vulkan->image_available_semaphores[i], NULL);
		vkDestroyFence(vulkan->logical_device, vulkan->framebuffer_in_use_fences[i], NULL);
	}
	vkDestroyCommandPool(vulkan->logical_device, vulkan->transfer_command_pool, NULL);
	vkDestroyCommandPool(vulkan->logical_device, vulkan->command_pool, NULL);
	vkDestroyDescriptorSetLayout(vulkan->logical_device, vulkan->uniform_descriptor_set_layout, NULL);
	vkDestroyPipelineLayout(vulkan->logical_device, vulkan->pipeline_layout, NULL);
	for(int i = 0; i < vulkan->swapchain_image_count; i++) vkDestroyFramebuffer(vulkan->logical_device, vulkan->swapchain_framebuffers[i], NULL); //*
	vkDestroyPipeline(vulkan->logical_device, vulkan->line_graphics_pipeline, NULL); //*
	vkDestroyPipeline(vulkan->logical_device, vulkan->graphics_pipeline, NULL); //*
	for(int i = 0; i < vulkan->swapchain_image_count; i++) vkDestroyImageView(vulkan->logical_device, vulkan->swapchain_image_views[i], NULL); //*
	vkDestroySwapchainKHR(vulkan->logical_device, vulkan->swapchain, NULL); //*
	vkDestroyRenderPass(vulkan->logical_device, vulkan->render_pass, NULL);
	vkDestroyDevice(vulkan->logical_device, NULL);
	destroy_debug_messenger(vulkan->instance, vulkan->debug_messenger);
	vkDestroyInstance(vulkan->instance, NULL);
}

void push_model_matrix(vulkan_state* vulkan, mat4 model)
{
	vkCmdPushConstants(vulkan->command_buffers[vulkan->swapchain_image_index], vulkan->pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mat4), &model);
}

void update_world_matrix(vulkan_state* vulkan, mat4 view, mat4 projection)
{
	world_matrices wm;
	wm.view = view;
	wm.projection = projection;

	for(int i = 0; i < vulkan->swapchain_image_count; i++)
	{
		size_t s = sizeof(world_matrices);
		world_matrices* p_wm = &wm;
		copy_data_to_buffer(vulkan->logical_device, vulkan->staging_buffer_memory, s, p_wm);
		copy_data_between_buffers(&vulkan->staging_buffer, &vulkan->world_matrix_buffers[i], s, &vulkan->transfer_command_buffer, &vulkan->transfer_queue);
	}
}

void complete_graphical_tasks(vulkan_state* vulkan)
{
	vkDeviceWaitIdle(vulkan->logical_device);
}

void begin_frame(vulkan_state* vulkan)
{
	//Make sure previous commands to swapchain image are completed
	vkWaitForFences(vulkan->logical_device, 1, &vulkan->framebuffer_in_use_fences[vulkan->current_frame], VK_TRUE, (uint64_t)(-1));
	vkResetFences(vulkan->logical_device, 1, &vulkan->framebuffer_in_use_fences[vulkan->current_frame]);

	//DRAW FRAME
	//GET IMAGE FROM SWAPCHAIN
	vkAcquireNextImageKHR(vulkan->logical_device, vulkan->swapchain, (uint64_t)(-1), vulkan->image_available_semaphores[vulkan->current_frame], VK_NULL_HANDLE, &vulkan->swapchain_image_index);

	//BEGIN RECORDING TO COMMAND BUFFERS
	VkCommandBufferBeginInfo command_begin_info = {};
	command_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	command_begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	
	if(vkBeginCommandBuffer(vulkan->command_buffers[vulkan->swapchain_image_index], &command_begin_info) != VK_SUCCESS)
	{
		printf("Can't record commands\n");
	}

	VkRenderPassBeginInfo render_pass_begin_info = {};
	render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	render_pass_begin_info.renderPass = vulkan->render_pass;
	render_pass_begin_info.framebuffer = vulkan->swapchain_framebuffers[vulkan->swapchain_image_index];
	render_pass_begin_info.renderArea.offset = {0, 0};
	render_pass_begin_info.renderArea.extent = vulkan->swapchain_extent;

	VkClearValue clear_colour = {0.0f, 1.0f, 1.0f, 1.0f};
	render_pass_begin_info.clearValueCount = 1;
	render_pass_begin_info.pClearValues = &clear_colour;

	vkCmdBeginRenderPass(vulkan->command_buffers[vulkan->swapchain_image_index], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
}

void draw(vulkan_state* vulkan, graphical_data_buffer* data)
{
	vkCmdBindPipeline(vulkan->command_buffers[vulkan->swapchain_image_index], VK_PIPELINE_BIND_POINT_GRAPHICS, vulkan->graphics_pipeline);
	VkBuffer vertex_buffers[] = {data->vertex_buffer};
	VkDeviceSize offsets[] = {0};
	
	vkCmdBindVertexBuffers(vulkan->command_buffers[vulkan->swapchain_image_index], 0, 1, vertex_buffers, offsets);
	vkCmdBindIndexBuffer(vulkan->command_buffers[vulkan->swapchain_image_index], data->index_buffer, 0, VK_INDEX_TYPE_UINT16);
	vkCmdBindDescriptorSets(vulkan->command_buffers[vulkan->swapchain_image_index], VK_PIPELINE_BIND_POINT_GRAPHICS, vulkan->pipeline_layout, 0, 1, &vulkan->descriptor_sets[vulkan->swapchain_image_index], 0, NULL);

	vkCmdDrawIndexed(vulkan->command_buffers[vulkan->swapchain_image_index], data->index_count, 1, 0, 0, 0);
}

void draw_line(vulkan_state* vulkan, graphical_data_buffer* data)
{
	vkCmdBindPipeline(vulkan->command_buffers[vulkan->swapchain_image_index], VK_PIPELINE_BIND_POINT_GRAPHICS, vulkan->line_graphics_pipeline);

	VkBuffer vertex_buffers[] = {data->vertex_buffer};
	VkDeviceSize offsets[] = {0};

	vkCmdBindVertexBuffers(vulkan->command_buffers[vulkan->swapchain_image_index], 0, 1, vertex_buffers, offsets);
	vkCmdBindDescriptorSets(vulkan->command_buffers[vulkan->swapchain_image_index], VK_PIPELINE_BIND_POINT_GRAPHICS, vulkan->pipeline_layout, 0, 1, &vulkan->descriptor_sets[vulkan->swapchain_image_index], 0, NULL);
	vkCmdDraw(vulkan->command_buffers[vulkan->swapchain_image_index], data->vertex_count, 1, 0, 0);
}

void render_frame(vulkan_state* vulkan)
{
	vkCmdEndRenderPass(vulkan->command_buffers[vulkan->swapchain_image_index]);

	if(vkEndCommandBuffer(vulkan->command_buffers[vulkan->swapchain_image_index]) != VK_SUCCESS)
	{
		printf("Command buffer failed\n");
	}

	VkSubmitInfo submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore wait_semaphores[] = {vulkan->image_available_semaphores[vulkan->current_frame]};
	VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	submit_info.waitSemaphoreCount = 1;
	submit_info.pWaitSemaphores = wait_semaphores;
	submit_info.pWaitDstStageMask = wait_stages;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &vulkan->command_buffers[vulkan->swapchain_image_index];
	
	VkSemaphore signal_semaphores[] = {vulkan->render_finished_semaphores[vulkan->current_frame]};
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores = signal_semaphores;

	//SUBMIT COMMAND BUFFER FOR EXECUTION
	if(vkQueueSubmit(vulkan->graphics_queue, 1, &submit_info, vulkan->framebuffer_in_use_fences[vulkan->current_frame]) != VK_SUCCESS)
	{
		printf("Couldn't submit\n");
	}

	//PRESENT SWAPCHAIN IMAGE
	VkPresentInfoKHR present_info = {};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = signal_semaphores;

	VkSwapchainKHR swapchains[] = {vulkan->swapchain};
	present_info.swapchainCount = 1;
	present_info.pSwapchains = swapchains;
	present_info.pImageIndices = &vulkan->swapchain_image_index;
	
	vkQueuePresentKHR(vulkan->graphics_queue, &present_info);
	
	vulkan->current_frame = (vulkan->current_frame + 1) % MAX_FRAMES_COMPUTED_AT_ONCE;
}

