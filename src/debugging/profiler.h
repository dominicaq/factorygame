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
        // ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiCond_Always);
        ImGui::Begin("Profiler", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

        // Display profiling data
        for (const auto& [label, record] : m_times) {
            if (record.active) {
                ImGui::Text("%s: running...", label.c_str());
            } else {
                ImGui::Text("%s: %lld µs", label.c_str(), record.duration);
            }
        }

        ImGui::Text("FPS: %.1f", m_fpsHistory.back().fps);

        ImGui::End();
    }

private:
    struct TimeRecord {
        std::chrono::time_point<std::chrono::high_resolution_clock> start;
        long long duration = 0;
        bool active = false;
    };

    std::unordered_map<std::string, TimeRecord> m_times;

    struct FPSDataPoint {
        double time;  // Time since start in seconds
        float fps;
    };

    std::vector<FPSDataPoint> m_fpsHistory;

    std::chrono::time_point<std::chrono::high_resolution_clock> m_startTime;
};
