#include <chrono>
#include <string>
#include <unordered_map>
#include <vector>
#include <stdexcept>
#include <numeric>
#include <algorithm>

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

        // Update history for this label
        it->second.history.push_back(static_cast<double>(it->second.duration) / 1000.0); // Convert to ms
        if (it->second.history.size() > 60) { // Keep last 60 samples
            it->second.history.erase(it->second.history.begin());
        }
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
        // Position the window in the top-left corner
        ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);

        ImGui::Begin("Performance Profiler", nullptr,
                    ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                    ImGuiWindowFlags_AlwaysAutoResize);

        // Header section with overall stats
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.8f, 0.2f, 1.0f));
        ImGui::Text("PERFORMANCE MONITOR");
        ImGui::PopStyleColor();
        ImGui::Separator();

        // FPS Display with color coding
        if (!m_fpsHistory.empty()) {
            float currentFPS = m_fpsHistory.back().fps;
            ImVec4 fpsColor = getFPSColor(currentFPS);

            ImGui::PushStyleColor(ImGuiCol_Text, fpsColor);
            ImGui::Text("FPS: %.1f", currentFPS);
            ImGui::PopStyleColor();

            // FPS status indicator
            ImGui::SameLine();
            const char* fpsStatus = getFPSStatus(currentFPS);
            ImGui::TextColored(fpsColor, "(%s)", fpsStatus);
        }

        ImGui::Spacing();

        // Timing section
        if (!m_times.empty()) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.8f, 0.2f, 1.0f));
            ImGui::Text("TIMING DATA");
            ImGui::PopStyleColor();
            ImGui::Separator();

            // Table for better organization
            if (ImGui::BeginTable("ProfilerTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 120.0f);
                ImGui::TableSetupColumn("Current", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                ImGui::TableSetupColumn("Avg/Graph", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableHeadersRow();

                for (const auto& [label, record] : m_times) {
                    ImGui::TableNextRow();

                    // Label column
                    ImGui::TableNextColumn();
                    ImGui::Text("%s", label.c_str());

                    // Current time column
                    ImGui::TableNextColumn();
                    if (record.active) {
                        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Running...");
                    } else {
                        double currentMs = static_cast<double>(record.duration) / 1000.0;
                        ImVec4 timeColor = getTimeColor(currentMs);
                        ImGui::TextColored(timeColor, "%.2f ms", currentMs);
                    }

                    // Average and mini-graph column
                    ImGui::TableNextColumn();
                    if (!record.history.empty()) {
                        double avgTime = std::accumulate(record.history.begin(), record.history.end(), 0.0) / record.history.size();
                        ImGui::Text("Avg: %.2f ms", avgTime);

                        // Mini sparkline graph
                        if (record.history.size() > 1) {
                            ImGui::SameLine();
                            drawMiniGraph(record.history, ImVec2(60, 20));
                        }
                    }
                }
                ImGui::EndTable();
            }
        }

        ImGui::Spacing();

        // Button to toggle FPS graph
        if (ImGui::Button(m_showFPSGraph ? "Hide FPS Graph" : "Show FPS Graph")) {
            m_showFPSGraph = !m_showFPSGraph;
        }

        ImGui::End();

        // Display FPS graph if enabled
        if (m_showFPSGraph) {
            displayFPSGraph();
        }
    }

private:
    struct TimeRecord {
        std::chrono::time_point<std::chrono::high_resolution_clock> start;
        long long duration = 0;
        bool active = false;
        std::vector<double> history; // History of timing measurements
    };

    std::unordered_map<std::string, TimeRecord> m_times;

    struct FPSDataPoint {
        double time;
        float fps;
    };

    std::vector<FPSDataPoint> m_fpsHistory;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_startTime;
    bool m_showFPSGraph = false;

    ImVec4 getFPSColor(float fps) {
        if (fps >= 60.0f) return ImVec4(0.2f, 0.8f, 0.2f, 1.0f);      // Green
        else if (fps >= 30.0f) return ImVec4(0.8f, 0.8f, 0.2f, 1.0f); // Yellow
        else return ImVec4(0.8f, 0.2f, 0.2f, 1.0f);                   // Red
    }

    const char* getFPSStatus(float fps) {
        if (fps >= 60.0f) return "Excellent";
        else if (fps >= 45.0f) return "Good";
        else if (fps >= 30.0f) return "Fair";
        else return "Poor";
    }

    ImVec4 getTimeColor(double timeMs) {
        if (timeMs <= 1.0) return ImVec4(0.2f, 0.8f, 0.2f, 1.0f);      // Green - very fast
        else if (timeMs <= 5.0) return ImVec4(0.8f, 0.8f, 0.2f, 1.0f); // Yellow - acceptable
        else if (timeMs <= 16.0) return ImVec4(1.0f, 0.5f, 0.0f, 1.0f); // Orange - slow
        else return ImVec4(0.8f, 0.2f, 0.2f, 1.0f);                    // Red - very slow
    }

    void drawMiniGraph(const std::vector<double>& data, ImVec2 size) {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 canvasPos = ImGui::GetCursorScreenPos();
        ImVec2 canvasSize = size;

        // Background
        drawList->AddRectFilled(canvasPos,
                               ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y),
                               IM_COL32(50, 50, 50, 100));

        if (data.size() < 2) {
            ImGui::Dummy(canvasSize);
            return;
        }

        // Find min/max for scaling
        auto [minIt, maxIt] = std::minmax_element(data.begin(), data.end());
        double minVal = *minIt;
        double maxVal = *maxIt;
        double range = maxVal - minVal;

        if (range < 0.001) range = 0.001; // Avoid division by zero

        // Draw the line graph
        for (size_t i = 1; i < data.size(); ++i) {
            float x1 = canvasPos.x + (static_cast<float>(i - 1) / static_cast<float>(data.size() - 1)) * canvasSize.x;
            float y1 = canvasPos.y + canvasSize.y - static_cast<float>((data[i - 1] - minVal) / range) * canvasSize.y;
            float x2 = canvasPos.x + (static_cast<float>(i) / static_cast<float>(data.size() - 1)) * canvasSize.x;
            float y2 = canvasPos.y + canvasSize.y - static_cast<float>((data[i] - minVal) / range) * canvasSize.y;

            drawList->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), IM_COL32(100, 200, 255, 255), 1.0f);
        }

        ImGui::Dummy(canvasSize);
    }

    void displayFPSGraph() {
        // Position FPS graph window next to the profiler
        ImGui::SetNextWindowPos(ImVec2(440, 20), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(500, 300), ImGuiCond_FirstUseEver);

        ImGui::Begin("FPS Graph", &m_showFPSGraph, ImGuiWindowFlags_NoMove);

        if (m_fpsHistory.empty()) {
            ImGui::Text("No FPS data available");
            ImGui::End();
            return;
        }

        size_t totalFrames = m_fpsHistory.size();

        if (ImPlot::BeginPlot("FPS Over Time", ImVec2(-1, 250))) {
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

            // Ensure x-axis starts from reasonable time
            double xMin = std::max(currentTime - timeWindow, 0.0);
            double xMax = currentTime;

            // Set the axis limits
            ImPlot::SetupAxisLimits(ImAxis_X1, xMin, xMax, ImGuiCond_Always);
            ImPlot::SetupAxisLimits(ImAxis_Y1, minFPS * 0.9, maxFPS * 1.1, ImGuiCond_Always);
            ImPlot::SetupAxis(ImAxis_X1, "Time (s)");
            ImPlot::SetupAxis(ImAxis_Y1, "FPS");

            // Plot the data with styling
            ImPlot::PushStyleVar(ImPlotStyleVar_LineWeight, 2.0f);
            ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(0.2f, 0.8f, 0.2f, 1.0f));
            ImPlot::PlotLine("FPS", times.data(), fpsValues.data(), static_cast<int>(dataSize));
            ImPlot::PopStyleColor();
            ImPlot::PopStyleVar();

            // Add reference lines for common FPS targets
            ImPlot::PushStyleVar(ImPlotStyleVar_LineWeight, 1.0f);
            ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(0.5f, 0.5f, 0.5f, 0.5f));

            double refFPS[] = {30.0, 60.0, 120.0};
            for (double fps : refFPS) {
                if (fps >= minFPS && fps <= maxFPS) {
                    double refTimes[] = {xMin, xMax};
                    double refValues[] = {fps, fps};
                    ImPlot::PlotLine((std::to_string(static_cast<int>(fps)) + " FPS").c_str(),
                                   refTimes, refValues, 2);
                }
            }

            ImPlot::PopStyleColor();
            ImPlot::PopStyleVar();

            ImPlot::EndPlot();
        }

        // Statistics section
        ImGui::Separator();
        if (!m_fpsHistory.empty()) {
            float currentFPS = m_fpsHistory.back().fps;

            // Calculate statistics from recent data
            size_t recentSamples = std::min(size_t(300), m_fpsHistory.size()); // Last 5 seconds at 60fps
            auto recentStart = m_fpsHistory.end() - static_cast<ptrdiff_t>(recentSamples);

            float minRecentFPS = std::min_element(recentStart, m_fpsHistory.end(),
                [](const FPSDataPoint& a, const FPSDataPoint& b) { return a.fps < b.fps; })->fps;
            float maxRecentFPS = std::max_element(recentStart, m_fpsHistory.end(),
                [](const FPSDataPoint& a, const FPSDataPoint& b) { return a.fps < b.fps; })->fps;

            float avgRecentFPS = std::accumulate(recentStart, m_fpsHistory.end(), 0.0f,
                [](float sum, const FPSDataPoint& p) { return sum + p.fps; }) / static_cast<float>(recentSamples);

            ImGui::Text("Recent Stats (last %.1fs):", static_cast<float>(recentSamples) / 60.0f);
            ImGui::Text("  Current: %.1f FPS", currentFPS);
            ImGui::Text("  Average: %.1f FPS", avgRecentFPS);
            ImGui::Text("  Min: %.1f FPS", minRecentFPS);
            ImGui::Text("  Max: %.1f FPS", maxRecentFPS);
        }

        ImGui::End();
    }
};
