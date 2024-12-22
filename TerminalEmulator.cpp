// TerminalEmulator.cpp
#include "TerminalEmulator.h"

#include <QVBoxLayout>
#include <QApplication>
#include <QTimer>
#include <QSocketNotifier>
#include <QRegularExpression>
#include <pty.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <poll.h>
#include <iostream>
#include <cstdlib>
#include <QPushButton>
#include <QMenu>
#include <QAction>
#include <QFont>

// Definition of TerminalEmulator Constructor
TerminalEmulator::TerminalEmulator(QWidget *parent) : QWidget(parent), outputArea(nullptr), inputArea(nullptr), master_fd(-1), slave_fd(-1), readNotifier(nullptr), childPid(-1) {
    // Setup the UI with a vertical box layout containing an output area, input area, and buttons
    outputArea = new QPlainTextEdit(this);
    inputArea = new QLineEdit(this);
    inputArea->setFocus();
    outputArea->setReadOnly(true);

    // Set the text in outputArea to bold
    QFont font = outputArea->font();
    font.setBold(true);
    outputArea->setFont(font);

    // Change the background color of inputArea
    QPalette inputPalette = inputArea->palette();
    inputPalette.setColor(QPalette::Base, Qt::lightGray);
    inputArea->setPalette(inputPalette);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(outputArea);
    layout->addWidget(inputArea);

    // Create buttons for changing background and text color
    QPushButton *backgroundColorButton = new QPushButton("Change Background Colour", this);
    QPushButton *textColorButton = new QPushButton("Change Text Colour", this);
    layout->addWidget(backgroundColorButton);
    layout->addWidget(textColorButton);

    setLayout(layout);
    inputArea->installEventFilter(this);

    // Connect the buttons' clicked signals to the slots
    connect(backgroundColorButton, &QPushButton::clicked, this, &TerminalEmulator::changeBackgroundColor);
    connect(textColorButton, &QPushButton::clicked, this, &TerminalEmulator::changeTextColor);

    // Setup pseudo-terminal
    if (openpty(&master_fd, &slave_fd, nullptr, nullptr, nullptr) == -1) {
        perror("openpty");
        exit(1);
    }

    // Fork the child process
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) { // Child process
        ::close(master_fd);

        setsid();

        if (ioctl(slave_fd, TIOCSCTTY, 0) == -1) {
            perror("ioctl");
            exit(1);
        }
        dup2(slave_fd, STDIN_FILENO);
        dup2(slave_fd, STDOUT_FILENO);
        dup2(slave_fd, STDERR_FILENO);
        ::close(slave_fd);

        if (setenv("TERM", "xterm-256color", 1) == -1) {
            perror("setenv");
            exit(1);
        }
        const char *shell = getenv("SHELL");
        if (!shell) {
            perror("getenv");
            exit(1);
        }
        const char *logMessage = "Slave terminal started successfully.\n";
        write(STDOUT_FILENO, logMessage, strlen(logMessage));
        execlp(shell, shell, nullptr);
        perror("execlp");
        exit(1);
    } else { // Parent process
        childPid = pid;
        ::close(slave_fd);

        readNotifier = new QSocketNotifier(master_fd, QSocketNotifier::Read, this);
        connect(readNotifier, &QSocketNotifier::activated, this, &TerminalEmulator::readFromMaster);

        connect(inputArea, &QLineEdit::returnPressed, this, &TerminalEmulator::sendInput);
    }
}

TerminalEmulator::~TerminalEmulator() {
    ::close(master_fd);
    if (childPid > 0) {
        kill(childPid, SIGKILL);
    }
}

void TerminalEmulator::readFromMaster() {
    char buffer[256];
    ssize_t count = read(master_fd, buffer, sizeof(buffer) - 1);

    if (count > 0) {
        buffer[count] = '\0';
        QString output = QString::fromLocal8Bit(buffer);

        if (output.contains("\033[H\033[2J")) {
            outputArea->clear();
            output.remove(QRegularExpression("\033\\[H\\033\\[2J"));
        }

        output.remove(QRegularExpression("\x1B\\[[0-9;]*[a-zA-Z]"));
        output.remove(QRegularExpression("\x1B\\][^\\x07]*\\x07"));
        output.remove(QRegularExpression("\x1B\\[\\?2004[h|l]"));
        output.remove(QRegularExpression("\x1B\\(B"));
        output.remove(QRegularExpression("\x1B\\]0;[^\\x07]*\\x07"));
        output.remove(QRegularExpression("[\x00-\x1F\x7F]"));

        outputArea->appendPlainText(output);
    } else if (count == 0) {
        readNotifier->setEnabled(false);
    } else {
        perror("read");
    }
}

