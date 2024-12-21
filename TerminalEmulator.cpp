// TerminalEmulator.cpp
#include "TerminalEmulator.h"

#include <QVBoxLayout> //Provides vertical layout management.
#include <QApplication> //The base class for Qt GUI applications.
#include <QTimer>
#include <QSocketNotifier> //Monitors file descriptors for I/O operations.
#include <QRegularExpression> //Used for pattern matching
#include <pty.h> //Manages pseudo-terminal devices.
#include <unistd.h> //Provides system-level I/O operations.
#include <termios.h> //Configures terminal attributes.
#include <sys/ioctl.h> //Handles signals like SIGINT.
#include <signal.h> //Manages terminal I/O control.
#include <poll.h> // Handles polling file descriptors.
#include <iostream> //For C++ standard input and output.
#include <cstdlib> //Provides general utilities like exit.


// Definition of TerminalEmulator Constructor
TerminalEmulator::TerminalEmulator(QWidget *parent) : QWidget(parent), outputArea(nullptr), inputArea(nullptr), master_fd(-1), slave_fd(-1), readNotifier(nullptr),childPid(-1) {
     // Setup the UI with a vertical box layout containing an output area and input area
    outputArea = new QPlainTextEdit(this); //we pass this which is the parent of outputArea
    inputArea = new QLineEdit(this);
    inputArea->setFocus(); // Will shift the focus to the input area when the Application opens
    outputArea->setReadOnly(true); // Sets the Output Area to readonly mode

    QVBoxLayout *layout = new QVBoxLayout(this); // Creates an instance of QVBoxLayout which contains the input and output area
    layout->addWidget(outputArea);
    layout->addWidget(inputArea);

    setLayout(layout); // Sets The Application layout in a vertical manner
    inputArea->installEventFilter(this); //Installs an event filter on the `inputArea` to capture and handle specific events.


    // Setup pseudo-terminal
    if (openpty(&master_fd, &slave_fd, nullptr, nullptr, nullptr) == -1) {
        perror("openpty");
        exit(1);
    }

    // Fork the child process - Creates a duplicate process
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) { // Child process
        ::close(master_fd); // Close master in child process

        setsid(); // Create a new session to detach the child process from the terminal

        if (ioctl(slave_fd, TIOCSCTTY, 0) == -1) { // Making the Slave side the controlling terminal
            perror("ioctl");
            exit(1);
        }
        dup2(slave_fd, STDIN_FILENO); // Redirect standard input to the slave PTY
        dup2(slave_fd, STDOUT_FILENO); // Redirect standard output to the slave PTY
        dup2(slave_fd, STDERR_FILENO); // Redirect standard error to the slave PTY
        ::close(slave_fd); // Close slave after duplication

        // Set the TERM environment variable to 'xterm-256color' to enable 256-color support
        if (setenv("TERM", "xterm-256color", 1) == -1) {
            perror("setenv");
            exit(1);
        }
        // Retrieve the user's default shell from the SHELL environment variable
        const char *shell = getenv("SHELL");
        if (!shell) {
            perror("getenv");
            exit(1);
        }
        const char *logMessage = "Slave terminal started successfully.\n";
        write(STDOUT_FILENO, logMessage, strlen(logMessage));
        execlp(shell, shell, nullptr); // Execute the retrieved shell
        perror("execlp");
        exit(1);
    } else { // Parent process
        childPid = pid; // Store the child process ID for later use
        ::close(slave_fd); // Close slave in parent process

        // Monitor master_fd for output
        readNotifier = new QSocketNotifier(master_fd, QSocketNotifier::Read, this);
        connect(readNotifier, &QSocketNotifier::activated, this, &TerminalEmulator::readFromMaster);

        // Handle user input
        connect(inputArea, &QLineEdit::returnPressed, this, &TerminalEmulator::sendInput);

    }

}

TerminalEmulator::~TerminalEmulator() {
    ::close(master_fd); // Explicitly close the master file descriptor
    if (childPid > 0) {
        kill(childPid, SIGKILL); // Kill child process if it is still running
    }
}

void TerminalEmulator::readFromMaster() {
    // Buffer to hold data read from the master PTY
    char buffer[256];

    // Read data from the master PTY into the buffer
    ssize_t count = read(master_fd, buffer, sizeof(buffer) - 1);

    if (count > 0) {
        buffer[count] = '\0'; // Null-terminate the buffer to make it a valid C-string

        // Convert the buffer to a QString
        QString output = QString::fromLocal8Bit(buffer);

        // Handle ANSI escape sequences for clearing the terminal screen
        if (output.contains("\033[H\033[2J")) {
            outputArea->clear(); // Clear the terminal output
            output.remove(QRegularExpression("\033\\[H\\033\\[2J")); // Remove the sequence
        }

        // Remove ANSI escape sequences for styling and control characters
        output.remove(QRegularExpression("\x1B\\[[0-9;]*[a-zA-Z]")); // CSI sequences
        output.remove(QRegularExpression("\x1B\\][^\\x07]*\\x07"));  // OSC sequences
        output.remove(QRegularExpression("\x1B\\[\\?2004[h|l]"));    // Bracketed paste mode sequences
        output.remove(QRegularExpression("\x1B\\(B"));               // Character set sequence
        output.remove(QRegularExpression("\x1B\\]0;[^\\x07]*\\x07"));// Title setting sequences
        output.remove(QRegularExpression("[\x00-\x1F\x7F]"));           // Remove other non-printable control characters

        // Append the cleaned output to the output area for display
        outputArea->appendPlainText(output);
    } else if (count == 0) { // EOF
        readNotifier->setEnabled(false); // Disable the read notifier on EOF
    } else { // Error
        perror("read"); // Print an error message if reading fails
    }
}



void TerminalEmulator::sendInput() {
    // Retrieve the user input from the input area, append a newline character
    QString input = inputArea->text() + "\n";

    // Debugging output to display the input being sent
    qDebug() << "Sending input to master_fd:" << input;

    // Write the input to the master PTY
    ssize_t bytesWritten = write(master_fd, input.toLocal8Bit().constData(), input.size());

    // Check if the write operation was successful
    if (bytesWritten == -1) {
        // Error handling: Print an error message if the write operation failed
        perror("write failed");
    } else {
        // Debugging output: Show the number of bytes written to the master PTY
        qDebug() << "Bytes written:" << bytesWritten;
    }

    // Clear the input area after sending the input
    inputArea->clear();
}


bool TerminalEmulator::eventFilter(QObject *obj, QEvent *event) {
    if (obj == inputArea && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->modifiers() == Qt::ControlModifier && keyEvent->key() == Qt::Key_C) {
            handleCtrlC();
            return true; // Do not process further
        }
    }
    return QWidget::eventFilter(obj, event);
}

void TerminalEmulator::handleCtrlC() {
    if (childPid > 0) {
        if (kill(-childPid, SIGINT) == -1) {
            perror("kill failed");
        }
    }
}
