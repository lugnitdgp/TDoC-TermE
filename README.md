# TermE - A Qt-Based Terminal Emulator
![Static Badge](https://img.shields.io/badge/Built%20With%20-%20C%2B%2B%20-%20Green) ![Static Badge](https://img.shields.io/badge/Contributions%20-%20Welcome%20%20-%20orange)

TermE is a lightweight GUI-based terminal application built with Qt, a GUI toolkit using C++, that provides a graphical interface for interacting with a bash shell. It leverages a pseudo-terminal mechanism to allow users to run and manage terminal commands.
## :rocket: Features
✅ **Pseudo-Terminal Integration**: Communicates with a bash shell using a master-slave pseudo-terminal mechanism.  
✅ **Real-time Input/Output**: Supports interactive user input and live terminal output updates.  
✅ **Customizable Interface**: Built with Qt, featuring separate areas for input and output with a responsive layout.  
✅ **Error Handling**: Includes robust mechanisms for managing terminal communication and system call failures.  
✅ **Cross-Platform Compatibility**: Designed to run on Linux systems with support for bash shell operations.
## 💻 Getting Started

Follow these steps to set up and build the Terminal Emulator project on your system.

### 📋 Prerequisites

Before you begin, ensure you have the following installed on your system:

- **Qt Framework**: Download and install the latest version of Qt from the [official Qt website](https://www.qt.io/download). Ensure that you install both the Qt libraries and the Qt Creator IDE.The steps to setup Qt creator are mentioned ahead.
- **C++ Compiler**: Make sure a C++ compiler compatible with Qt is installed .
- **Git**: Install Git for cloning the repository. You can download it from [here](https://git-scm.com/).

## 🛠️ Running TermE on your local machine 

### 1️⃣ Setting Up Qt Creator
1. Run the following command to install essential packages which ensure necessary tools for C++ compilation and graphical rendering.
   ```bash
   sudo apt-get install build-essential libgl1-mesa-dev

2. **Download and Install Qt**  
   - Visit the [Qt Downloads page](https://www.qt.io/download) and select the version appropriate for your operating system.
   - During installation, ensure you include:
     - Qt libraries for your target platform.
     - Qt Creator IDE.
     - MinGW (Windows only) or GCC (Linux/macOS).


### 2️⃣ Cloning the Repository

1. Open a terminal or Git Bash and run the following command to clone the repository:
   ```bash
   git clone https://github.com/DevanshS9881/Term_Emulator.git
   
### 3️⃣ Building the project using Qt Creator
1.Navigate to the project directory and open it using Qt Creator IDE.
2. Build the project:
   - Click on the Build button (hammer icon) to compile the project.
3.Run the application:
   - Click the Run button (green play icon) to launch the Terminal Emulator.
  
   
