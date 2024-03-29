/* Jordan Justen : gears3d is public domain */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "sim.h"
#include "winsys_x11r6.h"
#include "winsys_wl.h"
#include "g_math.h"
#include "vert_buf.h"

#include <pthread.h>
#define VK_USE_PLATFORM_WAYLAND_KHR
#define VK_USE_PLATFORM_XLIB_KHR
#include <vulkan/vulkan.h>

#include "g_math.h"
#include "load_lib.h"
#include "vk-spirv.h"

#pragma pack(1)

struct vs_uniform_data {
    mat4 model;
    mat4 view;
    mat4 projection;
    float gear_angle;
    float tooth_angle;
};

#pragma pack()

#define VFN(fn) _##fn
#define DECL_PVKFN(fn) PFN_##fn _##fn = NULL

/* dlsym function pointers */
DECL_PVKFN(vkCreateInstance);
DECL_PVKFN(vkGetDeviceProcAddr);
DECL_PVKFN(vkGetInstanceProcAddr);

/* vkGetInstanceProcAddr function pointers */
DECL_PVKFN(vkCreateDevice);
DECL_PVKFN(vkCreateWaylandSurfaceKHR);
DECL_PVKFN(vkCreateXlibSurfaceKHR);
DECL_PVKFN(vkEnumeratePhysicalDevices);
DECL_PVKFN(vkGetPhysicalDeviceMemoryProperties);
DECL_PVKFN(vkGetPhysicalDeviceProperties);
DECL_PVKFN(vkGetPhysicalDeviceQueueFamilyProperties);
DECL_PVKFN(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
DECL_PVKFN(vkGetPhysicalDeviceSurfaceFormatsKHR);
DECL_PVKFN(vkGetPhysicalDeviceSurfacePresentModesKHR);
DECL_PVKFN(vkGetPhysicalDeviceSurfaceSupportKHR);

/* vkGetDeviceProcAddr function pointers */
DECL_PVKFN(vkAcquireNextImageKHR);
DECL_PVKFN(vkAllocateCommandBuffers);
DECL_PVKFN(vkAllocateDescriptorSets);
DECL_PVKFN(vkAllocateMemory);
DECL_PVKFN(vkBeginCommandBuffer);
DECL_PVKFN(vkBindBufferMemory);
DECL_PVKFN(vkBindImageMemory);
DECL_PVKFN(vkCmdBeginRenderPass);
DECL_PVKFN(vkCmdBindDescriptorSets);
DECL_PVKFN(vkCmdBindPipeline);
DECL_PVKFN(vkCmdBindVertexBuffers);
DECL_PVKFN(vkCmdCopyImageToBuffer);
DECL_PVKFN(vkCmdDraw);
DECL_PVKFN(vkCmdEndRenderPass);
DECL_PVKFN(vkCmdPushConstants);
DECL_PVKFN(vkCmdSetScissor);
DECL_PVKFN(vkCmdSetViewport);
DECL_PVKFN(vkCreateBuffer);
DECL_PVKFN(vkCreateCommandPool);
DECL_PVKFN(vkCreateDescriptorPool);
DECL_PVKFN(vkCreateDescriptorSetLayout);
DECL_PVKFN(vkCreateFence);
DECL_PVKFN(vkCreateFramebuffer);
DECL_PVKFN(vkCreateGraphicsPipelines);
DECL_PVKFN(vkCreateImage);
DECL_PVKFN(vkCreateImageView);
DECL_PVKFN(vkCreatePipelineLayout);
DECL_PVKFN(vkCreateRenderPass);
DECL_PVKFN(vkCreateSemaphore);
DECL_PVKFN(vkCreateShaderModule);
DECL_PVKFN(vkCreateSwapchainKHR);
DECL_PVKFN(vkDestroyBuffer);
DECL_PVKFN(vkDestroyImage);
DECL_PVKFN(vkDestroySwapchainKHR);
DECL_PVKFN(vkEndCommandBuffer);
DECL_PVKFN(vkFlushMappedMemoryRanges);
DECL_PVKFN(vkFreeMemory);
DECL_PVKFN(vkGetBufferMemoryRequirements);
DECL_PVKFN(vkGetDeviceQueue);
DECL_PVKFN(vkGetFenceStatus);
DECL_PVKFN(vkGetImageMemoryRequirements);
DECL_PVKFN(vkGetSwapchainImagesKHR);
DECL_PVKFN(vkMapMemory);
DECL_PVKFN(vkQueuePresentKHR);
DECL_PVKFN(vkQueueSubmit);
DECL_PVKFN(vkResetFences);
DECL_PVKFN(vkUnmapMemory);
DECL_PVKFN(vkUpdateDescriptorSets);
DECL_PVKFN(vkWaitForFences);

static const char *
res_to_str(VkResult res)
{
    switch (res) {
    #define RES(r) case r: return #r
    RES(VK_SUCCESS);
    RES(VK_NOT_READY);
    RES(VK_TIMEOUT);
    RES(VK_EVENT_SET);
    RES(VK_EVENT_RESET);
    RES(VK_INCOMPLETE);
    RES(VK_ERROR_OUT_OF_HOST_MEMORY);
    RES(VK_ERROR_OUT_OF_DEVICE_MEMORY);
    RES(VK_ERROR_INITIALIZATION_FAILED);
    RES(VK_ERROR_DEVICE_LOST);
    RES(VK_ERROR_MEMORY_MAP_FAILED);
    RES(VK_ERROR_LAYER_NOT_PRESENT);
    RES(VK_ERROR_EXTENSION_NOT_PRESENT);
    RES(VK_ERROR_FEATURE_NOT_PRESENT);
    RES(VK_ERROR_INCOMPATIBLE_DRIVER);
    RES(VK_ERROR_TOO_MANY_OBJECTS);
    RES(VK_ERROR_FORMAT_NOT_SUPPORTED);
    RES(VK_ERROR_FRAGMENTED_POOL);
    RES(VK_ERROR_OUT_OF_POOL_MEMORY);
    RES(VK_ERROR_INVALID_EXTERNAL_HANDLE);
    RES(VK_ERROR_SURFACE_LOST_KHR);
    RES(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR);
    RES(VK_SUBOPTIMAL_KHR);
    RES(VK_ERROR_OUT_OF_DATE_KHR);
    RES(VK_ERROR_INCOMPATIBLE_DISPLAY_KHR);
    RES(VK_ERROR_VALIDATION_FAILED_EXT);
    RES(VK_ERROR_INVALID_SHADER_NV);
    RES(VK_ERROR_FRAGMENTATION_EXT);
    RES(VK_ERROR_NOT_PERMITTED_EXT);
    /* RES(VK_ERROR_OUT_OF_POOL_MEMORY_KHR); */
    /* RES(VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR); */
    /* RES(VK_RESULT_BEGIN_RANGE); */
    /* RES(VK_RESULT_END_RANGE); */
    /* RES(VK_RESULT_RANGE_SIZE); */
    #undef RES
    default: return NULL;
    }
}

static void* vk_alloc_func(void* pUserData, size_t size, size_t alignment,
                           VkSystemAllocationScope allocationScope)
{
    return malloc(size);
}

static void* vk_realloc_func(void* pUserData, void* pOriginal, size_t size,
                             size_t alignment,
                             VkSystemAllocationScope allocationScope)
{
    return realloc(pOriginal, size);
}

static void vk_free_func(void* pUserData, void* pMemory)
{
    free(pMemory);
}

static void vk_int_alloc_notif(void* pUserData, size_t size,
                               VkInternalAllocationType allocationType,
                               VkSystemAllocationScope allocationScope)
{
}

static void vk_int_free_notif(void* pUserData, size_t size,
                              VkInternalAllocationType allocationType,
                              VkSystemAllocationScope allocationScope)
{
}

static const VkAllocationCallbacks alloc_callbacks = {
    .pUserData = NULL,
    .pfnAllocation = vk_alloc_func,
    .pfnReallocation = vk_realloc_func,
    .pfnFree = vk_free_func,
    .pfnInternalAllocation = vk_int_alloc_notif,
    .pfnInternalFree = vk_int_free_notif,
};

static const float queue_priority = 1.0f;
static const VkDeviceQueueCreateInfo dev_queue_create_info = {
    .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
    .queueFamilyIndex = 0,
    .queueCount = 1,
    .pQueuePriorities = &queue_priority,
};

static const VkCommandPoolCreateInfo cmd_pool_create_info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
};

