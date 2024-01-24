#include <chrono>
#include <cstdlib>
#include <iostream>
#include <vector>
#include "arcball_camera.h"
#include "import_gltf.h"
#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/html5_webgpu.h>
#include "webgpu_cpp.h"

#include "embedded_files.h"

struct AppState {
    wgpu::Device device;
    wgpu::Queue queue;

    wgpu::ShaderModule shader_module;

    wgpu::Surface surface;
    wgpu::SwapChain swap_chain;
    wgpu::Texture depth_texture;
    wgpu::Buffer view_param_buf;
    wgpu::BindGroup bind_group;

    ArcballCamera camera;
    glm::mat4 proj;

    bool done = false;
    bool camera_changed = true;
    glm::vec2 prev_mouse = glm::vec2(-2.f);

    std::unique_ptr<GLTFRenderData> gltf_model;

    // The new gltf model we've imported and will swap to render on the next frame
    std::unique_ptr<GLTFRenderData> new_gltf_model;
};

double css_w = 0.0;
double css_h = 0.0;

int win_width = 1280;
int win_height = 720;
float dpi = 2.f;

AppState *app_state = nullptr;

glm::vec2 transform_mouse(glm::vec2 in)
{
    return glm::vec2(in.x * 2.f / css_w - 1.f, 1.f - 2.f * in.y / css_h);
}

int mouse_move_callback(int type, const EmscriptenMouseEvent *event, void *user_data);
int mouse_wheel_callback(int type, const EmscriptenWheelEvent *event, void *user_data);

// Exported function called by the app to import and use a new gltf file
extern "C" EMSCRIPTEN_KEEPALIVE void load_gltf_buffer(const uint8_t *glb,
                                                      const size_t glb_size);

void loop_iteration(void *user_data);

