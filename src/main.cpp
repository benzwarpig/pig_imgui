#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl2.h>
#include <imgui/imgui.h>

#include <future>
#include <iostream>

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif

#include <GLFW/glfw3.h>
#include <imgui/implot.h>
#include <imgui/implot_internal.h>

#include "imgui_oscilloscope.h"

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to
// maximize ease of testing and compatibility with old VS compilers. To link
// with VS2010-era libraries, VS2015+ requires linking with
// legacy_stdio_definitions.lib, which we do using this pragma. Your own project
// should not be affected, as you are likely to link with a newer binary of GLFW
// that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && \
    !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

static void glfw_error_callback(int error, const char *description) {
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

int main(int args, char **argv) {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) return 1;
    GLFWwindow *window =
        glfwCreateWindow(1280, 720, "墨派机器人调试工具", NULL, NULL);
    if (window == NULL) {
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);  // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();

    ImGuiIO &io = ImGui::GetIO();
    // 加入简体中文支持
    io.Fonts->AddFontFromFileTTF(
        "../fonts/simhei.ttf", 16.0f, NULL,
        io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
    (void)io;
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable
    // Keyboard Controls io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; //
    // Enable Gamepad Controls

    // Setup Dear ImGui style
    // ImGui::StyleColorsDark();
    ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL2_Init();

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    /******************************************************************/
    /********************* imgui plot 测试部分代码 **********************/
    /******************************************************************/

    auto scope = MoproboGui::OscilloscopeFactory::getInstance().createScopes(
        "Moprobo软件示波器", "实时波形绘制");

    auto plot1 = scope->createPlot("测试数据波形1");
    auto plot2 = scope->createPlot("测试数据波形2");

    std::future<void> scopeThread = std::async(std::launch::async, [&]() {
        int num = 0;
        double time = 0;
        while (true) {
            if (++num > 100) {
                num -= 100;
            }
            plot1->addPoint(num, 0.02);
            plot2->addPoint(100 - num, 0.02);
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
    });

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to
        // tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data
        // to your main application, or clear/overwrite your copy of the mouse
        // data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input
        // data to your main application, or clear/overwrite your copy of the
        // keyboard data. Generally you may always pass all inputs to dear
        // imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        MoproboGui::OscilloscopeFactory::getInstance().showMoproboWindow();

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w,
                     clear_color.y * clear_color.w,
                     clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);

        // If you are using this code with non-legacy OpenGL header/contexts
        // (which you should not, prefer using imgui_impl_opengl3.cpp!!), you
        // may need to backup/reset/restore other state, e.g. for current shader
        // using the commented lines below. GLint last_program;
        // glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
        // glUseProgram(0);
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
        // glUseProgram(last_program);

        glfwMakeContextCurrent(window);
        glfwSwapBuffers(window);
    }

    std::cout << "hello imgui" << std::endl;

    // Cleanup
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    if (scopeThread.valid()) {
        scopeThread.get();
    }

    return 0;
}