#define GEARS 3
#define NUM_IMAGES 10
static uint32_t num_images = 0;
static uint32_t max_in_flight_images = NUM_IMAGES;

static VkInstance instance;

static VkPhysicalDevice phy_device;
static uint32_t family_queue_index;
static VkDevice device;
static VkPhysicalDeviceProperties dev_prop;
static VkQueue queue;
static VkCommandPool cmd_pool;
static VkSurfaceKHR surface;
static bool immediate_present_supported = false;
static bool mailbox_present_supported = false;
static VkSemaphore semaphores[NUM_IMAGES];
static VkFence fences[NUM_IMAGES];
static bool using_wsi = false;
static bool wayland_wsi_supported = false;
static bool xlib_wsi_supported = false;
static enum winsys_type active_winsys;

static VkCommandBuffer cmd_buffers[NUM_IMAGES];
static VkDeviceMemory vert_mem;
static VkBuffer vert_buf;
static VkDeviceMemory uniform_mem;
static VkBuffer uniform_buf;
struct vs_uniform_data *uniform_map;
static VkDescriptorSet descriptor_sets[NUM_IMAGES * GEARS];
static VkRenderPass render_pass;
static VkPipelineLayout pipeline_layout;
static VkPipeline pipeline;

static unsigned int gear_uniform_data_size;
static unsigned int img_uniform_data_size;
static unsigned int uniform_data_size;

static bool update_angle_uniform = true;

static int32_t in_flight_indices[NUM_IMAGES];
static unsigned int submitted_render_count = 0;
static int next_render_pos = 0;

static VkResult
create_instance()
{
    VkResult res;

    static const char *extensions[3] = {
        "VK_KHR_surface",
        "VK_KHR_wayland_surface",
        "VK_KHR_xlib_surface",
    };
    static const char *x_extensions[2] = {
        "VK_KHR_surface",
        "VK_KHR_xlib_surface",
    };
    VkInstanceCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &(VkApplicationInfo) {
            .pApplicationName = "gears3d",
            .apiVersion = VK_MAKE_VERSION(1, 0, 0),
        },
        .ppEnabledExtensionNames = extensions,
        .enabledExtensionCount = 3,
    };

    switch (gears_options.winsys_type) {
    case WINSYS_NONE:
        create_info.enabledExtensionCount = 1;
        return VFN(vkCreateInstance)(&create_info, &alloc_callbacks, &instance);
    case WINSYS_WAYLAND:
        create_info.enabledExtensionCount = 2;
        res = VFN(vkCreateInstance)(&create_info, &alloc_callbacks, &instance);
        if (res == VK_SUCCESS)
            wayland_wsi_supported = true;
        return res;
    case WINSYS_X11:
        create_info.ppEnabledExtensionNames = x_extensions;
        create_info.enabledExtensionCount = 2;
        res = VFN(vkCreateInstance)(&create_info, &alloc_callbacks, &instance);
        if (res == VK_SUCCESS)
            xlib_wsi_supported = true;
        return res;
    case WINSYS_AUTO:
        res = VFN(vkCreateInstance)(&create_info, &alloc_callbacks, &instance);
        if (res == VK_SUCCESS) {
            wayland_wsi_supported = true;
            xlib_wsi_supported = true;
            return res;
        }
        create_info.enabledExtensionCount = 2;
        res = VFN(vkCreateInstance)(&create_info, &alloc_callbacks, &instance);
        if (res == VK_SUCCESS) {
            wayland_wsi_supported = true;
            return res;
        }
        create_info.ppEnabledExtensionNames = x_extensions;
        res = VFN(vkCreateInstance)(&create_info, &alloc_callbacks, &instance);
        if (res == VK_SUCCESS) {
            xlib_wsi_supported = true;
            return res;
        }
        create_info.enabledExtensionCount = 1;
        return VFN(vkCreateInstance)(&create_info, &alloc_callbacks, &instance);
    default:
        return VK_ERROR_FORMAT_NOT_SUPPORTED;
    }

    return res;
}

