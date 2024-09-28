#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <memory>
#include <iostream>
#include <stdexcept>

struct RenderPass {
    std::string name;

    // Resource names (dependencies)
    std::vector<std::string> inputs;
    std::vector<std::string> outputs;

    // Lambda functions

    // Resource(s) allocation
    std::function<void()> setup;

    // Run a render pass, passing the required resources
    std::function<void(const std::unordered_map<std::string, std::shared_ptr<void>>&)> execute;
};

class RenderGraph {
public:
    // Add a render pass to the graph
    void addPass(RenderPass pass) {
        m_passes.push_back(std::move(pass));
    }

    // Setup all passes
    void setup() {
        for (auto& pass : m_passes) {
            if (pass.setup) {
                std::cerr << "[Info] RenderGraph::setup: Setting up pass: " + pass.name + "\n";
                pass.setup();
            }
        }
    }

    // Execute all passes
    void execute() {
        std::unordered_set<std::string> executedPasses;

        for (auto& pass : m_passes) {
            executePass(pass, executedPasses);
        }
    }

    /*
    * Resource management
    */
    // Store a resource using a smart pointer
    template<typename T>
    void setResource(const std::string& name, std::shared_ptr<T> resource) {
        m_resources[name] = std::static_pointer_cast<void>(resource);
    }

    // Retrieve a resource by name
    template<typename T>
    std::shared_ptr<T> getResource(const std::string& name) const {
        auto it = m_resources.find(name);
        if (it != m_resources.end()) {
            return std::static_pointer_cast<T>(it->second);
        }
        return nullptr;
    }

    // Cleanup all stored resources
    void cleanup() {
        // Cleanup individual resources
        for (auto& resourcePair : m_resources) {
            std::cout << "[Info] Cleaning up resource: " << resourcePair.first << "\n";
            resourcePair.second.reset();
        }
        // Cleanup resource map
        m_resources.clear();
    }

private:
    void executePass(RenderPass& pass, std::unordered_set<std::string>& executedPasses) {
        // Ensure that all dependencies (inputs) are satisfied
        for (const auto& input : pass.inputs) {
            if (m_resources.find(input) == m_resources.end()) {
                throw std::runtime_error("[Error] RenderGraph::execute Missing resource: " +
                    input + " requested by pass: " + pass.name);
            }
        }

        // If this pass hasn't been executed yet
        if (executedPasses.find(pass.name) == executedPasses.end()) {
            std::cout << "[Info] Executing pass: " << pass.name << "\n";

            // Execute the pass, passing the available resources
            if (pass.execute) {
                pass.execute(m_resources);
            }

            // Mark this pass as executed
            executedPasses.insert(pass.name);

            // Store outputs produced by the pass (if any)
            for (const auto& output : pass.outputs) {
                // Example logic to handle outputs
                // Here, pass.createOutput() should handle output resource creation
                // m_resources[output] = pass.createOutput();
            }
        }
    }

    std::vector<RenderPass> m_passes;
    std::unordered_map<std::string, std::shared_ptr<void>> m_resources;
};
