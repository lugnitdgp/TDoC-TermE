// TerminalEmulator.h
#ifndef TERMINALEMULATOR_H
#define TERMINALEMULATOR_H

#include <QWidget>
#include <QPlainTextEdit>
#include <QLineEdit>
#include <QSocketNotifier>

class TerminalEmulator : public QWidget {
    Q_OBJECT

public:
    explicit TerminalEmulator(QWidget *parent = nullptr);
    ~TerminalEmulator() override;

    // protected:
    //     void keyPressEvent(QKeyEvent *event) override;

private slots:
    void readFromMaster();
    void sendInput();

private:
    QPlainTextEdit *outputArea;
    QLineEdit *inputArea;
    int master_fd, slave_fd;
    QSocketNotifier *readNotifier;
};

#endif // TERMINALEMULATOR_H