static void init_vk_instance()
{
    VkResult res;
    void *lib = open_library("libvulkan.so.1");
    if (lib == NULL)
        abort();

    #define DLSYM(f)                          \
    do {                                      \
        VFN(f) = get_library_symbol(lib, #f); \
        assert(VFN(f) != NULL);               \
    } while(0)

    DLSYM(vkCreateInstance);
    DLSYM(vkGetInstanceProcAddr);
    DLSYM(vkGetDeviceProcAddr);
    #undef DLSYM

    res = create_instance();
    if (res != VK_SUCCESS)
        printf("Failed to create vulkan instance: %s\n", res_to_str(res));
    assert(res == VK_SUCCESS);

    #define GET_I_PROC(f)                                               \
    do {                                                                \
        VFN(f) = (void*)VFN(vkGetInstanceProcAddr)(instance, #f);       \
        assert(VFN(f) != NULL);                                         \
    } while(0)

    GET_I_PROC(vkCreateDevice);
    if (wayland_wsi_supported)
        GET_I_PROC(vkCreateWaylandSurfaceKHR);
    if (xlib_wsi_supported)
        GET_I_PROC(vkCreateXlibSurfaceKHR);
    GET_I_PROC(vkEnumeratePhysicalDevices);
    GET_I_PROC(vkGetPhysicalDeviceMemoryProperties);
    GET_I_PROC(vkGetPhysicalDeviceProperties);
    GET_I_PROC(vkGetPhysicalDeviceQueueFamilyProperties);
    GET_I_PROC(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
    GET_I_PROC(vkGetPhysicalDeviceSurfaceFormatsKHR);
    GET_I_PROC(vkGetPhysicalDeviceSurfacePresentModesKHR);
    GET_I_PROC(vkGetPhysicalDeviceSurfaceSupportKHR);
    #undef GET_I_PROC
}

static void
init_vk_device()
{
    VkResult res;

    uint32_t dev_count;
    res = VFN(vkEnumeratePhysicalDevices)(instance, &dev_count, NULL);
    assert(res == VK_SUCCESS && dev_count >= 1);

    VkPhysicalDevice *devices = malloc(sizeof(*devices) * dev_count);
    assert(devices != NULL);

    res = VFN(vkEnumeratePhysicalDevices)(instance, &dev_count, devices);
    assert(res == VK_SUCCESS && dev_count >= 1);

    uint32_t dev_num, queue_num;

    for (dev_num = 0; dev_num < dev_count; dev_num++) {
        uint32_t queue_count;
        VFN(vkGetPhysicalDeviceQueueFamilyProperties)(devices[dev_num],
                                                      &queue_count, NULL);
        assert(res == VK_SUCCESS && queue_count >= 1);

        VkQueueFamilyProperties *props = malloc(sizeof(*props) * queue_count);
        assert(props != NULL);
        VFN(vkGetPhysicalDeviceQueueFamilyProperties)(devices[dev_num],
                                                      &queue_count, props);

        for (queue_num = 0; queue_num < queue_count; queue_num++) {
            assert(props[queue_num].queueCount >= 1);
            if ((props[queue_num].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
                break;
        }

        free(props);

        if (queue_num < queue_count) {
            family_queue_index = queue_num;
            break;
        }
    }

    if (dev_num >= dev_count) {
        device = VK_NULL_HANDLE;
        free(devices);
        return;
    }
    phy_device = devices[dev_num];
    free(devices);
    devices = NULL;

    const char *extensions[] = {
        "VK_KHR_swapchain",
    };
    const VkDeviceCreateInfo dev_create_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &dev_queue_create_info,
        .enabledExtensionCount = 1,
        .ppEnabledExtensionNames = extensions,
    };

    res = VFN(vkCreateDevice)(phy_device, &dev_create_info, NULL, &device);
    assert(res == VK_SUCCESS);

#define GET_D_PROC(f) \
    VFN(f) = (void*)VFN(vkGetDeviceProcAddr)(device, #f);   \
    assert(VFN(f) != NULL)

    GET_D_PROC(vkAcquireNextImageKHR);
    GET_D_PROC(vkAllocateCommandBuffers);
    GET_D_PROC(vkAllocateDescriptorSets);
    GET_D_PROC(vkAllocateMemory);
    GET_D_PROC(vkBeginCommandBuffer);
    GET_D_PROC(vkBindBufferMemory);
    GET_D_PROC(vkBindImageMemory);
    GET_D_PROC(vkCmdBeginRenderPass);
    GET_D_PROC(vkCmdBindDescriptorSets);
    GET_D_PROC(vkCmdBindPipeline);
    GET_D_PROC(vkCmdBindVertexBuffers);
    GET_D_PROC(vkCmdCopyImageToBuffer);
    GET_D_PROC(vkCmdDraw);
    GET_D_PROC(vkCmdEndRenderPass);
    GET_D_PROC(vkCmdPushConstants);
    GET_D_PROC(vkCmdSetScissor);
    GET_D_PROC(vkCmdSetViewport);
    GET_D_PROC(vkCreateBuffer);
    GET_D_PROC(vkCreateCommandPool);
    GET_D_PROC(vkCreateDescriptorPool);
    GET_D_PROC(vkCreateDescriptorSetLayout);
    GET_D_PROC(vkCreateFence);
    GET_D_PROC(vkCreateFramebuffer);
    GET_D_PROC(vkCreateGraphicsPipelines);
    GET_D_PROC(vkCreateImage);
    GET_D_PROC(vkCreateImageView);
    GET_D_PROC(vkCreatePipelineLayout);
    GET_D_PROC(vkCreateRenderPass);
    GET_D_PROC(vkCreateSemaphore);
    GET_D_PROC(vkCreateShaderModule);
    GET_D_PROC(vkCreateSwapchainKHR);
    GET_D_PROC(vkDestroyBuffer);
    GET_D_PROC(vkDestroyImage);
    GET_D_PROC(vkDestroySwapchainKHR);
    GET_D_PROC(vkEndCommandBuffer);
    GET_D_PROC(vkFlushMappedMemoryRanges);
    GET_D_PROC(vkFreeMemory);
    GET_D_PROC(vkGetBufferMemoryRequirements);
    GET_D_PROC(vkGetDeviceQueue);
    GET_D_PROC(vkGetFenceStatus);
    GET_D_PROC(vkGetImageMemoryRequirements);
    GET_D_PROC(vkGetSwapchainImagesKHR);
    GET_D_PROC(vkMapMemory);
    GET_D_PROC(vkQueuePresentKHR);
    GET_D_PROC(vkQueueSubmit);
    GET_D_PROC(vkResetFences);
    GET_D_PROC(vkUnmapMemory);
    GET_D_PROC(vkUpdateDescriptorSets);
    GET_D_PROC(vkWaitForFences);
}

static void
set_window_attributes()
{
    VkResult res;

    init_vk_instance();
    init_vk_device();

    VFN(vkGetPhysicalDeviceProperties)(phy_device, &dev_prop);

    VFN(vkGetDeviceQueue)(device, family_queue_index, 0, &queue);

    res = VFN(vkCreateCommandPool)(device, &cmd_pool_create_info, NULL,
                                   &cmd_pool);
    assert(res == VK_SUCCESS);

    VkSemaphoreCreateInfo semaphore_create_info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };

    VkFenceCreateInfo fence_create_info = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO
    };
    for (int i = 0; i < NUM_IMAGES; i++) {
        res = VFN(vkCreateSemaphore)(device, &semaphore_create_info, NULL,
                                     &semaphores[i]);
        assert(res == VK_SUCCESS);
        res = VFN(vkCreateFence)(device, &fence_create_info, NULL, &fences[i]);
        assert(res == VK_SUCCESS);
    }
}

struct gear_info {
    int teeth;
    float inner_radius, outer_radius, width, tooth_depth;
    float angle_rate;
    float angle_adjust;
    struct vec3 translate;
    float color[4];
    mat4 model;
    float angle;
    uint32_t num_vertices, vertex_buf_offset;
};

static struct gear_info gears[GEARS] = {
    {
        .teeth = 20, .inner_radius = 1.0, .outer_radius = 4.0, .width = 1.0,
        .tooth_depth = 0.7,
        .angle_rate = 1.0, .angle_adjust = 0.0,
        .translate = { -3.0, -2.0 },
        .color = { 0.8, 0.1, 0.0, 1.0 },
    },
    {
        .teeth = 10, .inner_radius = 0.5, .outer_radius = 2.0, .width = 2.0,
        .tooth_depth = 0.7,
        .angle_rate = -2.0, .angle_adjust = (M_PI * -9.0 / 180.0),
        .translate = { 3.1, -2.0 },
        .color = { 0.0, 0.8, 0.2, 1.0 },
    },
    {
        .teeth = 10, .inner_radius = 1.3, .outer_radius = 2.0, .width = 0.5,
        .tooth_depth = 0.7,
        .angle_rate = -2.0, .angle_adjust = (M_PI * -25.0 / 180.0),
        .translate = { -3.1, 4.2 },
        .color = { 0.2, 0.2, 1.0, 1.0 },
    },
};

static void
rotate_gears(float x, float y, float z)
{
    float m4[16];
    float tmp[16];
    int i;

    for (i = 0; i < GEARS; i++) {
        rotate(m4, x, 1.0, 0.0, 0.0);
        if (y != 0.0) {
            rotate(tmp, y, 0.0, 1.0, 0.0);
            mult_m4m4(m4, m4, tmp);
        }
        if (z != 0.0) {
            rotate(tmp, z, 0.0, 0.0, 1.0);
            mult_m4m4(m4, m4, tmp);
        }
        translate(tmp, gears[i].translate.x, gears[i].translate.y, 0.0);
        mult_m4m4(m4, m4, tmp);
        memcpy(&gears[i].model, m4, sizeof(m4));
    }
}

static struct vs_uniform_data*
gear_uniform_data(void *uniform_map, unsigned int img, unsigned int gear)
{
    return uniform_map + (GEARS * img + gear) * gear_uniform_data_size;
}

static void
set_gear_const_uniform_data(void *uniform_map)
{
    struct vs_uniform_data *gear_u_data;
    unsigned int gear;

    rotate_gears(20.0 / 180.0 * M_PI, 30.0 / 180.0 * M_PI, 0.0);

    for (unsigned int img = 0; img < NUM_IMAGES; img++) {
        for (gear = 0; gear < GEARS; gear++) {
            gear_u_data = gear_uniform_data(uniform_map, img, gear);
            memcpy(gear_u_data->model, gears[gear].model,
                   sizeof(gears[gear].model));
        }
    }
}

static void
set_view_projection(int width, int height, void *uniform_map)
{
    struct vs_uniform_data *gear_u_data;
    unsigned int gear;
    float h = (float) height / (float) width;
    float view[16], projection[16];

    translate(view, 0.0, 0.0, -40.0);

    frustum(projection, -1.0, 1.0, h, -h, 5.0, 200.0);

    for (unsigned int img = 0; img < NUM_IMAGES; img++) {
        for (gear = 0; gear < GEARS; gear++) {
            gear_u_data = gear_uniform_data(uniform_map, img, gear);
            memcpy(gear_u_data->view, view, sizeof(view));
            memcpy(gear_u_data->projection, projection, sizeof(projection));
            gear_u_data->tooth_angle = M_PI / (gears[gear].teeth / 2.0);
        }
    }
}

static void
create_pipeline(int width, int height);

static void
reshape(int width, int height)
{
    set_view_projection(width, height, uniform_map);

    VkMappedMemoryRange mem_range = {
        .sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
        .pNext = NULL,
        .memory = uniform_mem,
        .offset = 0,
        .size = uniform_data_size,
    };
    VkResult res = VFN(vkFlushMappedMemoryRanges)(device, 1, &mem_range);
    assert(res == VK_SUCCESS);

    create_pipeline(width, height);
}

static void
win_resize(int width, int height)
{
    reshape(width, height);
}

static void
init_with_surface()
{
    VkResult res;
    VkBool32 supported;
    res = VFN(vkGetPhysicalDeviceSurfaceSupportKHR)(phy_device, 0, surface,
                                                    &supported);
    assert(res == VK_SUCCESS && supported == VK_TRUE);

    uint32_t present_mode_count, present_mode_num;
    res = VFN(vkGetPhysicalDeviceSurfacePresentModesKHR)(phy_device, surface,
                                                         &present_mode_count,
                                                         NULL);
    assert(res == VK_SUCCESS);

    VkPresentModeKHR *present_modes = malloc(sizeof(VkPresentModeKHR) *
                                             present_mode_count);
    assert(present_modes != NULL);
    res = VFN(vkGetPhysicalDeviceSurfacePresentModesKHR)(phy_device, surface,
                                                         &present_mode_count,
                                                         present_modes);
    assert(res == VK_SUCCESS);

    for (present_mode_num = 0;
         present_mode_num < present_mode_count;
         present_mode_num++) {
        switch(present_modes[present_mode_num]) {
        case VK_PRESENT_MODE_IMMEDIATE_KHR:
            immediate_present_supported = true;
            break;
        case VK_PRESENT_MODE_MAILBOX_KHR:
            mailbox_present_supported = true;
            break;
        default:
            break;
        }
    }
    free(present_modes);
    present_modes = NULL;

    uint32_t format_count, format_num;
    res = VFN(vkGetPhysicalDeviceSurfaceFormatsKHR)(phy_device, surface,
                                                    &format_count, NULL);
    assert(res == VK_SUCCESS && format_count >= 1);

    VkSurfaceFormatKHR *formats = malloc(sizeof(*formats) * format_count);
    assert(formats != NULL);
    res = VFN(vkGetPhysicalDeviceSurfaceFormatsKHR)(phy_device, surface,
                                                    &format_count, formats);
    assert(res == VK_SUCCESS);
    for(format_num = 0; format_num < format_count; format_num++) {
        if (formats[format_num].format == VK_FORMAT_B8G8R8A8_UNORM)
            break;
    }
    assert(format_num < format_count);

    free(formats);
    formats = NULL;
}

static uint32_t
find_memory_idx(uint32_t allowed_indices, VkMemoryPropertyFlags props)
{
    VkPhysicalDeviceMemoryProperties dev_props;
    VFN(vkGetPhysicalDeviceMemoryProperties)(phy_device, &dev_props);

    uint32_t idx;
    for (idx = 0; idx < dev_props.memoryTypeCount; idx++) {
        if ((allowed_indices & (1 << idx)) &&
            (dev_props.memoryTypes[idx].propertyFlags & props) == props) {
            return idx;
        }
    }
    assert(idx < dev_props.memoryTypeCount);
    return dev_props.memoryTypeCount;
}

static void
set_global_state()
{
    Display *dpy;
    Window wnd;
    struct wl_display *wl_dpy;
    struct wl_surface *wl_srf;

    switch (gears_options.winsys_type) {
    case WINSYS_NONE:
            active_winsys = WINSYS_NONE;
            break;
    case WINSYS_WAYLAND:
        assert(wayland_wsi_supported);
        if (get_wl_dpy_srf(&wl_dpy, &wl_srf))
            active_winsys = gears_options.winsys_type;
        break;
    case WINSYS_X11:
        assert(xlib_wsi_supported);
        if (get_x11r6_dpy_wnd(&dpy, &wnd))
            active_winsys = gears_options.winsys_type;
        break;
    case WINSYS_AUTO:
    default:
        if (xlib_wsi_supported && get_x11r6_dpy_wnd(&dpy, &wnd))
            active_winsys = WINSYS_X11;
        else if (wayland_wsi_supported && get_wl_dpy_srf(&wl_dpy, &wl_srf))
            active_winsys = WINSYS_WAYLAND;
        else
            active_winsys = WINSYS_NONE;
    }

    assert(gears_options.winsys_type == WINSYS_AUTO ||
           gears_options.winsys_type == active_winsys);

    using_wsi = active_winsys == WINSYS_WAYLAND || active_winsys == WINSYS_X11;

    VkResult res;
    int i;

    VkImageLayout initialColorLayout =
        using_wsi ? VK_IMAGE_LAYOUT_UNDEFINED :
                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    VkImageLayout finalColorLayout =
        using_wsi ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR :
                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    VkAttachmentDescription att_desc[] = {
        {
            .format = VK_FORMAT_B8G8R8A8_UNORM,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .initialLayout = initialColorLayout,
            .finalLayout = finalColorLayout,
        },
        {
            .format = VK_FORMAT_D16_UNORM,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .stencilLoadOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        },
    };
    VkAttachmentReference att_ref = {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };
    VkAttachmentReference unused_att_ref = {
        .attachment = VK_ATTACHMENT_UNUSED,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };
    VkAttachmentReference depth_att_ref = {
        .attachment = 1,
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };
    VkSubpassDescription subpass_desc = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .inputAttachmentCount = 0,
        .colorAttachmentCount = 1,
        .pColorAttachments = &att_ref,
        .pResolveAttachments = &unused_att_ref,
        .pDepthStencilAttachment = &depth_att_ref,
        .preserveAttachmentCount = 0,
        .pPreserveAttachments = NULL,
    };
    VkRenderPassCreateInfo render_pass_create_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 2,
        .pAttachments = att_desc,
        .subpassCount = 1,
        .pSubpasses = &subpass_desc,
        .dependencyCount = 0,
    };
    res = VFN(vkCreateRenderPass)(device, &render_pass_create_info, NULL,
                                  &render_pass);
    assert(res == VK_SUCCESS);

    switch(active_winsys) {
    case WINSYS_WAYLAND: {
        VkWaylandSurfaceCreateInfoKHR wl_surf_create_info = {
            .sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
            .display = wl_dpy,
            .surface = wl_srf,
            .flags = 0,
            .pNext = NULL,
        };

        res = VFN(vkCreateWaylandSurfaceKHR)(instance, &wl_surf_create_info,
                                             NULL, &surface);
        assert(res == VK_SUCCESS);
        init_with_surface();
        break;
    }
    case WINSYS_X11: {
        VkXlibSurfaceCreateInfoKHR xlib_surf_create_info = {
            .sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
            .dpy = dpy,
            .window = wnd,
            .flags = 0,
            .pNext = NULL,
        };

        res = VFN(vkCreateXlibSurfaceKHR)(instance, &xlib_surf_create_info,
                                          NULL, &surface);
        assert(res == VK_SUCCESS);
        init_with_surface();
        break;
    }
    case WINSYS_NONE:
        break;
    default:
        assert(false);
        break;
    }

    uint32_t total_vertex_count = 0;
    for (i = 0; i < GEARS; i++) {
        gears[i].vertex_buf_offset = total_vertex_count;
        gears[i].num_vertices = tooth_vertex_count();
        total_vertex_count += gears[i].num_vertices;
    }

    unsigned int vertex_data_size =
        total_vertex_count * sizeof(struct gear_vert);

    VkBufferCreateInfo buf_alloc_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = vertex_data_size,
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
    };
    res = VFN(vkCreateBuffer)(device, &buf_alloc_info, NULL, &vert_buf);
    assert(res == VK_SUCCESS);

    VkMemoryRequirements mem_req;
    VFN(vkGetBufferMemoryRequirements)(device, vert_buf, &mem_req);

    uint32_t mem_ty_idx =
        find_memory_idx(mem_req.memoryTypeBits,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    VkMemoryAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = vertex_data_size,
        .memoryTypeIndex = mem_ty_idx,
    };
    res = VFN(vkAllocateMemory)(device, &alloc_info, NULL, &vert_mem);
    assert(res == VK_SUCCESS);

    struct gear_vert *gear_map;
    res = VFN(vkMapMemory)(device, vert_mem, 0, vertex_data_size, 0,
                           (void**)&gear_map);
    assert(res == VK_SUCCESS);

    for (i = 0; i < GEARS; i++) {
        fill_tooth_vertices(gear_map + gears[i].vertex_buf_offset,
                            gears[i].inner_radius, gears[i].outer_radius,
                            gears[i].width, gears[i].teeth,
                            gears[i].tooth_depth);
    }

    VFN(vkUnmapMemory)(device, vert_mem);
    gear_map = NULL;

    res = VFN(vkBindBufferMemory)(device, vert_buf, vert_mem, 0);
    assert(res == VK_SUCCESS);

    gear_uniform_data_size =
        ALIGN(sizeof(struct vs_uniform_data),
              dev_prop.limits.minUniformBufferOffsetAlignment);
    img_uniform_data_size = GEARS * gear_uniform_data_size;
    uniform_data_size = NUM_IMAGES * img_uniform_data_size;

    VkBufferCreateInfo uniform_buf_alloc_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = uniform_data_size,
        .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
    };
    res = VFN(vkCreateBuffer)(device, &uniform_buf_alloc_info, NULL,
                              &uniform_buf);
    assert(res == VK_SUCCESS);

    VFN(vkGetBufferMemoryRequirements)(device, uniform_buf, &mem_req);

    mem_ty_idx =
        find_memory_idx(mem_req.memoryTypeBits,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    alloc_info.memoryTypeIndex = mem_ty_idx;

    alloc_info.allocationSize = uniform_data_size;
    res = VFN(vkAllocateMemory)(device, &alloc_info, NULL, &uniform_mem);
    assert(res == VK_SUCCESS);

    res = VFN(vkMapMemory)(device, uniform_mem, 0, uniform_data_size, 0,
                           (void**)&uniform_map);
    assert(res == VK_SUCCESS);

    memset(uniform_map, 0, uniform_data_size);
    set_gear_const_uniform_data(uniform_map);

    VkMappedMemoryRange mem_range = {
        .sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
        .pNext = NULL,
        .memory = uniform_mem,
        .offset = 0,
        .size = uniform_data_size,
    };
    res = VFN(vkFlushMappedMemoryRanges)(device, 1, &mem_range);
    assert(res == VK_SUCCESS);

    res = VFN(vkBindBufferMemory)(device, uniform_buf, uniform_mem, 0);
    assert(res == VK_SUCCESS);

    VkDescriptorSetLayout set_layout;
    VkDescriptorSetLayoutBinding set_layout_binding = {
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .pImmutableSamplers = NULL
    };
    VkDescriptorSetLayoutCreateInfo set_layout_create_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 1,
        .pBindings = &set_layout_binding,
    };
    res = VFN(vkCreateDescriptorSetLayout)(device, &set_layout_create_info,
                                           NULL, &set_layout);
    assert(res == VK_SUCCESS);

    VkPipelineLayoutCreateInfo pipeline_layout_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts = &set_layout,
        .pushConstantRangeCount = 1,
        .pPushConstantRanges =
            &(VkPushConstantRange) {
                .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                .offset = 0,
                .size = sizeof(gears[0].color),
            },
    };
    res = VFN(vkCreatePipelineLayout)(device, &pipeline_layout_info, NULL,
                                      &pipeline_layout);
    assert(res == VK_SUCCESS);

    VkDescriptorPoolSize pool_size = {
        .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = NUM_IMAGES * GEARS,
    };
    VkDescriptorPoolCreateInfo desc_pool_create_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .maxSets = NUM_IMAGES * GEARS,
        .poolSizeCount = 1,
        .pPoolSizes = &pool_size,
    };
    VkDescriptorPool desc_pool;
    res = VFN(vkCreateDescriptorPool)(device, &desc_pool_create_info, NULL,
                                      &desc_pool);
    assert(res == VK_SUCCESS);

    int desc_num;
    VkDescriptorSetLayout set_layouts[NUM_IMAGES * GEARS];
    for (desc_num = 0; desc_num < NUM_IMAGES * GEARS; desc_num++) {
        set_layouts[desc_num] = set_layout;
    }
    VkDescriptorSetAllocateInfo desc_set_alloc_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = desc_pool,
        .descriptorSetCount = NUM_IMAGES * GEARS,
        .pSetLayouts = set_layouts,
    };
    res = VFN(vkAllocateDescriptorSets)(device, &desc_set_alloc_info,
                                        &descriptor_sets[0]);
    assert(res == VK_SUCCESS);

    VkDescriptorBufferInfo desc_buf_info_tmpl = {
        .buffer = uniform_buf,
        .offset = 0,
        .range = sizeof(struct vs_uniform_data),
    };
    VkDescriptorBufferInfo desc_buf_infos[NUM_IMAGES * GEARS];
    for (desc_num = 0; desc_num < NUM_IMAGES * GEARS; desc_num++) {
        memcpy(&desc_buf_infos[desc_num], &desc_buf_info_tmpl,
               sizeof(VkDescriptorBufferInfo));
        desc_buf_infos[desc_num].offset =
            desc_num * gear_uniform_data_size;
    }

    VkWriteDescriptorSet desc_write_tmpl = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = 0,
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .pBufferInfo = &(VkDescriptorBufferInfo) {
        }
    };
    VkWriteDescriptorSet desc_write_sets[NUM_IMAGES * GEARS];
    for (desc_num = 0; desc_num < NUM_IMAGES * GEARS; desc_num++) {
        memcpy(&desc_write_sets[desc_num], &desc_write_tmpl,
               sizeof(VkWriteDescriptorSet));
        desc_write_sets[desc_num].dstSet = descriptor_sets[desc_num];
        desc_write_sets[desc_num].pBufferInfo = &desc_buf_infos[desc_num];
    }
    VFN(vkUpdateDescriptorSets)(device, NUM_IMAGES * GEARS, desc_write_sets, 0,
                                NULL);

    VkShaderModule vert_module;
    VkShaderModuleCreateInfo module_create_info = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = vk_vert_spirv_len,
        .pCode = (void*)vk_vert_spirv,
    };
    res = VFN(vkCreateShaderModule)(device, &module_create_info, NULL,
                                    &vert_module);
    assert(res == VK_SUCCESS);

    VkShaderModule frag_module;
    module_create_info.codeSize = vk_frag_spirv_len;
    module_create_info.pCode = (void*)vk_frag_spirv;
    res = VFN(vkCreateShaderModule)(device, &module_create_info, NULL,
                                    &frag_module);
    assert(res == VK_SUCCESS);

    VkPipelineShaderStageCreateInfo stages_create_info[] = {
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = vert_module,
            .pName = "main",
        },
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = frag_module,
            .pName = "main",
        },
    };

    VkVertexInputBindingDescription vert_input_binding_desc[] = {
        {
            .binding = 0,
            .stride = 6 * sizeof(float),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
        },
    };
    VkVertexInputAttributeDescription vert_input_attrib_desc[] = {
        {
            .location = 0,
            .binding = 0,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = 0,
        },
        {
            .location = 1,
            .binding = 0,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = 3 * sizeof(float),
        },
    };
    VkPipelineVertexInputStateCreateInfo vert_input_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = vert_input_binding_desc,
        .vertexAttributeDescriptionCount = 2,
        .pVertexAttributeDescriptions = vert_input_attrib_desc,
    };

    VkPipelineInputAssemblyStateCreateInfo input_assembly_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
    };

    VkPipelineViewportStateCreateInfo viewport_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount = 1,
    };

    VkPipelineRasterizationStateCreateInfo rasterization_state_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
    };

    VkPipelineMultisampleStateCreateInfo multisample_state_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = 1,
    };

    VkPipelineDepthStencilStateCreateInfo depth_stencil_state_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp = VK_COMPARE_OP_LESS,
    };

    VkPipelineColorBlendAttachmentState attachment_state = {
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    };
    VkPipelineColorBlendStateCreateInfo blend_state_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &attachment_state,
    };

    VkDynamicState dynamic_states[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };
    VkPipelineDynamicStateCreateInfo dynamic_state_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = 2,
        .pDynamicStates = dynamic_states,
    };

    VkGraphicsPipelineCreateInfo pipeline_create_info = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = 2,
        .pStages = stages_create_info,
        .pVertexInputState = &vert_input_create_info,
        .pInputAssemblyState = &input_assembly_state,
        .pViewportState = &viewport_create_info,
        .pRasterizationState = &rasterization_state_info,
        .pMultisampleState = &multisample_state_info,
        .pDepthStencilState = &depth_stencil_state_info,
        .pColorBlendState = &blend_state_info,
        .pDynamicState = &dynamic_state_info,
        .layout = pipeline_layout,
        .renderPass = render_pass,
    };
    res = VFN(vkCreateGraphicsPipelines)(device, VK_NULL_HANDLE, 1,
                                         &pipeline_create_info, NULL,
                                         &pipeline);
    assert(res == VK_SUCCESS);

    VkCommandBufferAllocateInfo cmd_buf_alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = cmd_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = NUM_IMAGES,
    };
    res = VFN(vkAllocateCommandBuffers)(device, &cmd_buf_alloc_info,
                                        cmd_buffers);
    assert(res == VK_SUCCESS);
}