int main(int argc, const char **argv)
{
    app_state = new AppState;

    // TODO: we can't call this because we also load this same wasm module into a worker
    // which doesn't have access to the window APIs
    dpi = emscripten_get_device_pixel_ratio();
    emscripten_get_element_css_size("#webgpu-canvas", &css_w, &css_h);
    std::cout << "Canvas element size = " << css_w << "x" << css_h << "\n";

    emscripten_get_canvas_element_size("#webgpu-canvas", &win_width, &win_height);
    std::cout << "Canvas size: " << win_width << "x" << win_height << "\n";
    win_width = css_w * dpi;
    win_height = css_h * dpi;
    std::cout << "Setting canvas size: " << win_width << "x" << win_height << "\n";

    emscripten_set_canvas_element_size("#webgpu-canvas", win_width, win_height);

    app_state->device = wgpu::Device::Acquire(emscripten_webgpu_get_device());

    wgpu::InstanceDescriptor instance_desc;
    wgpu::Instance instance = wgpu::CreateInstance(&instance_desc);

    app_state->device.SetUncapturedErrorCallback(
        [](WGPUErrorType type, const char *msg, void *data) {
            std::cout << "WebGPU Error: " << msg << "\n" << std::flush;
            emscripten_cancel_main_loop();
            emscripten_force_exit(1);
            std::exit(1);
        },
        nullptr);

    /*
    app_state->device.SetLoggingCallback(
        [](WGPULoggingType type, const char *msg, void *data) {
            std::cout << "WebGPU Log: " << msg << "\n" << std::flush;
        },
        nullptr);
        */

    app_state->queue = app_state->device.GetQueue();

    wgpu::SurfaceDescriptorFromCanvasHTMLSelector selector;
    selector.selector = "#webgpu-canvas";

    wgpu::SurfaceDescriptor surface_desc;
    surface_desc.nextInChain = &selector;

    app_state->surface = instance.CreateSurface(&surface_desc);

    wgpu::SwapChainDescriptor swap_chain_desc;
    swap_chain_desc.format = wgpu::TextureFormat::BGRA8Unorm;
    swap_chain_desc.usage = wgpu::TextureUsage::RenderAttachment;
    swap_chain_desc.presentMode = wgpu::PresentMode::Fifo;
    swap_chain_desc.width = win_width;
    swap_chain_desc.height = win_height;

    app_state->swap_chain =
        app_state->device.CreateSwapChain(app_state->surface, &swap_chain_desc);

    // Create the depth buffer
    {
        wgpu::TextureDescriptor depth_desc;
        depth_desc.format = wgpu::TextureFormat::Depth32Float;
        depth_desc.size.width = win_width;
        depth_desc.size.height = win_height;
        depth_desc.usage = wgpu::TextureUsage::RenderAttachment;

        app_state->depth_texture = app_state->device.CreateTexture(&depth_desc);
    }

    {
        wgpu::ShaderModuleWGSLDescriptor shader_module_wgsl;
        shader_module_wgsl.code = reinterpret_cast<const char *>(gltf_wgsl);

        wgpu::ShaderModuleDescriptor shader_module_desc;
        shader_module_desc.nextInChain = &shader_module_wgsl;
        app_state->shader_module = app_state->device.CreateShaderModule(&shader_module_desc);

        app_state->shader_module.GetCompilationInfo(
            [](WGPUCompilationInfoRequestStatus status,
               WGPUCompilationInfo const *info,
               void *) {
                if (info->messageCount != 0) {
                    std::cout << "Shader compilation info:\n";
                    for (uint32_t i = 0; i < info->messageCount; ++i) {
                        const auto &m = info->messages[i];
                        std::cout << m.lineNum << ":" << m.linePos << ": ";
                        switch (m.type) {
                        case WGPUCompilationMessageType_Error:
                            std::cout << "error";
                            break;
                        case WGPUCompilationMessageType_Warning:
                            std::cout << "warning";
                            break;
                        case WGPUCompilationMessageType_Info:
                            std::cout << "info";
                            break;
                        default:
                            break;
                        }

                        std::cout << ": " << m.message << "\n";
                    }
                }
            },
            nullptr);
    }

    // Create the view params bind group
    wgpu::BindGroupLayoutEntry view_param_layout_entry = {};
    view_param_layout_entry.binding = 0;
    view_param_layout_entry.buffer.hasDynamicOffset = false;
    view_param_layout_entry.buffer.type = wgpu::BufferBindingType::Uniform;
    view_param_layout_entry.visibility = wgpu::ShaderStage::Vertex;

    wgpu::BindGroupLayoutDescriptor view_params_bg_layout_desc = {};
    view_params_bg_layout_desc.entryCount = 1;
    view_params_bg_layout_desc.entries = &view_param_layout_entry;

    wgpu::BindGroupLayout view_params_bg_layout =
        app_state->device.CreateBindGroupLayout(&view_params_bg_layout_desc);

    // Import the embedded default GLTF file
    load_gltf_buffer(DamagedHelmet_glb, DamagedHelmet_glb_size);

    // Create the UBO for our bind group
    wgpu::BufferDescriptor ubo_buffer_desc;
    ubo_buffer_desc.mappedAtCreation = false;
    ubo_buffer_desc.size = 16 * sizeof(float);
    ubo_buffer_desc.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
    app_state->view_param_buf = app_state->device.CreateBuffer(&ubo_buffer_desc);

    wgpu::BindGroupEntry view_param_bg_entry = {};
    view_param_bg_entry.binding = 0;
    view_param_bg_entry.buffer = app_state->view_param_buf;
    view_param_bg_entry.size = ubo_buffer_desc.size;

    wgpu::BindGroupDescriptor bind_group_desc = {};
    bind_group_desc.layout = view_params_bg_layout;
    bind_group_desc.entryCount = 1;
    bind_group_desc.entries = &view_param_bg_entry;

    app_state->bind_group = app_state->device.CreateBindGroup(&bind_group_desc);

    app_state->proj = glm::perspective(
        glm::radians(50.f), static_cast<float>(win_width) / win_height, 0.01f, 1000.f);
    app_state->camera = ArcballCamera(glm::vec3(0, 0, 3.f), glm::vec3(0), glm::vec3(0, 1, 0));

    emscripten_set_mousemove_callback("#webgpu-canvas", app_state, true, mouse_move_callback);
    emscripten_set_wheel_callback("#webgpu-canvas", app_state, true, mouse_wheel_callback);

    emscripten_set_main_loop_arg(loop_iteration, app_state, -1, 0);

    return 0;
}

int mouse_move_callback(int type, const EmscriptenMouseEvent *event, void *user_data)
{
    const glm::vec2 cur_mouse = transform_mouse(glm::vec2(event->targetX, event->targetY));

    if (app_state->prev_mouse != glm::vec2(-2.f)) {
        if (event->buttons & 1) {
            app_state->camera.rotate(app_state->prev_mouse, cur_mouse);
            app_state->camera_changed = true;
        } else if (event->buttons & 2) {
            app_state->camera.pan(cur_mouse - app_state->prev_mouse);
            app_state->camera_changed = true;
        }
    }
    app_state->prev_mouse = cur_mouse;

    return true;
}

int mouse_wheel_callback(int type, const EmscriptenWheelEvent *event, void *user_data)
{
    // Pinch events on the touchpad the ctrl key set
    // TODO: this likely breaks scroll on a scroll wheel, so we need a way to detect if the
    // user has a mouse and change the behavior. Need to test on a real mouse
    if (true) {  // event->mouse.ctrlKey) {
        app_state->camera.zoom(-event->deltaY * 0.0025f * dpi);
        app_state->camera_changed = true;
    } else {
        glm::vec2 prev_mouse(css_w / 2.f, css_h / 2.f);

        const auto cur_mouse =
            transform_mouse(prev_mouse - glm::vec2(event->deltaX, event->deltaY));
        prev_mouse = transform_mouse(prev_mouse);

        app_state->camera.rotate(prev_mouse, cur_mouse);
        app_state->camera_changed = true;
    }

    return true;
}

