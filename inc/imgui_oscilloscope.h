#pragma once

#include <imgui/imgui.h>
#include <imgui/implot.h>
#include <imgui/implot_internal.h>

#include <memory>
#include <string>
#include <vector>

#ifndef MOPROBO_API
#define MOPROBO_API extern
#endif

namespace MoproboGui {
// utility structure for realtime plot
struct ScrollingBuffer {
    int MaxSize;
    int Offset;
    ImVector<ImVec2> Data;
    ScrollingBuffer(int max_size = 2000) {
        MaxSize = max_size;
        Offset = 0;
        Data.reserve(MaxSize);
    }
    void AddPoint(float x, float y) {
        if (Data.size() < MaxSize)
            Data.push_back(ImVec2(x, y));
        else {
            Data[Offset] = ImVec2(x, y);
            Offset = (Offset + 1) % MaxSize;
        }
    }
    void Erase() {
        if (Data.size() > 0) {
            Data.shrink(0);
            Offset = 0;
        }
    }
};

// utility structure for realtime plot
struct RollingBuffer {
    float Span;
    ImVector<ImVec2> Data;
    RollingBuffer() {
        Span = 10.0f;
        Data.reserve(20000);
    }
    void AddPoint(float x, float y) {
        float xmod = fmodf(x, Span);
        if (!Data.empty() && xmod < Data.back().x) Data.shrink(0);
        Data.push_back(ImVec2(xmod, y));
    }
};

class OscilloscopeBase;
class OscilloscopeFactory {
    OscilloscopeFactory() {}

public:
    static OscilloscopeFactory& getInstance() {
        static OscilloscopeFactory instance;
        return instance;
    }
    ~OscilloscopeFactory() {}

    void ShowMoproboWindow();

private:
    std::vector<std::unique_ptr<OscilloscopeBase>> m_scopes;
};

class OscilloscopeBase {
public:
    OscilloscopeBase() {}
    virtual ~OscilloscopeBase() = 0;

    virtual void setScopeConfig(const std::string& fold,
                                const std::string& plot, ImPlotAxisFlags flag,
                                float history, float max) = 0;

private:
    std::string m_foldName;
    std::string m_plotName;
    ImPlotAxisFlags m_flag;
    float m_history{0};
    float m_maxTime{0};
};

class MoproboOscilloscope : public OscilloscopeBase {
    MoproboOscilloscope(const MoproboOscilloscope&) = delete;
    MoproboOscilloscope& operator=(const MoproboOscilloscope&) = delete;

public:
    MoproboOscilloscope() {}
    ~MoproboOscilloscope() {}

    void setPlotLine(const RollingBuffer& buffer) {}

private:
    std::vector<RollingBuffer> m_buffers;
};

};  // namespace MoproboGui