static VkSwapchainKHR swapchain = VK_NULL_HANDLE;
static VkImage depth_image = VK_NULL_HANDLE;
static VkDeviceMemory depth_mem = VK_NULL_HANDLE;
static VkImage non_wsi_images[NUM_IMAGES] = { VK_NULL_HANDLE, };
static VkDeviceMemory non_wsi_mem = VK_NULL_HANDLE;
static pthread_mutex_t win_size_lock = PTHREAD_MUTEX_INITIALIZER;
static VkDeviceMemory capture_mem = VK_NULL_HANDLE;
static VkBuffer capture_buf = VK_NULL_HANDLE;

static void
create_wsi_images(int width, int height, VkImage *images)
{
    VkResult res;

    VkSurfaceCapabilitiesKHR surf_caps;
    res = VFN(vkGetPhysicalDeviceSurfaceCapabilitiesKHR)(phy_device, surface,
                                                         &surf_caps);
    assert(res == VK_SUCCESS);

    VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
    if (!gears_options.vsync) {
        if (immediate_present_supported)
            present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
        else if (mailbox_present_supported)
            present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
    }

    uint32_t request_num_images = NUM_IMAGES;
    if (surf_caps.minImageCount > request_num_images)
        request_num_images = surf_caps.minImageCount;
    if (surf_caps.maxImageCount != 0 &&
        surf_caps.maxImageCount < request_num_images)
        request_num_images = surf_caps.maxImageCount;
    assert(request_num_images <= NUM_IMAGES);

    uint32_t family_indices[] = { 0 };
    VkSwapchainCreateInfoKHR swapchain_create_info = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = 0,
        .flags = 0,
        .surface = surface,
        .minImageCount = request_num_images,
        .imageFormat = VK_FORMAT_B8G8R8A8_UNORM,
        .imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
        .imageExtent = { width, height },
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                      (gears_options.output_type != OUTPUT_NONE ?
                       VK_IMAGE_USAGE_TRANSFER_SRC_BIT : 0),
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 1,
        .pQueueFamilyIndices = family_indices,
        .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = present_mode,
        .clipped = VK_FALSE,
        .oldSwapchain = 0,
    };
    res = VFN(vkCreateSwapchainKHR)(device, &swapchain_create_info, NULL,
                                    &swapchain);
    assert(res == VK_SUCCESS);

    uint32_t wsi_image_count;
    res = VFN(vkGetSwapchainImagesKHR)(device, swapchain, &wsi_image_count,
                                       NULL);
    assert(res == VK_SUCCESS);

    num_images = NUM_IMAGES;
    res = VFN(vkGetSwapchainImagesKHR)(device, swapchain, &num_images,
                                       images);
    assert(res == VK_SUCCESS ||
           (res == VK_INCOMPLETE && num_images < wsi_image_count));

    if (num_images > surf_caps.minImageCount)
        max_in_flight_images = num_images - surf_caps.minImageCount;
    else
        max_in_flight_images = 1;
}

