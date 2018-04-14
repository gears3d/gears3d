/* Jordan Justen : gears3d is public domain */

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
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
DECL_PVKFN(vkDestroyImage);
DECL_PVKFN(vkDestroySwapchainKHR);
DECL_PVKFN(vkEndCommandBuffer);
DECL_PVKFN(vkFreeMemory);
DECL_PVKFN(vkGetBufferMemoryRequirements);
DECL_PVKFN(vkGetDeviceQueue);
DECL_PVKFN(vkGetImageMemoryRequirements);
DECL_PVKFN(vkGetSwapchainImagesKHR);
DECL_PVKFN(vkMapMemory);
DECL_PVKFN(vkQueuePresentKHR);
DECL_PVKFN(vkQueueSubmit);
DECL_PVKFN(vkResetFences);
DECL_PVKFN(vkUnmapMemory);
DECL_PVKFN(vkUpdateDescriptorSets);
DECL_PVKFN(vkWaitForFences);

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

const char *enable_extensions[] = {
    "VK_KHR_wayland_surface",
    "VK_KHR_xlib_surface",
    "VK_KHR_surface",
};
static const VkInstanceCreateInfo create_info = {
    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    .pApplicationInfo = &(VkApplicationInfo) {
        .pApplicationName = "gears3d",
        .apiVersion = VK_MAKE_VERSION(1, 0, 0),
    },
    .ppEnabledExtensionNames = enable_extensions,
    .enabledExtensionCount = 3,
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
#define NUM_IMAGES 2

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
static VkSemaphore semaphore;
static VkFence fence;

static VkCommandBuffer cmd_buffers[NUM_IMAGES];
static VkDeviceMemory vert_mem;
static VkBuffer vert_buf;
static VkDeviceMemory uniform_mem;
static VkBuffer uniform_buf;
static VkDescriptorSet descriptor_sets[GEARS];
static VkRenderPass render_pass;
static VkPipelineLayout pipeline_layout;
static VkPipeline pipeline;

static unsigned int gear_uniform_data_size;
static unsigned int uniform_data_size;

static void init_vk_instance()
{
    VkResult res;
    void *lib = open_library("libvulkan.so.1");
    if (lib == NULL)
        abort();

#define DLSYM(f) \
    VFN(f) = get_library_symbol(lib, #f); \
    assert(VFN(f) != NULL)

    DLSYM(vkCreateInstance);
    DLSYM(vkGetInstanceProcAddr);
    DLSYM(vkGetDeviceProcAddr);

    res = VFN(vkCreateInstance)(&create_info, &alloc_callbacks, &instance);
    assert(res == VK_SUCCESS);

#define GET_I_PROC(f) \
    VFN(f) = (void*)VFN(vkGetInstanceProcAddr)(instance, #f);   \
    assert(VFN(f) != NULL)

    GET_I_PROC(vkCreateDevice);
    GET_I_PROC(vkCreateWaylandSurfaceKHR);
    GET_I_PROC(vkCreateXlibSurfaceKHR);
    GET_I_PROC(vkEnumeratePhysicalDevices);
    GET_I_PROC(vkGetPhysicalDeviceProperties);
    GET_I_PROC(vkGetPhysicalDeviceQueueFamilyProperties);
    GET_I_PROC(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
    GET_I_PROC(vkGetPhysicalDeviceSurfaceFormatsKHR);
    GET_I_PROC(vkGetPhysicalDeviceSurfacePresentModesKHR);
    GET_I_PROC(vkGetPhysicalDeviceSurfaceSupportKHR);
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
    GET_D_PROC(vkDestroyImage);
    GET_D_PROC(vkDestroySwapchainKHR);
    GET_D_PROC(vkEndCommandBuffer);
    GET_D_PROC(vkFreeMemory);
    GET_D_PROC(vkGetBufferMemoryRequirements);
    GET_D_PROC(vkGetDeviceQueue);
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
    res = VFN(vkCreateSemaphore)(device, &semaphore_create_info, NULL,
                                 &semaphore);
    assert(res == VK_SUCCESS);

    VkFenceCreateInfo fence_create_info = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO
    };
    res = VFN(vkCreateFence)(device, &fence_create_info, NULL, &fence);
    assert(res == VK_SUCCESS);
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
gear_uniform_data(void *uniform_map, unsigned int gear)
{
    return uniform_map + gear * gear_uniform_data_size;
}

static void
set_gear_const_uniform_data(void *uniform_map)
{
    struct vs_uniform_data *gear_u_data;
    unsigned int gear;

    rotate_gears(20.0 / 180.0 * M_PI, 30.0 / 180.0 * M_PI, 0.0);

    for (gear = 0; gear < GEARS; gear++) {
        gear_u_data = gear_uniform_data(uniform_map, gear);
        memcpy(gear_u_data->model, gears[gear].model,
               sizeof(gears[gear].model));
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

    for (gear = 0; gear < GEARS; gear++) {
        gear_u_data = gear_uniform_data(uniform_map, gear);
        memcpy(gear_u_data->view, view, sizeof(view));
        memcpy(gear_u_data->projection, projection, sizeof(projection));
        gear_u_data->tooth_angle = M_PI / (gears[gear].teeth / 2.0);
    }
}

static void
create_pipeline(int width, int height);

static void
reshape(int width, int height)
{
    VkResult res;
    struct vs_uniform_data *uniform_map;

    res = VFN(vkMapMemory)(device, uniform_mem, 0, uniform_data_size, 0,
                           (void**)&uniform_map);
    assert(res == VK_SUCCESS);

    set_view_projection(width, height, uniform_map);

    VFN(vkUnmapMemory)(device, uniform_mem);

    create_pipeline(width, height);
}

static void
win_resize(int width, int height)
{
    reshape(width, height);
}

static void
set_global_state()
{
    Display *dpy;
    Window wnd;
    bool got_x11 = get_x11r6_dpy_wnd(&dpy, &wnd);
    struct wl_display *wl_dpy;
    struct wl_surface *wl_srf;
    bool got_wl = get_wl_dpy_srf(&wl_dpy, &wl_srf);
    assert(got_x11 || got_wl);

    VkResult res;
    int i;

    VkAttachmentDescription att_desc[] = {
        {
            .format = VK_FORMAT_B8G8R8A8_UNORM,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        },
        {
            .format = VK_FORMAT_D16_UNORM,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .stencilLoadOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
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
    uint32_t preserve_att[] = { 0 };
    VkSubpassDescription subpass_desc = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .inputAttachmentCount = 0,
        .colorAttachmentCount = 1,
        .pColorAttachments = &att_ref,
        .pResolveAttachments = &unused_att_ref,
        .pDepthStencilAttachment = &depth_att_ref,
        .preserveAttachmentCount = 1,
        .pPreserveAttachments = preserve_att,
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

    if (got_wl) {
        VkWaylandSurfaceCreateInfoKHR wl_surf_create_info = {
            .sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
            .display = wl_dpy,
            .surface = wl_srf,
            .flags = 0,
            .pNext = NULL,
        };

        res = VFN(vkCreateWaylandSurfaceKHR)(instance, &wl_surf_create_info,
                                             NULL, &surface);
        assert(res == VK_SUCCESS);
    } else {
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
    }

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

    VkSurfaceCapabilitiesKHR surf_caps;
    res = VFN(vkGetPhysicalDeviceSurfaceCapabilitiesKHR)(phy_device, surface,
                                                         &surf_caps);
    assert(res == VK_SUCCESS);
    /* TODO: check capabilities */

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
    uint32_t mem_ty_idx = ffs(mem_req.memoryTypeBits);
    assert(mem_ty_idx != 0);
    mem_ty_idx--;

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
    uniform_data_size = GEARS * gear_uniform_data_size;
    alloc_info.allocationSize = uniform_data_size;
    res = VFN(vkAllocateMemory)(device, &alloc_info, NULL, &uniform_mem);
    assert(res == VK_SUCCESS);

    struct vs_uniform_data *uniform_map;
    res = VFN(vkMapMemory)(device, uniform_mem, 0, uniform_data_size, 0,
                           (void**)&uniform_map);
    assert(res == VK_SUCCESS);

    memset(uniform_map, 0, uniform_data_size);
    set_gear_const_uniform_data(uniform_map);

    VFN(vkUnmapMemory)(device, uniform_mem);
    uniform_map = NULL;

    VkBufferCreateInfo uniform_buf_alloc_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = uniform_data_size,
        .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
    };
    res = VFN(vkCreateBuffer)(device, &uniform_buf_alloc_info, NULL,
                              &uniform_buf);
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
    };
    res = VFN(vkCreatePipelineLayout)(device, &pipeline_layout_info, NULL,
                                      &pipeline_layout);
    assert(res == VK_SUCCESS);

    VkDescriptorPoolSize pool_size = {
        .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = GEARS,
    };
    VkDescriptorPoolCreateInfo desc_pool_create_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .maxSets = GEARS,
        .poolSizeCount = 1,
        .pPoolSizes = &pool_size,
    };
    VkDescriptorPool desc_pool;
    res = VFN(vkCreateDescriptorPool)(device, &desc_pool_create_info, NULL,
                                      &desc_pool);
    assert(res == VK_SUCCESS);

    VkDescriptorSetAllocateInfo desc_set_alloc_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = desc_pool,
        .descriptorSetCount = 1,
        .pSetLayouts = &set_layout,
    };
    int gear_num;
    for (gear_num = 0; gear_num < GEARS; gear_num++) {
        res = VFN(vkAllocateDescriptorSets)(device, &desc_set_alloc_info,
                                            &descriptor_sets[gear_num]);
        assert(res == VK_SUCCESS);
    }

    VkDescriptorBufferInfo desc_buf_info_tmpl = {
        .buffer = uniform_buf,
        .offset = 0,
        .range = sizeof(struct vs_uniform_data),
    };
    VkDescriptorBufferInfo desc_buf_infos[GEARS];
    for (gear_num = 0; gear_num < GEARS; gear_num++) {
        memcpy(&desc_buf_infos[gear_num], &desc_buf_info_tmpl,
               sizeof(VkDescriptorBufferInfo));
        desc_buf_infos[gear_num].offset =
            gear_num * gear_uniform_data_size;
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
    VkWriteDescriptorSet desc_write_sets[GEARS];
    for (gear_num = 0; gear_num < GEARS; gear_num++) {
        memcpy(&desc_write_sets[gear_num], &desc_write_tmpl,
               sizeof(VkWriteDescriptorSet));
        desc_write_sets[gear_num].dstSet = descriptor_sets[gear_num];
        desc_write_sets[gear_num].pBufferInfo = &desc_buf_infos[gear_num];
    }
    VFN(vkUpdateDescriptorSets)(device, GEARS, desc_write_sets, 0, NULL);

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
        {
            .binding = 1,
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
            .binding = 1,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = 0,
        },
    };
    VkPipelineVertexInputStateCreateInfo vert_input_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 2,
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
        .depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
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
static pthread_mutex_t win_size_lock = PTHREAD_MUTEX_INITIALIZER;

static void
create_pipeline(int width, int height)
{
    VkResult res;

    pthread_mutex_lock(&win_size_lock);

    if (swapchain != VK_NULL_HANDLE) {
        VFN(vkDestroySwapchainKHR)(device, swapchain, NULL);
        swapchain = VK_NULL_HANDLE;
    }

    VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
    if (!gears_options.vsync) {
        if (immediate_present_supported)
            present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
        else if (mailbox_present_supported)
            present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
    }

    uint32_t family_indices[] = { 0 };
    VkSwapchainCreateInfoKHR swapchain_create_info = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = 0,
        .flags = 0,
        .surface = surface,
        .minImageCount = NUM_IMAGES,
        .imageFormat = VK_FORMAT_B8G8R8A8_UNORM,
        .imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
        .imageExtent = { width, height },
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
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

    uint32_t image_count;
    res = VFN(vkGetSwapchainImagesKHR)(device, swapchain, &image_count, NULL);
    assert(res == VK_SUCCESS && image_count >= NUM_IMAGES);

    VkImage images[NUM_IMAGES];
    image_count = NUM_IMAGES;
    res = VFN(vkGetSwapchainImagesKHR)(device, swapchain, &image_count,
                                       images);
    assert(res == VK_SUCCESS);

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
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };
    res = VFN(vkCreateImage)(device, &depth_image_info, NULL, &depth_image);
    assert(res == VK_SUCCESS);

    VkMemoryRequirements depth_mem_req;
    VFN(vkGetImageMemoryRequirements)(device, depth_image, &depth_mem_req);

    uint32_t mem_ty_idx = ffs(depth_mem_req.memoryTypeBits);
    assert(mem_ty_idx != 0);
    mem_ty_idx--;

    VkMemoryAllocateInfo depth_alloc_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = depth_mem_req.size,
        .memoryTypeIndex = mem_ty_idx,
    };
    res = VFN(vkAllocateMemory)(device, &depth_alloc_info, NULL, &depth_mem);
    assert(res == VK_SUCCESS);

    res = VFN(vkBindImageMemory)(device, depth_image, depth_mem, 0);
    assert(res == VK_SUCCESS);

    int image_num;
    for (image_num = 0; image_num < NUM_IMAGES; image_num++) {

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
        VkBuffer buffers[] = { vert_buf, vert_buf, };
        VFN(vkCmdBindVertexBuffers)(cmd_buf, 0, 2, buffers,
                                    (VkDeviceSize[]) { 0, 3 * sizeof(float) });
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
        VFN(vkEndCommandBuffer)(cmd_buf);
    }

    pthread_mutex_unlock(&win_size_lock);

}

static void
update_angle(float angle)
{
    int i;
    for (i = 0; i < GEARS; i++) {
        gears[i].angle = angle * gears[i].angle_rate + gears[i].angle_adjust;
    }
}

static void
update_uniform_gear_angles()
{
    VkResult res;
    struct vs_uniform_data *uniform_map;
    res = VFN(vkMapMemory)(device, uniform_mem, 0,
                           GEARS * gear_uniform_data_size, 0,
                           (void**)&uniform_map);
    assert(res == VK_SUCCESS);

    struct vs_uniform_data *gear_u_data;
    unsigned int i;
    for (i = 0; i < GEARS; i++) {
        gear_u_data = gear_uniform_data(uniform_map, i);
        gear_u_data->gear_angle = gears[i].angle;
    }

    VFN(vkUnmapMemory)(device, uniform_mem);
    uniform_map = NULL;
}

static void
draw()
{
    uint32_t timeouts = 0;
    uint32_t index;
    VkResult res;

    update_uniform_gear_angles();

    pthread_mutex_lock(&win_size_lock);

    do {
        res = VFN(vkAcquireNextImageKHR)(device, swapchain, UINT64_MAX,
                                         semaphore, VK_NULL_HANDLE, &index);
        if (res == VK_TIMEOUT) {
            timeouts++;
            if (timeouts > 20)
                break;
        }
    } while (res == VK_TIMEOUT);

    /* The window was probably resized, so skip this frame */
    if (res == VK_ERROR_OUT_OF_DATE_KHR) {
        pthread_mutex_unlock(&win_size_lock);
        return;
    }

    assert(res == VK_SUCCESS);

    VkPipelineStageFlags state_flags =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &semaphore,
        .pWaitDstStageMask = &state_flags,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd_buffers[index],
    };
    res = VFN(vkQueueSubmit)(queue, 1, &submit_info, fence);
    assert(res == VK_SUCCESS);

    res = VFN(vkWaitForFences)(device, 1, &fence, true, INT64_MAX);
    VFN(vkResetFences)(device, 1, &fence);

    VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .swapchainCount = 1,
        .pSwapchains = &swapchain,
        .pImageIndices = &index,
        .pResults = &res,
    };
    res = VFN(vkQueuePresentKHR)(queue, &present_info);
    assert(res == VK_SUCCESS);

    pthread_mutex_unlock(&win_size_lock);
}

static void
destruct()
{
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
