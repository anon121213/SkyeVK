cmake_minimum_required(VERSION 3.21)
project(SkyRenderer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


find_package(Vulkan REQUIRED)

add_library(SkyRenderer STATIC
        src/Vulkan/VulkanInstance.cpp
        src/Vulkan/VulkanDevice.cpp
        src/Vulkan/VulkanSurface.cpp
        src/Vulkan/VMAImplementation.cpp
        include/SkyRenderer/VulkanSwapchain.h
        src/Vulkan/VulkanSwapchain.cpp
        include/SkyRenderer/VulkanRenderPass.h
        src/Vulkan/VulkanRenderPass.cpp
        include/SkyRenderer/VulkanFramebuffers.h
        src/Vulkan/VulkanFramebuffers.cpp
        include/SkyRenderer/VulkanShaderModule.h
        src/Vulkan/VulkanShaderModule.cpp
        include/SkyRenderer/VulkanPipeline.h
        src/Vulkan/VulkanPipeline.cpp
        include/SkyRenderer/VulkanCommandPool.h
        src/Vulkan/VulkanCommandPool.cpp
        include/SkyRenderer/VulkanRenderer.h
        src/Vulkan/VulkanRenderer.cpp
)

include(FetchContent)

FetchContent_Declare(
        vma
        GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
        GIT_TAG        v3.3.0
        GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(vma)

target_include_directories(SkyRenderer PUBLIC include)
target_link_libraries(SkyRenderer PUBLIC
        Vulkan::Vulkan
        GPUOpen::VulkanMemoryAllocator
)

if (PROJECT_IS_TOP_LEVEL)
    add_subdirectory(SkyApp)
endif()