static void
create_non_wsi_images(int width, int height, VkImage *images)
{
    VkResult res;

    VkImageCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = VK_FORMAT_B8G8R8A8_UNORM,
        .extent = { .width = width, .height = height, .depth = 1 },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    num_images = NUM_IMAGES;

    int image_num;
    for (image_num = 0; image_num < num_images; image_num++) {
        res = VFN(vkCreateImage)(device, &info, NULL, &images[image_num]);
        assert(res == VK_SUCCESS);
        non_wsi_images[image_num] = images[image_num];
    }

    VkMemoryRequirements mem_req[NUM_IMAGES];
    for (image_num = 0; image_num < num_images; image_num++) {
        VFN(vkGetImageMemoryRequirements)(device, images[image_num],
                                          &mem_req[image_num]);
    }

    uint32_t mem_ty_idx =
        find_memory_idx(mem_req[0].memoryTypeBits,
                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    for (image_num = 1; image_num < num_images; image_num++) {
        assert(find_memory_idx(mem_req[image_num].memoryTypeBits,
                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) ==
               mem_ty_idx);
    }

    VkDeviceSize image_size = 0;
    uint32_t image_offsets[NUM_IMAGES];
    image_offsets[0] = 0;
    for (image_num = 1; image_num < num_images; image_num++) {
        image_size = ALIGN(image_size, mem_req[image_num].alignment);
        image_offsets[image_num] = image_size;
        image_size += mem_req[image_num].size;
    }
    VkMemoryAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = image_size,
        .memoryTypeIndex = mem_ty_idx,
    };

    res = VFN(vkAllocateMemory)(device, &alloc_info, NULL, &non_wsi_mem);
    assert(res == VK_SUCCESS);

    for (image_num = 0; image_num < num_images; image_num++) {
        res = VFN(vkBindImageMemory)(device, images[image_num], non_wsi_mem,
                                     image_offsets[image_num]);
        assert(res == VK_SUCCESS);
    }
}

