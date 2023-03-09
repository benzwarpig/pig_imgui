#pragma once

#include "data_comm.h"

namespace MoproboGui {

class HistogramFactory {
    HistogramFactory() = default;

public:
    static HistogramFactory& getInstance() {
        static HistogramFactory instance;
        return instance;
    }

    ~HistogramFactory() = default;

    void showHistogram() {
        static ImPlotHistogramFlags hist_flags = ImPlotHistogramFlags_Density;
        static int bins = 50;
        static double mu = 5;
        static double sigma = 2;
        ImGui::SetNextItemWidth(200);
        if (ImGui::RadioButton("Sqrt", bins == ImPlotBin_Sqrt)) {
            bins = ImPlotBin_Sqrt;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Sturges", bins == ImPlotBin_Sturges)) {
            bins = ImPlotBin_Sturges;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Rice", bins == ImPlotBin_Rice)) {
            bins = ImPlotBin_Rice;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Scott", bins == ImPlotBin_Scott)) {
            bins = ImPlotBin_Scott;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("N Bins", bins >= 0)) {
            bins = 50;
        }
        if (bins >= 0) {
            ImGui::SameLine();
            ImGui::SetNextItemWidth(200);
            ImGui::SliderInt("##Bins", &bins, 1, 100);
        }
        ImGui::CheckboxFlags("Horizontal", (unsigned int*)&hist_flags,
                             ImPlotHistogramFlags_Horizontal);
        ImGui::SameLine();
        ImGui::CheckboxFlags("Density", (unsigned int*)&hist_flags,
                             ImPlotHistogramFlags_Density);
        ImGui::SameLine();
        ImGui::CheckboxFlags("Cumulative", (unsigned int*)&hist_flags,
                             ImPlotHistogramFlags_Cumulative);

        static bool range = false;
        ImGui::Checkbox("Range", &range);
        static float rmin = -3;
        static float rmax = 13;
        if (range) {
            ImGui::SameLine();
            ImGui::SetNextItemWidth(200);
            ImGui::DragFloat2("##Range", &rmin, 0.1f, -3, 13);
            ImGui::SameLine();
            ImGui::CheckboxFlags("Exclude Outliers", (unsigned int*)&hist_flags,
                                 ImPlotHistogramFlags_NoOutliers);
        }
        static NormalDistribution<10000> dist(mu, sigma);
        static double x[100];
        static double y[100];
        if (hist_flags & ImPlotHistogramFlags_Density) {
            for (int i = 0; i < 100; ++i) {
                x[i] = -3 + 16 * (double)i / 99.0;
                y[i] = exp(-(x[i] - mu) * (x[i] - mu) / (2 * sigma * sigma)) /
                       (sigma * sqrt(2 * 3.141592653589793238));
            }
            if (hist_flags & ImPlotHistogramFlags_Cumulative) {
                for (int i = 1; i < 100; ++i) y[i] += y[i - 1];
                for (int i = 0; i < 100; ++i) y[i] /= y[99];
            }
        }

        if (ImPlot::BeginPlot("##Histograms")) {
            ImPlot::SetupAxes(NULL, NULL, ImPlotAxisFlags_AutoFit,
                              ImPlotAxisFlags_AutoFit);
            ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL, 0.5f);
            ImPlot::PlotHistogram(
                "Empirical", dist.Data, 10000, bins, 1.0,
                range ? ImPlotRange(rmin, rmax) : ImPlotRange(), hist_flags);
            if ((hist_flags & ImPlotHistogramFlags_Density) &&
                !(hist_flags & ImPlotHistogramFlags_NoOutliers)) {
                if (hist_flags & ImPlotHistogramFlags_Horizontal)
                    ImPlot::PlotLine("Theoretical", y, x, 100);
                else
                    ImPlot::PlotLine("Theoretical", x, y, 100);
            }
            ImPlot::EndPlot();
        }
    }

private:
};

};  // namespace MoproboGui