void TerminalEmulator::sendInput() {
    QString input = inputArea->text() + "\n";
    qDebug() << "Sending input to master_fd:" << input;

    ssize_t bytesWritten = write(master_fd, input.toLocal8Bit().constData(), input.size());

    if (bytesWritten == -1) {
        perror("write failed");
    } else {
        qDebug() << "Bytes written:" << bytesWritten;
    }

    inputArea->clear();
}

bool TerminalEmulator::eventFilter(QObject *obj, QEvent *event) {
    if (obj == inputArea && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->modifiers() == Qt::ControlModifier && keyEvent->key() == Qt::Key_C) {
            handleCtrlC();
            return true;
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

// Slot to handle background colour change
void TerminalEmulator::changeBackgroundColor() {
    QMenu colorMenu;
    QAction *blackAction = colorMenu.addAction("Black");
    QAction *whiteAction = colorMenu.addAction("White");
    QAction *redAction = colorMenu.addAction("Red");
    QAction *greenAction = colorMenu.addAction("Green");
    QAction *blueAction = colorMenu.addAction("Blue");
    QAction *yellowAction=colorMenu.addAction("Yellow");
     QAction *magentaAction = colorMenu.addAction("Magenta");
      QAction *cyanAction = colorMenu.addAction("Cyan");

    QAction *selectedAction = colorMenu.exec(QCursor::pos());
    if (selectedAction) {
        QColor color;
        if (selectedAction == blackAction) {
            color = Qt::black;
        } else if (selectedAction == whiteAction) {
            color = Qt::white;
        } else if (selectedAction == redAction) {
            color = Qt::red;
        } else if (selectedAction == greenAction) {
            color = Qt::green;
        } else if (selectedAction == blueAction) {
            color = Qt::blue;
        }
        else if (selectedAction == yellowAction)
        {
            color=Qt::yellow;
        }
        else if (selectedAction ==magentaAction) {
            color = Qt::magenta;
        }
        else if (selectedAction == cyanAction) {
            color = Qt::cyan;
        }

        QPalette outputPalette = outputArea->palette();
        outputPalette.setColor(QPalette::Base, color);
        outputArea->setPalette(outputPalette);

        QPalette inputPalette = inputArea->palette();
        inputPalette.setColor(QPalette::Base, color);
        inputArea->setPalette(inputPalette);
    }
}

// Slot to handle text colour change
void TerminalEmulator::changeTextColor() {
    QMenu colorMenu;
    QAction *blackAction = colorMenu.addAction("Black");
    QAction *whiteAction = colorMenu.addAction("White");
    QAction *redAction = colorMenu.addAction("Red");
    QAction *greenAction = colorMenu.addAction("Green");
    QAction *blueAction = colorMenu.addAction("Blue");
    QAction *yellowAction=colorMenu.addAction("Yellow");
    QAction *magentaAction = colorMenu.addAction("Magenta");
    QAction *cyanAction = colorMenu.addAction("Cyan");

    QAction *selectedAction = colorMenu.exec(QCursor::pos());
    if (selectedAction) {
        QColor color;
        if (selectedAction == blackAction) {
            color = Qt::black;
        } else if (selectedAction == whiteAction) {
            color = Qt::white;
        } else if (selectedAction == redAction) {
            color = Qt::red;
        } else if (selectedAction == greenAction) {
            color = Qt::green;
        } else if (selectedAction == blueAction) {
            color = Qt::blue;
        }
        else if (selectedAction == yellowAction)
        {
            color=Qt::yellow;
        }
        else if (selectedAction ==magentaAction) {
            color = Qt::magenta;
        }
        else if (selectedAction == cyanAction) {
            color = Qt::cyan;
        }

        QPalette outputPalette = outputArea->palette();
        outputPalette.setColor(QPalette::Text, color);
        outputArea->setPalette(outputPalette);

        QPalette inputPalette = inputArea->palette();
        inputPalette.setColor(QPalette::Text, color);
        inputArea->setPalette(inputPalette);
    }
}