static void
create_pipeline(int width, int height)
{
    VkResult res;

    pthread_mutex_lock(&win_size_lock);

    if (swapchain != VK_NULL_HANDLE) {
        VFN(vkDestroySwapchainKHR)(device, swapchain, NULL);
        swapchain = VK_NULL_HANDLE;
    }

    VkImage images[NUM_IMAGES];

    switch (active_winsys) {
    case WINSYS_NONE:
        create_non_wsi_images(width, height, images);
        break;
    case WINSYS_WAYLAND:
    case WINSYS_X11:
        create_wsi_images(width, height, images);
        break;
    default:
        assert(false);
    }

    if (depth_image != VK_NULL_HANDLE) {
        assert(depth_mem != VK_NULL_HANDLE);
        VFN(vkDestroyImage)(device, depth_image, NULL);
        depth_image = VK_NULL_HANDLE;
        VFN(vkFreeMemory)(device, depth_mem, NULL);
        depth_mem = VK_NULL_HANDLE;
    }

    VkImageCreateInfo depth_image_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = VK_FORMAT_D16_UNORM,
        .extent = { .width = width, .height = height, .depth = 1 },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        .initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };
    res = VFN(vkCreateImage)(device, &depth_image_info, NULL, &depth_image);
    assert(res == VK_SUCCESS);

    VkMemoryRequirements depth_mem_req;
    VFN(vkGetImageMemoryRequirements)(device, depth_image, &depth_mem_req);

    uint32_t mem_ty_idx =
        find_memory_idx(depth_mem_req.memoryTypeBits,
                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkMemoryAllocateInfo depth_alloc_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = depth_mem_req.size,
        .memoryTypeIndex = mem_ty_idx,
    };
    res = VFN(vkAllocateMemory)(device, &depth_alloc_info, NULL, &depth_mem);
    assert(res == VK_SUCCESS);

    res = VFN(vkBindImageMemory)(device, depth_image, depth_mem, 0);
    assert(res == VK_SUCCESS);

    if (gears_options.output_type != OUTPUT_NONE) {
        if (capture_buf != VK_NULL_HANDLE) {
            assert(capture_mem != VK_NULL_HANDLE);
            VFN(vkDestroyBuffer)(device, capture_buf, NULL);
            capture_buf = VK_NULL_HANDLE;
            VFN(vkFreeMemory)(device, capture_mem, NULL);
            capture_mem = VK_NULL_HANDLE;
        }

        unsigned int capture_data_size = 4 * width * height;
        VkBufferCreateInfo capture_buf_alloc_info = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = capture_data_size,
            .usage = VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT |
            VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        };
        res = VFN(vkCreateBuffer)(device, &capture_buf_alloc_info, NULL,
                                  &capture_buf);
        assert(res == VK_SUCCESS);

        VkMemoryRequirements capture_mem_req;
        VFN(vkGetBufferMemoryRequirements)(device, capture_buf, &capture_mem_req);

        uint32_t mem_ty_idx =
            find_memory_idx(capture_mem_req.memoryTypeBits,
                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        VkMemoryAllocateInfo capture_alloc_info = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = capture_mem_req.size,
            .memoryTypeIndex = mem_ty_idx,
        };
        res = VFN(vkAllocateMemory)(device, &capture_alloc_info, NULL, &capture_mem);
        assert(res == VK_SUCCESS);

        res = VFN(vkBindBufferMemory)(device, capture_buf, capture_mem, 0);
        assert(res == VK_SUCCESS);
    }

    VkBufferImageCopy copy_region = {
        .bufferOffset = 0,
        .bufferRowLength = width,
        .bufferImageHeight = height,
        .imageSubresource = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
        .imageOffset = { .x = 0, .y = 0, .z = 0 },
        .imageExtent = { .width = width, .height = height, .depth = 1 },
    };

    int image_num;
    for (image_num = 0; image_num < num_images; image_num++) {

        VkImageViewCreateInfo image_view_create_info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = images[image_num],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = VK_FORMAT_B8G8R8A8_UNORM,
            .components = {
                .r = VK_COMPONENT_SWIZZLE_R,
                .g = VK_COMPONENT_SWIZZLE_G,
                .b = VK_COMPONENT_SWIZZLE_B,
                .a = VK_COMPONENT_SWIZZLE_A,
            },
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            }
        };
        VkImageViewCreateInfo depth_image_view_create_info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = depth_image,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = VK_FORMAT_D16_UNORM,
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        };
        VkImageView image_views[2];
        res = VFN(vkCreateImageView)(device, &image_view_create_info, NULL,
                                     &image_views[0]);
        assert(res == VK_SUCCESS);
        res = VFN(vkCreateImageView)(device, &depth_image_view_create_info,
                                     NULL, &image_views[1]);
        assert(res == VK_SUCCESS);

        VkFramebufferCreateInfo fb_create_info = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = render_pass,
            .attachmentCount = 2,
            .pAttachments = image_views,
            .width = width,
            .height = height,
            .layers = 1,
        };
        VkFramebuffer framebuffer;
        res = VFN(vkCreateFramebuffer)(device, &fb_create_info, NULL,
                                       &framebuffer);
        assert(res == VK_SUCCESS);

        VkCommandBufferBeginInfo cmd_buf_begin_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        };
        res = VFN(vkBeginCommandBuffer)(cmd_buffers[image_num],
                                        &cmd_buf_begin_info);
        assert(res == VK_SUCCESS);

        VkClearValue clear_values[] = {
            { .color = { .float32 = { 0.0f, 0.0f, 0.0f, 1.0f } } },
            { .depthStencil = { .depth = 1.0 } },
        };
        VkRenderPassBeginInfo render_pass_begin_info = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = render_pass,
            .framebuffer = framebuffer,
            .renderArea = { { 0, 0 }, { width, height } },
            .clearValueCount = 2,
            .pClearValues = clear_values,
        };
        VkCommandBuffer cmd_buf = cmd_buffers[image_num];
        VFN(vkCmdBeginRenderPass)(cmd_buf, &render_pass_begin_info,
                                  VK_SUBPASS_CONTENTS_INLINE);
        VkBuffer buffers[] = { vert_buf, };
        VFN(vkCmdBindVertexBuffers)(cmd_buf, 0, 1, buffers,
                                    (VkDeviceSize[]) { 0 });
        VFN(vkCmdBindPipeline)(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS,
                               pipeline);

        const VkViewport viewport = {
            .x = 0, .y = 0,
            .width = width, .height = height,
            .minDepth = 0.0, .maxDepth = 1.0,
        };
        VFN(vkCmdSetViewport)(cmd_buf, 0, 1, &viewport);

        const VkRect2D scissor = {
            .offset = { 0, 0 },
            .extent = { width, height },
        };
        VFN(vkCmdSetScissor)(cmd_buf, 0, 1, &scissor);

        int gear_num;
        for (gear_num = 0; gear_num < GEARS; gear_num++) {
            VFN(vkCmdBindDescriptorSets)(cmd_buf,
                                         VK_PIPELINE_BIND_POINT_GRAPHICS,
                                         pipeline_layout, 0, 1,
                                         &descriptor_sets[gear_num], 0, NULL);
            VFN(vkCmdPushConstants)(cmd_buf, pipeline_layout,
                                    VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                                    sizeof(gears[gear_num].color),
                                    (void*)gears[gear_num].color);
            VFN(vkCmdDraw)(cmd_buf, gears[gear_num].num_vertices,
                           gears[gear_num].teeth,
                           gears[gear_num].vertex_buf_offset, 0);
        }

        VFN(vkCmdEndRenderPass)(cmd_buf);
        if (gears_options.output_type != OUTPUT_NONE) {
            VFN(vkCmdCopyImageToBuffer)(cmd_buf, images[image_num],
                                        VK_IMAGE_LAYOUT_GENERAL,
                                        capture_buf, 1,
                                        &copy_region);
        }
        VFN(vkEndCommandBuffer)(cmd_buf);
    }

    pthread_mutex_unlock(&win_size_lock);

}

