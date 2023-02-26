#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "data_comm.h"

#ifndef MOPROBO_API
#define MOPROBO_API extern
#endif

namespace MoproboGui {

class OscilloscopeWindow;
class OscilloscopeBuffer;

class OscilloscopeFactory {
    OscilloscopeFactory() {}

public:
    static OscilloscopeFactory& getInstance() {
        static OscilloscopeFactory instance;
        return instance;
    }
    ~OscilloscopeFactory() = default;

    void showMoproboWindow();

    std::shared_ptr<OscilloscopeWindow> createScopes(
        const std::string& fold, const std::string& plot,
        ImPlotAxisFlags flag = ImPlotAxisFlags_None, float history = 10.f,
        float max = 60.f);

private:
    std::unordered_map<std::string, std::shared_ptr<OscilloscopeWindow>>
        m_scopes;
};

class OscilloscopeWindow {
public:
    OscilloscopeWindow() = delete;
    explicit OscilloscopeWindow(const std::string& fold,
                                const std::string& plot, ImPlotAxisFlags flag,
                                float history, float max)
        : m_foldName(fold),
          m_plotName(plot),
          m_flag(flag),
          m_history(history),
          m_maxTime(max) {}
    ~OscilloscopeWindow() = default;

    void setScopeConfig(const std::string& fold, const std::string& plot,
                        ImPlotAxisFlags flag, float history, float max);

    std::shared_ptr<OscilloscopeBuffer> createPlot(const std::string& plot);

    void showOscilloscopeWindow();

private:
    std::string m_foldName{""};
    std::string m_plotName{""};
    ImPlotAxisFlags m_flag;
    float m_history{0};
    float m_maxTime{0};

    std::unordered_map<std::string, std::shared_ptr<OscilloscopeBuffer>>
        m_buffers;
};

class OscilloscopeBuffer {
public:
    OscilloscopeBuffer() = default;
    explicit OscilloscopeBuffer(const std::string& id) : m_plotId(id) {}
    ~OscilloscopeBuffer() = default;

    std::string id() const;

    RollingBuffer getBuffer() const;

    bool addPoint(float number, float time);

    bool reSpan(float span);

    bool clear();

private:
    std::string m_plotId{""};

    double m_time{0};

    RollingBuffer m_buffer;
};

// void createPlotLine(const std::string& line_id, float* x, float* y) {}

};  // namespace MoproboGui
