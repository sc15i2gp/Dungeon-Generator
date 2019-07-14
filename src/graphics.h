#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <windows.h>
#include <stdio.h>
#include "maths.h"

#define MAX_FRAMES_COMPUTED_AT_ONCE 2

#define KILOBYTES(n) 1024*n
#define MEGABYTES(n) KILOBYTES(1024*n)

struct world_matrices
{
	mat4 view;
	mat4 projection;
};


struct vertex
{
	vec2d position;
	vec3d colour;
};

struct vulkan_state
{
	//Instance
	VkInstance instance;

	VkDebugUtilsMessengerEXT debug_messenger;

	//Device
	VkPhysicalDevice physical_device;
	VkDevice logical_device;

	//Surface
	VkSurfaceKHR surface;

	//Queues
	VkQueue graphics_queue;
	uint32_t graphics_queue_index;
	VkQueue transfer_queue;
	uint32_t transfer_queue_index;

	//Swapchain
	VkExtent2D swapchain_extent; //Dimensions (in fragments) of swapchain images
	VkSwapchainKHR swapchain;
	
	uint32_t swapchain_image_count;
	VkImageView swapchain_image_views[4];
	VkFramebuffer swapchain_framebuffers[4];

	//Command buffer (coupled to swapchain)
	VkCommandPool command_pool;
	VkCommandBuffer command_buffers[4];
	VkCommandPool transfer_command_pool;
	VkCommandBuffer transfer_command_buffer;

	//Uniform buffer
	VkBuffer world_matrix_buffers[4];

	//Uniform descriptors
	VkDescriptorSetLayout uniform_descriptor_set_layout;
	VkDescriptorPool descriptor_pool;
	VkDescriptorSet descriptor_sets[4];

	//Pipeline
	VkRenderPass render_pass;
	VkPipelineLayout pipeline_layout;
	VkPipeline graphics_pipeline;
	VkPipeline line_graphics_pipeline;

	//Swapchain target
	int current_frame;
	uint32_t swapchain_image_index;

	//Swapchain synchronisation
	VkSemaphore image_available_semaphores[MAX_FRAMES_COMPUTED_AT_ONCE];
	VkSemaphore render_finished_semaphores[MAX_FRAMES_COMPUTED_AT_ONCE];
	VkFence framebuffer_in_use_fences[MAX_FRAMES_COMPUTED_AT_ONCE];

	//Memory
	VkDeviceMemory staging_buffer_memory;
	VkBuffer staging_buffer;
	VkDeviceMemory gpu_memory; //Device local memory
	uint16_t device_local_mem_in_use; //Device local
};

//Allocated memory on gpu which contains data to render
struct graphical_data_buffer
{
	VkBuffer vertex_buffer;
	VkBuffer index_buffer;
	int vertex_count;
	int index_count;
};

uint8_t startup_vulkan(vulkan_state*, HWND, HINSTANCE);
void shutdown_vulkan(vulkan_state*);

graphical_data_buffer buffer_graphical_data(vulkan_state*, vertex*, int, uint16_t*, int);
graphical_data_buffer buffer_graphical_data(vulkan_state*, vertex*, int);

void begin_frame(vulkan_state*);
void draw(vulkan_state*, graphical_data_buffer*);
void draw_line(vulkan_state*, graphical_data_buffer*);
void render_frame(vulkan_state*);
void complete_graphical_tasks(vulkan_state*);
void destroy_graphical_data(vulkan_state*, graphical_data_buffer*);
void update_world_matrix(vulkan_state*, mat4, mat4);
void push_model_matrix(vulkan_state*, mat4);
void resize_window(vulkan_state*);
