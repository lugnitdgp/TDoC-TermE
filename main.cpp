// main.cpp
#include <QApplication>
#include "TerminalEmulator.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    TerminalEmulator terminal;
    terminal.setWindowTitle("Qt Terminal Emulator");
    terminal.resize(800, 600);
    terminal.show();

    return app.exec();
}
