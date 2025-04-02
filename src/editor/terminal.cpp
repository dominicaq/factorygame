#include "terminal.h"
#include <algorithm>
#include <regex>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#endif

// Initialize static instance
Terminal* Terminal::s_instance = nullptr;

/*
* TerminalBuf implementation
*/
Terminal::TerminalBuf::TerminalBuf(Terminal& terminal, MessageType type)
    : m_terminal(terminal), m_type(type) {}

Terminal::TerminalBuf::~TerminalBuf() {}

int Terminal::TerminalBuf::overflow(int c) {
    if (c != EOF) {
        if (c == '\n') {
            // Process completed line
            m_terminal.processLine(m_buffer, m_type);
            m_buffer.clear();
        } else {
            // Accumulate characters
            m_buffer += static_cast<char>(c);
        }
    }
    return c;
}

int Terminal::TerminalBuf::sync() {
    // Process any remaining text
    if (!m_buffer.empty()) {
        m_terminal.processLine(m_buffer, m_type);
        m_buffer.clear();
    }
    return 0;
}

/*
* Terminal implementation
*/
Terminal::Terminal()
    : m_terminalFocus(false),
      m_maxMessages(1000),
      m_originalCoutBuf(nullptr),
      m_originalCerrBuf(nullptr)
{
    memset(m_terminalInput, 0, sizeof(m_terminalInput));

    // Create stream buffers
    m_coutBuf = std::make_unique<TerminalBuf>(*this, MessageType::STANDARD);
    m_cerrBuf = std::make_unique<TerminalBuf>(*this, MessageType::ERR);

    // Add welcome message
    addMessage("Welcome to the engine terminal.", MessageType::INFO);
}

Terminal::~Terminal() {
    // Restore original streams
    restoreStdCapture();
}

Terminal& Terminal::getInstance() {
    if (s_instance == nullptr) {
        s_instance = new Terminal();
    }
    return *s_instance;
}

void Terminal::registerStdCapture() {
    // Save original buffers
    m_originalCoutBuf = std::cout.rdbuf();
    m_originalCerrBuf = std::cerr.rdbuf();

    // Redirect to our buffers
    std::cout.rdbuf(m_coutBuf.get());
    std::cerr.rdbuf(m_cerrBuf.get());
}

void Terminal::restoreStdCapture() {
    // Restore only if we have originals saved
    if (m_originalCoutBuf) {
        std::cout.rdbuf(m_originalCoutBuf);
        m_originalCoutBuf = nullptr;
    }

    if (m_originalCerrBuf) {
        std::cerr.rdbuf(m_originalCerrBuf);
        m_originalCerrBuf = nullptr;
    }
}

void Terminal::processLine(const std::string& line, MessageType defaultType) {
    // Remove carriage returns
    std::string cleanLine = line;
    cleanLine.erase(std::remove(cleanLine.begin(), cleanLine.end(), '\r'), cleanLine.end());

    if (cleanLine.empty()) {
        return;
    }

    // Determine message type based on content
    MessageType type = defaultType;

    // Check for prefixes to determine type
    if (defaultType != MessageType::ERR && cleanLine.find("[Error]") != std::string::npos) {
        type = MessageType::ERR;
    }
    else if (cleanLine.find("[Warning]") != std::string::npos) {
        type = MessageType::WARNING;
    }
    else if (cleanLine.find("[Info]") != std::string::npos) {
        type = MessageType::INFO;
    }
    else if (cleanLine.find("[Debug]") != std::string::npos) {
        type = MessageType::DEBUG;
    }

    // Add message to terminal
    addMessage(cleanLine, type);
}

void Terminal::addMessage(const std::string& message, MessageType type) {
    std::lock_guard<std::mutex> lock(m_messagesMutex);
    m_messages.emplace_back(message, type);

    // Limit message count
    if (m_messages.size() > m_maxMessages) {
        m_messages.erase(m_messages.begin());
    }
}

void Terminal::clear() {
    std::lock_guard<std::mutex> lock(m_messagesMutex);
    m_messages.clear();
    addMessage("Terminal cleared.", MessageType::INFO);
}

ImVec4 Terminal::getColor(MessageType type) const {
    switch (type) {
        case MessageType::ERR:
            return ImVec4(1.0f, 0.3f, 0.3f, 1.0f); // Red
        case MessageType::WARNING:
            return ImVec4(1.0f, 0.9f, 0.2f, 1.0f); // Yellow
        case MessageType::INFO:
            return ImVec4(0.5f, 0.8f, 1.0f, 1.0f); // Light Blue
        case MessageType::DEBUG:
            return ImVec4(0.7f, 0.7f, 0.7f, 1.0f); // Gray
        case MessageType::STANDARD:
        default:
            return ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // White
    }
}

/*
* Commands
*/
void Terminal::executeCommand(const std::string& command) {
    if (command.empty()) {
        return;
    }

    addMessage("> " + command, MessageType::STANDARD);
    handleCommand(command);
}

void Terminal::handleCommand(const std::string& command) {
    // Simple command system
    if (command == "clear") {
        clear();
    }
    else if (command == "help") {
        addMessage("Available commands:", MessageType::INFO);
        addMessage("  clear - Clear the terminal", MessageType::INFO);
        addMessage("  help - Show this help message", MessageType::INFO);
    }
    else {
        // Execute command - for demonstration just echo it back
        std::cout << "Command executed: " << command << std::endl;
    }
}

/*
* ImGUI Draw call
*/
void Terminal::drawTerminal() {
    ImGui::Text("Terminal");
    ImGui::Separator();

    // Terminal output area (read-only)
    ImGui::BeginChild("TerminalOutput", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), true, ImGuiWindowFlags_HorizontalScrollbar);

    // Display messages with appropriate colors
    {
        std::lock_guard<std::mutex> lock(m_messagesMutex);
        for (const auto& msg : m_messages) {
            ImGui::PushStyleColor(ImGuiCol_Text, getColor(msg.type));
            ImGui::TextWrapped("%s", msg.text.c_str());
            ImGui::PopStyleColor();
        }
    }

    // Auto-scroll to bottom when at bottom
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 20) {
        ImGui::SetScrollHereY(1.0f);
    }

    ImGui::EndChild();

    // Terminal input
    ImGui::PushItemWidth(-1);
    if (!m_terminalFocus && ImGui::IsWindowFocused()) {
        ImGui::SetKeyboardFocusHere();
    }

    if (ImGui::InputText("##TerminalInput", m_terminalInput, IM_ARRAYSIZE(m_terminalInput),
                        ImGuiInputTextFlags_EnterReturnsTrue)) {
        // Process command
        std::string command = m_terminalInput;
        executeCommand(command);

        memset(m_terminalInput, 0, sizeof(m_terminalInput));
        m_terminalFocus = true;
    } else {
        m_terminalFocus = false;
    }
    ImGui::PopItemWidth();
}