static void
update_angle(float angle)
{
    static float last_angle = 0.0f;

    if (!update_angle_uniform && last_angle == angle)
        return;

    int i;
    for (i = 0; i < GEARS; i++) {
        gears[i].angle = angle * gears[i].angle_rate + gears[i].angle_adjust;
    }

    update_angle_uniform = true;
    last_angle = angle;
}

static void
update_uniform_gear_angles(unsigned int img)
{
    if (!update_angle_uniform)
        return;

    struct vs_uniform_data *gear_u_data;
    unsigned int i;
    for (i = 0; i < GEARS; i++) {
        gear_u_data = gear_uniform_data(uniform_map, img, i);
        gear_u_data->gear_angle = gears[i].angle;
    }

    VkMappedMemoryRange mem_range = {
        .sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
        .pNext = NULL,
        .memory = uniform_mem,
        .offset = img * img_uniform_data_size,
        .size = img_uniform_data_size,
    };
    VkResult res = VFN(vkFlushMappedMemoryRanges)(device, 1, &mem_range);
    assert(res == VK_SUCCESS);
    update_angle_uniform = false;
}

static void
read_pixels(void *rgba8)
{
    VkResult res;
    uint8_t *capture_map;
    unsigned buf_size = 4 * sim_width * sim_height;
    res = VFN(vkMapMemory)(device, capture_mem, 0, buf_size, 0,
                           (void**)&capture_map);
    assert(res == VK_SUCCESS);

    unsigned int p;
    for (p = 0; p < buf_size; p += 4) {
        ((uint8_t*)rgba8)[p + 0] = capture_map[p + 2];
        ((uint8_t*)rgba8)[p + 1] = capture_map[p + 1];
        ((uint8_t*)rgba8)[p + 2] = capture_map[p + 0];
        ((uint8_t*)rgba8)[p + 3] = capture_map[p + 3];
    }

    VFN(vkUnmapMemory)(device, capture_mem);
    capture_map = NULL;
}

