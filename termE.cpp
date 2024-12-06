#include <iostream>
#include <unistd.h>
#include <pty.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <poll.h>
using namespace std;
int main() {
    int master_fd, slave_fd;
    char slave_name[256];
    if (openpty(&master_fd, &slave_fd, slave_name, NULL, NULL) == -1) {
        perror("openpty");
        return 1;
    }
    cout << "Slave terminal name: " << slave_name << endl;
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return 1;
    }
    if (pid == 0) { 
        close(master_fd);
        setsid(); 
        if (ioctl(slave_fd, TIOCSCTTY, 0) == -1) {
            perror("ioctl");
            return 1;
        }
        dup2(slave_fd, STDIN_FILENO);
        dup2(slave_fd, STDOUT_FILENO);
        dup2(slave_fd, STDERR_FILENO);
        close(slave_fd);
        execlp("/bin/bash", "bash", nullptr);
        perror("execlp");
        return 1;
    } else { 
        close(slave_fd);

        struct pollfd fds[2];
        fds[0].fd = master_fd;      
        fds[0].events = POLLIN;
        fds[1].fd = STDIN_FILENO;    
        fds[1].events = POLLIN;
        char buffer[256];
        while (true) {
            int ret = poll(fds, 2, -1); 
            if (ret == -1) {
                perror("poll");
                break;
            }
            if (fds[0].revents & POLLIN) { 
                ssize_t count = read(master_fd, buffer, sizeof(buffer) - 1);
                if (count > 0) {
                    buffer[count] = '\0';
                   cout << buffer; 
                }
            }
            if (fds[1].revents & POLLIN) { 
                ssize_t count = read(STDIN_FILENO, buffer, sizeof(buffer) - 1);
                if (count > 0) {
                    buffer[count] = '\0';
                    write(master_fd, buffer, count);
                }
            }
        }
    }

    return 0;
}

