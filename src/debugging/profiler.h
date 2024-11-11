#include <chrono>
#include <string>
#include <unordered_map>
#include <vector>
#include <stdexcept>
#include <numeric>

#include "imgui/imgui.h"
#include "implot/implot.h"

#define MAX_FPS_HISTORY 10000

class Profiler {
public:
    Profiler() {
        m_startTime = std::chrono::high_resolution_clock::now();
    }

    void start(const std::string& label) {
        auto now = std::chrono::high_resolution_clock::now();
        m_times[label].start = now;
        m_times[label].active = true;
    }

    void end(const std::string& label) {
        auto now = std::chrono::high_resolution_clock::now();
        auto it = m_times.find(label);
        if (it == m_times.end() || !it->second.active) {
            throw std::runtime_error("[Error] Profiler end called without matching start for label: " + label);
        }
        it->second.duration = std::chrono::duration_cast<std::chrono::microseconds>(now - it->second.start).count();
        it->second.active = false;
    }

    void record(float fps) {
        // Record the current time in seconds since start
        auto now = std::chrono::high_resolution_clock::now();
        double time = std::chrono::duration<double>(now - m_startTime).count();
        m_fpsHistory.push_back({time, fps});

        // Remove old data to keep the history within MAX_FPS_HISTORY size
        if (m_fpsHistory.size() > MAX_FPS_HISTORY) {
            m_fpsHistory.erase(m_fpsHistory.begin());
        }
    }

    void display() {
        // Position the window in the top-right corner
        ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 320, 20), ImGuiCond_Always);
        ImGui::Begin("Profiler", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

        // Display profiling data
        for (const auto& [label, record] : m_times) {
            if (record.active) {
                ImGui::Text("%s: running...", label.c_str());
            } else {
                ImGui::Text("%s: %lld µs", label.c_str(), record.duration);
            }
        }

        if (!m_fpsHistory.empty()) {
            ImGui::Text("FPS: %.1f", m_fpsHistory.back().fps);
        }

        ImGui::End();
        // displayFPSgraph();
    }

private:
    struct TimeRecord {
        std::chrono::time_point<std::chrono::high_resolution_clock> start;
        long long duration = 0;
        bool active = false;
    };

    std::unordered_map<std::string, TimeRecord> m_times;

    struct FPSDataPoint {
        double time;
        float fps;
    };

    std::vector<FPSDataPoint> m_fpsHistory;

    std::chrono::time_point<std::chrono::high_resolution_clock> m_startTime;

    void displayFPSgraph() {
        // Create a new window for the FPS graph
        ImGui::Begin("FPS Graph");

        size_t totalFrames = m_fpsHistory.size();

        if (ImPlot::BeginPlot("FPS Over Time", "Time (s)", "FPS")) {
            // Get current time
            double currentTime = m_fpsHistory.back().time;
            // Define time window (e.g., 10 seconds)
            const double timeWindow = 10.0; // seconds

            // Find the starting index where time >= currentTime - timeWindow
            auto it = std::lower_bound(
                m_fpsHistory.begin(), m_fpsHistory.end(), currentTime - timeWindow,
                [](const FPSDataPoint& dataPoint, double time) {
                    return dataPoint.time < time;
                }
            );

            // Get the index
            size_t startIndex = std::distance(m_fpsHistory.begin(), it);

            // Prepare data arrays
            size_t dataSize = totalFrames - startIndex;
            std::vector<double> times(dataSize);
            std::vector<double> fpsValues(dataSize);

            // Initialize min and max fps for y-axis limits
            double minFPS = std::numeric_limits<double>::max();
            double maxFPS = std::numeric_limits<double>::lowest();

            for (size_t i = startIndex; i < totalFrames; ++i) {
                times[i - startIndex] = m_fpsHistory[i].time;
                fpsValues[i - startIndex] = m_fpsHistory[i].fps;

                // Update min and max FPS for y-axis limits
                minFPS = std::min(minFPS, fpsValues[i - startIndex]);
                maxFPS = std::max(maxFPS, fpsValues[i - startIndex]);
            }

            // Ensure minFPS is non-negative
            if (minFPS < 0.0f) {
                minFPS = 0.0f;
            }

            // Ensure x-axis starts from zero
            double xMin = std::max(currentTime - timeWindow, 0.0);
            double xMax = currentTime;

            // Set the axis limits to be non-negative ranges
            ImPlot::SetupAxisLimits(ImAxis_X1, xMin, xMax, ImGuiCond_Always);
            ImPlot::SetupAxisLimits(ImAxis_Y1, minFPS, maxFPS * 1.1f, ImGuiCond_Always);

            // Plot the data
            ImPlot::PushStyleVar(ImPlotStyleVar_LineWeight, 0.01f);
            ImPlot::PlotLine("FPS", times.data(), fpsValues.data(), static_cast<int>(dataSize));
            ImPlot::PopStyleVar();

            ImPlot::EndPlot();
        }

        ImGui::End();
    }
};
