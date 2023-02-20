#include "imgui_oscilloscope.h"

namespace MoproboGui {

void OscilloscopeFactory::ShowMoproboWindow() {
    // 软件示波器
    ImGui::Begin("软件示波器");
    ImGui::Text("该窗口用于显示波形");
}

void ShowMoproboWindow() {
    // 软件示波器
    ImGui::Begin("软件示波器");
    ImGui::Text("该窗口用于显示波形");

    ImGui::BulletText("Move your mouse to change the data!");
    ImGui::BulletText(
        "This example assumes 60 FPS. Higher FPS requires larger buffer size.");
    static RollingBuffer rdata1, rdata2;
    ImVec2 mouse = ImGui::GetMousePos();
    static float t = 0;
    t += ImGui::GetIO().DeltaTime;
    rdata1.AddPoint(t, mouse.x * 0.0005f);
    rdata2.AddPoint(t, mouse.y * 0.0005f);

    static ImPlotAxisFlags flags = ImPlotAxisFlags_None;

    if (ImGui::CollapsingHeader("mouse test")) {
        static float history = 10.0f;
        ImGui::SliderFloat("History", &history, 1, 60, "%.1f s");
        rdata1.Span = history;
        rdata2.Span = history;

        if (ImPlot::BeginPlot("##Rolling", ImVec2(-1, 150))) {
            ImPlot::SetupAxes(NULL, NULL, flags, flags);
            ImPlot::SetupAxisLimits(ImAxis_X1, 0, history, ImGuiCond_Always);
            ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 1);
            ImPlot::PlotLine("Mouse X", &rdata1.Data[0].x, &rdata1.Data[0].y,
                             rdata1.Data.size(), 0, 0, 2 * sizeof(float));
            ImPlot::PlotLine("Mouse Y", &rdata2.Data[0].x, &rdata2.Data[0].y,
                             rdata2.Data.size(), 0, 0, 2 * sizeof(float));
            ImPlot::EndPlot();
        }
    }

    ImGui::End();
}

};  // namespace MoproboGui
