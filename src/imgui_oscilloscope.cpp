#include "imgui_oscilloscope.h"

#include "iostream"

namespace MoproboGui {

void OscilloscopeFactory::showMoproboWindow() {
    // 软件示波器
    ImGui::Begin("软件示波器");
    ImGui::Text("该窗口用于显示波形");

    for (const auto& scope : m_scopes) {
        scope.second->showOscilloscopeWindow();
    }
    ImGui::End();
}

std::shared_ptr<OscilloscopeWindow> OscilloscopeFactory::createScopes(
    const std::string& fold, const std::string& plot, ImPlotAxisFlags flag,
    float history, float max) {
    if (m_scopes.find(fold) != m_scopes.end()) {
        return m_scopes[fold];
    }
    auto ret =
        std::make_shared<OscilloscopeWindow>(fold, plot, flag, history, max);
    m_scopes.insert({fold, ret});
    return m_scopes[fold];
}

void OscilloscopeWindow::showOscilloscopeWindow() {
    if (ImGui::CollapsingHeader(m_foldName.c_str())) {
        static float history = m_history;
        ImGui::SliderFloat("History", &history, 1, m_maxTime, "%.1f s");
        if (ImPlot::BeginPlot(m_plotName.c_str(), ImVec2(-1, 150))) {
            ImPlot::SetupAxes(NULL, NULL, m_flag, m_flag);
            ImPlot::SetupAxisLimits(ImAxis_X1, 0, history, ImGuiCond_Always);
            ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 1);
            for (const auto& buffer : m_buffers) {
                const auto& id = buffer.first;
                const auto& plot = buffer.second;
                plot->reSpan(history);
                ImPlot::PlotLine(
                    plot->id().c_str(), &plot->getBuffer().Data[0].x,
                    &plot->getBuffer().Data[0].y, plot->getBuffer().Data.size(),
                    0, 0, 2 * sizeof(float));
            }
            ImPlot::EndPlot();
        }
    }
}

void OscilloscopeWindow::setScopeConfig(const std::string& fold,
                                        const std::string& plot,
                                        ImPlotAxisFlags flag, float history,
                                        float max) {
    m_foldName = fold;
    m_plotName = plot;
    m_flag = flag;
    m_history = history;
    m_maxTime = max;
}

std::shared_ptr<OscilloscopeBuffer> OscilloscopeWindow::createPlot(
    const std::string& plot) {
    if (m_buffers.find(plot) != m_buffers.end()) {
        return m_buffers[plot];
    }
    auto ret = std::make_shared<OscilloscopeBuffer>(plot);
    m_buffers.insert(std::make_pair(plot, ret));
    return ret;
}

std::string OscilloscopeBuffer::id() const { return m_plotId; }

RollingBuffer OscilloscopeBuffer::getBuffer() const { return m_buffer; }

bool OscilloscopeBuffer::addPoint(float number, float time) {
    m_time += time;
    m_buffer.AddPoint(m_time, number);
    return true;
}

bool OscilloscopeBuffer::reSpan(float span) {
    m_buffer.Span = span;
    return true;
}

bool OscilloscopeBuffer::clear() {
    m_buffer.Data.clear();
    return true;
}

};  // namespace MoproboGui
