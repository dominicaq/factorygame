#ifndef TERMINAL_H
#define TERMINAL_H

#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <imgui.h>

class Terminal {
public:
    enum class MessageType {
        STANDARD,
        ERR,
        WARNING,
        INFO,
        DEBUG
    };

    struct Message {
        std::string text;
        MessageType type;

        Message(const std::string& msg, MessageType t = MessageType::STANDARD)
            : text(msg), type(t) {}
    };

    // Custom streambuf to capture output
    class TerminalBuf : public std::streambuf {
    public:
        TerminalBuf(Terminal& terminal, MessageType type);
        virtual ~TerminalBuf();

    protected:
        // Override std::streambuf methods for output capturing
        virtual int overflow(int c = EOF) override;
        virtual int sync() override;

    private:
        Terminal& m_terminal;
        MessageType m_type;
        std::string m_buffer;
    };

    Terminal();
    ~Terminal();

    // Singleton access
    static Terminal& getInstance();

    // ImGui drawing function (should be called from your ImGui UI code)
    void drawTerminal();

    // Add a message directly (used by TerminalBuf)
    void addMessage(const std::string& message, MessageType type);
    ImVec4 getColor(MessageType type) const;
    void clear();

    // stream redirection
    void registerStdCapture();
    void restoreStdCapture();

    // Misc
    void executeCommand(const std::string& command);

private:
    char m_terminalInput[256];
    bool m_terminalFocus;
    std::vector<Message> m_messages;
    const size_t m_maxMessages;
    std::mutex m_messagesMutex;

    // Custom stream buffers
    std::unique_ptr<TerminalBuf> m_coutBuf;
    std::unique_ptr<TerminalBuf> m_cerrBuf;

    // Original stream buffers (saved for restoration)
    std::streambuf* m_originalCoutBuf;
    std::streambuf* m_originalCerrBuf;

    // Static instance for singleton
    static Terminal* s_instance;

    // Process a line of output, detecting message types
    void processLine(const std::string& line, MessageType defaultType);

    // Internal function to handle commands
    void handleCommand(const std::string& command);
};

#endif // TERMINAL_H