static bool
post_draw(void)
{
    if (gears_options.output_type == OUTPUT_NONE)
        return true;

    if (ready_for_new_frame()) {
        unsigned buf_size = 4 * sim_width * sim_height;
        void *rgba8 = malloc(buf_size);
        read_pixels(rgba8);
        new_frame_data(rgba8);
        free(rgba8);
    }

    return true;
}

static void flush_one_render();

static int32_t
get_wsi_image()
{
    uint32_t index;
    VkResult res;
    uint64_t start_ms = get_real_time_ms();

    do {
        uint64_t wait_time = submitted_render_count > 0 ? 0 : 1000000;
        res = VFN(vkAcquireNextImageKHR)(device, swapchain, wait_time,
                                         VK_NULL_HANDLE, VK_NULL_HANDLE,
                                         &index);
        if (res == VK_NOT_READY && submitted_render_count > 0) {
            flush_one_render();
        }
        if (res == VK_TIMEOUT || res == VK_NOT_READY) {
            if (get_real_time_ms() - start_ms > 1100)
                break;
        }
    } while (res == VK_TIMEOUT || res == VK_NOT_READY || index >= num_images);

    /* The window was probably resized, so skip this frame */
    if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR) {
        return -1;
    }

    assert(res == VK_SUCCESS);

    return index;
}

static void
wsi_present(uint32_t index)
{
    VkResult res;
    VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &semaphores[index],
        .swapchainCount = 1,
        .pSwapchains = &swapchain,
        .pImageIndices = &index,
        .pResults = &res,
    };
    res = VFN(vkQueuePresentKHR)(queue, &present_info);
    assert(res == VK_SUCCESS || res == VK_SUBOPTIMAL_KHR);
}

static int32_t
get_non_wsi_image()
{
    static int32_t index = 0;
    int32_t ret_index = index;
    index = (index + 1) % NUM_IMAGES;
    return ret_index;
}

static int
submitted_pos()
{
    return
        (next_render_pos + num_images - submitted_render_count) % num_images;
}

static void
flush_one_render()
{
    assert(submitted_render_count > 0);
    int index = in_flight_indices[submitted_pos()];
    VkResult res =
        VFN(vkWaitForFences)(device, 1, &fences[index], true, INT64_MAX);
    assert(res == VK_SUCCESS);

    post_draw();

    submitted_render_count--;
}

static void
check_for_complete_frames()
{
    int pos = submitted_pos();
    while (submitted_render_count > 0) {
        int index = in_flight_indices[pos];
        VkResult res = VFN(vkGetFenceStatus)(device, fences[index]);
        if (res == VK_NOT_READY)
            return;

        post_draw();

        submitted_render_count--;
        pos = (pos + 1) % num_images;
    }
}

static void
flush_all_renders()
{
    check_for_complete_frames();
    while (submitted_render_count > 0)
        flush_one_render();
}

static void
draw()
{
    VkResult res;
    int32_t index;

    pthread_mutex_lock(&win_size_lock);

    if (submitted_render_count >= max_in_flight_images)
        flush_one_render();

    if (submitted_render_count > 0)
        check_for_complete_frames();

    index = using_wsi ? get_wsi_image() : get_non_wsi_image();

    if (index < 0) {
        pthread_mutex_unlock(&win_size_lock);
        return;
    }

    update_uniform_gear_angles(index);

    VkPipelineStageFlags state_flags =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .signalSemaphoreCount = using_wsi ? 1 : 0,
        .pSignalSemaphores = &semaphores[index],
        .pWaitDstStageMask = &state_flags,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd_buffers[index],
    };
    res = VFN(vkQueueSubmit)(queue, 1, &submit_info,
                             using_wsi ? VK_NULL_HANDLE : fences[index]);
    assert(res == VK_SUCCESS);

    if (using_wsi) {
        wsi_present(index);
        VkSubmitInfo submit_info = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        };
        res = VFN(vkQueueSubmit)(queue, 1, &submit_info,
                                 fences[index]);
    }

    submitted_render_count++;
    in_flight_indices[next_render_pos] = index;
    next_render_pos = (next_render_pos + 1) % num_images;

    pthread_mutex_unlock(&win_size_lock);
}

static void
destruct()
{
    pthread_mutex_lock(&win_size_lock);
    flush_all_renders();
    pthread_mutex_unlock(&win_size_lock);
}

struct gears_drawer vk10_drawer = {
    .name = "Vulkan 1.0 Gears",
    .api_type = API_VULKAN,
    .set_window_attributes = set_window_attributes,
    .set_global_state = set_global_state,
    .resize = win_resize,
    .update_angle = update_angle,
    .draw = draw,
    .destruct = destruct,
};
