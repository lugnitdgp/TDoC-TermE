// TerminalEmulator.h

#ifndef TERMINALEMULATOR_H
#define TERMINALEMULATOR_H

#include <QWidget> // Base class for the UI elements
#include <QPlainTextEdit> //multi-line text display. Used here to show terminal output.
#include <QLineEdit> //single-line text input. Used for capturing user input.
#include <QSocketNotifier> //Monitors file descriptors for events

/**
 * @file TerminalEmulator.h
 * @brief Header file for the TerminalEmulator class, which provides a GUI-based terminal emulator.
 *
 * This class uses a pseudo-terminal (PTY) to interact with a shell process and provides a
 * graphical interface for terminal input and output. It supports reading and writing data
 * to the shell, handling Ctrl+C, and managing ANSI escape sequences.
 */
class TerminalEmulator : public QWidget {
    Q_OBJECT // a macro for signal slot mechanism
public:
    explicit TerminalEmulator(QWidget *parent = nullptr); // Constructor to set up UI and PTY
    ~TerminalEmulator() override; // Destructor to clean up resources

protected:
    bool eventFilter(QObject *obj, QEvent *event) override; // Filter specific events, e.g., Ctrl+C

private slots:
    void readFromMaster(); // Reads shell output from the master PTY
    void sendInput(); // Sends user input to the shell

private:
    void handleCtrlC(); // Handles Ctrl+C to send SIGINT to the shell process

    QPlainTextEdit *outputArea; // Displays terminal output
    QLineEdit *inputArea; // Captures user input
    int master_fd, slave_fd; // File descriptors for the PTY
    QSocketNotifier *readNotifier; // Monitors the PTY for readable data
    pid_t childPid; // Process ID of the child shell process
};

#endif // TERMINALEMULATOR_H
