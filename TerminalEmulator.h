// TerminalEmulator.h
#ifndef TERMINALEMULATOR_H
#define TERMINALEMULATOR_H

#include <QWidget>
#include <QPlainTextEdit>
#include <QLineEdit>
#include <QSocketNotifier>
//class CustomLineEdit;

class TerminalEmulator : public QWidget {
    Q_OBJECT

public:
    explicit TerminalEmulator(QWidget *parent = nullptr);
    ~TerminalEmulator() override;
    QString renderVTermScreen();

    // protected:
    //     void keyPressEvent(QKeyEvent *event) override;
protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void readFromMaster();
    void sendInput();

private:
    void handleCtrlC();
    QPlainTextEdit *outputArea;
    QLineEdit *inputArea;
    //CustomLineEdit *inputArea;
    int master_fd, slave_fd;
    QSocketNotifier *readNotifier;
    pid_t childPid;
};

#endif // TERMINALEMULATOR_H