void loop_iteration(void *user_data)
{
    if (app_state->new_gltf_model) {
        std::cout << "Uploading new GLB file to the GPU\n";

        app_state->gltf_model = std::move(app_state->new_gltf_model);
        // Upload all buffers that need to be uploaded
        for (auto &bv : app_state->gltf_model->buffers) {
            if (bv.needs_upload) {
                std::cout << "Uploading buffer of " << bv.byte_length() << " bytes\n";
                bv.upload(app_state->device);
                bv.needs_upload = false;
            }
        }

        // Create the view params bind group
        wgpu::BindGroupLayoutEntry view_param_layout_entry = {};
        view_param_layout_entry.binding = 0;
        view_param_layout_entry.buffer.hasDynamicOffset = false;
        view_param_layout_entry.buffer.type = wgpu::BufferBindingType::Uniform;
        view_param_layout_entry.visibility = wgpu::ShaderStage::Vertex;

        wgpu::BindGroupLayoutDescriptor view_params_bg_layout_desc = {};
        view_params_bg_layout_desc.entryCount = 1;
        view_params_bg_layout_desc.entries = &view_param_layout_entry;

        wgpu::BindGroupLayout view_params_bg_layout =
            app_state->device.CreateBindGroupLayout(&view_params_bg_layout_desc);

        std::vector<wgpu::ColorTargetState> color_targets;
        {
            wgpu::ColorTargetState cts;
            cts.format = wgpu::TextureFormat::BGRA8Unorm;
            color_targets.push_back(cts);
        }

        wgpu::DepthStencilState depth_state;
        depth_state.format = wgpu::TextureFormat::Depth32Float;
        depth_state.depthCompare = wgpu::CompareFunction::Less;
        depth_state.depthWriteEnabled = true;

        // Build render pipelines for each node
        for (auto &n : app_state->gltf_model->nodes) {
            std::cout << "Building render pipeline for: " << n.get_name() << "\n";
            n.build_render_pipeline(app_state->device,
                                    app_state->shader_module,
                                    color_targets,
                                    depth_state,
                                    {view_params_bg_layout});
        }
    }

    wgpu::Buffer upload_buf;
    if (app_state->camera_changed) {
        wgpu::BufferDescriptor upload_buffer_desc;
        upload_buffer_desc.mappedAtCreation = true;
        upload_buffer_desc.size = 16 * sizeof(float);
        upload_buffer_desc.usage = wgpu::BufferUsage::CopySrc;
        upload_buf = app_state->device.CreateBuffer(&upload_buffer_desc);

        const glm::mat4 proj_view = app_state->proj * app_state->camera.transform();

        std::memcpy(
            upload_buf.GetMappedRange(), glm::value_ptr(proj_view), 16 * sizeof(float));
        upload_buf.Unmap();
    }

    wgpu::RenderPassColorAttachment color_attachment;
    color_attachment.view = app_state->swap_chain.GetCurrentTextureView();
    color_attachment.clearValue.r = 0.1f;
    color_attachment.clearValue.g = 0.1f;
    color_attachment.clearValue.b = 0.1f;
    color_attachment.clearValue.a = 1.f;
    color_attachment.loadOp = wgpu::LoadOp::Clear;
    color_attachment.storeOp = wgpu::StoreOp::Store;

    wgpu::RenderPassDescriptor pass_desc;
    pass_desc.colorAttachmentCount = 1;
    pass_desc.colorAttachments = &color_attachment;

    wgpu::RenderPassDepthStencilAttachment depth_attachment;
    depth_attachment.view = app_state->depth_texture.CreateView();
    depth_attachment.depthClearValue = 1.f;
    depth_attachment.depthLoadOp = wgpu::LoadOp::Clear;
    depth_attachment.depthStoreOp = wgpu::StoreOp::Store;

    pass_desc.depthStencilAttachment = &depth_attachment;

    wgpu::CommandEncoder encoder = app_state->device.CreateCommandEncoder();
    if (app_state->camera_changed) {
        encoder.CopyBufferToBuffer(
            upload_buf, 0, app_state->view_param_buf, 0, 16 * sizeof(float));
    }

    wgpu::RenderPassEncoder render_pass_enc = encoder.BeginRenderPass(&pass_desc);

    render_pass_enc.SetBindGroup(0, app_state->bind_group);

    if (app_state->gltf_model) {
        // Render all nodes of the gltf model
        for (auto &n : app_state->gltf_model->nodes) {
            n.render(render_pass_enc);
        }
    }

    render_pass_enc.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    // Here the # refers to the number of command buffers being submitted
    app_state->queue.Submit(1, &commands);

    app_state->camera_changed = false;
}

extern "C" EMSCRIPTEN_KEEPALIVE void load_gltf_buffer(const uint8_t *glb,
                                                      const size_t glb_size)
{
    using namespace std::chrono;
    std::cout << "Importing new GLTF data!\n";
    std::cout << "Byte size is " << glb_size << "\n";

    auto start = steady_clock::now();
    app_state->new_gltf_model = import_gltf(glb, glb_size);
    auto end = steady_clock::now();

    std::cout << "C++ timed loading as: " << duration_cast<milliseconds>(end - start).count()
              << "ms\n";
}
