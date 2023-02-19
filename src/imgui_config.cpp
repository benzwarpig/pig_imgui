#include "imgui_config.h"

#include <imgui/imgui.h>
#include <imgui/implot.h>
#include <imgui/implot_internal.h>

namespace MoproboGui
{
    // utility structure for realtime plot
    struct ScrollingBuffer
    {
        int MaxSize;
        int Offset;
        ImVector<ImVec2> Data;
        ScrollingBuffer(int max_size = 2000)
        {
            MaxSize = max_size;
            Offset = 0;
            Data.reserve(MaxSize);
        }
        void AddPoint(float x, float y)
        {
            if (Data.size() < MaxSize)
                Data.push_back(ImVec2(x, y));
            else
            {
                Data[Offset] = ImVec2(x, y);
                Offset = (Offset + 1) % MaxSize;
            }
        }
        void Erase()
        {
            if (Data.size() > 0)
            {
                Data.shrink(0);
                Offset = 0;
            }
        }
    };

    // utility structure for realtime plot
    struct RollingBuffer
    {
        float Span;
        ImVector<ImVec2> Data;
        RollingBuffer()
        {
            Span = 10.0f;
            Data.reserve(2000);
        }
        void AddPoint(float x, float y)
        {
            float xmod = fmodf(x, Span);
            if (!Data.empty() && xmod < Data.back().x)
                Data.shrink(0);
            Data.push_back(ImVec2(xmod, y));
        }
    };

    void ShowMoproboWindow()
    {
        // 软件示波器
        ImGui::Begin("软件示波器");
        ImGui::Text("该窗口用于显示波形");

        ImGui::BulletText("Move your mouse to change the data!");
        ImGui::BulletText("This example assumes 60 FPS. Higher FPS requires larger buffer size.");
        static RollingBuffer rdata1, rdata2;
        ImVec2 mouse = ImGui::GetMousePos();
        static float t = 0;
        t += ImGui::GetIO().DeltaTime;
        rdata1.AddPoint(t, mouse.x * 0.0005f);
        rdata2.AddPoint(t, mouse.y * 0.0005f);

        static ImPlotAxisFlags flags = ImPlotAxisFlags_None;

        if (ImGui::CollapsingHeader("mouse test"))
        {
            static float history = 10.0f;
            ImGui::SliderFloat("History", &history, 1, 30, "%.1f s");
            rdata1.Span = history;
            rdata2.Span = history;

            if (ImPlot::BeginPlot("##Rolling", ImVec2(-1, 150)))
            {
                ImPlot::SetupAxes(NULL, NULL, flags, flags);
                ImPlot::SetupAxisLimits(ImAxis_X1, 0, history, ImGuiCond_Always);
                ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 1);
                ImPlot::PlotLine("Mouse X", &rdata1.Data[0].x, &rdata1.Data[0].y, rdata1.Data.size(), 0, 0, 2 * sizeof(float));
                ImPlot::PlotLine("Mouse Y", &rdata2.Data[0].x, &rdata2.Data[0].y, rdata2.Data.size(), 0, 0, 2 * sizeof(float));
                ImPlot::EndPlot();
            }
        }

        ImGui::End();
    }

}; // namespace MoproboGui
