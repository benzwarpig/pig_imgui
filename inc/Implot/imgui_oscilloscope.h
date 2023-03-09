/**
 * @file imgui_oscilloscope.h
 * @author benzs_war_pig (benzwarpig@outlook.com)
 * @brief
 * 基于implot,实现软件示波器功能，可以生成任意窗口，每个窗口可以添加任意波形曲线。
 *
 * 用法如下：
 *  auto scope = MoproboGui::OscilloscopeFactory::getInstance().createScopes(
 *      "Moprobo软件示波器", "实时波形绘制");
 *
 *  auto plot1 = scope->createPlot("测试数据波形1");
 *  auto plot2 = scope->createPlot("测试数据波形2");
 *
 *  std::future<void> scopeThread = std::async(std::launch::async, [&]() {
 *      int num = 0;
 *      double time = 0;
 *      while (true) {
 *          if (++num > 100) {
 *              num -= 100;
 *          }
 *          plot1->addPoint(num, 0.02);
 *          plot2->addPoint(100 - num, 0.02);
 *          std::this_thread::sleep_for(std::chrono::milliseconds(20));
 *      }
 *  });
 *
 * @version 1.0
 * @date 2023-03-10
 *
 * @copyright Copyright (c) 2023
 *
 */

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
