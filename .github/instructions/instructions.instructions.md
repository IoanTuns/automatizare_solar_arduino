---
applyTo: '**'
---
Provide project context and coding guidelines that AI should follow when generating code, answering questions, or reviewing changes.
# Project Goals
1. **Automation**: The primary goal of the project is to create a robust and scalable automation system that can be easily extended with new features. The project should prioritize maintainability and readability of the code.
2. **Integration**: The project aims to integrate various hardware components and software libraries to create a cohesive system. This includes ensuring compatibility between different components and libraries.
3. **Testing**: The project should include comprehensive unit tests to ensure the reliability of the codebase. All new features must be accompanied by tests that cover various scenarios and edge cases.

# Hardware reference
The project utilizes various hardware components to achieve its automation goals. The main hardware components include:
- **Arduino UNO R4 WiFi**: The primary microcontroller for controlling sensors and actuators.
- **Key Components**: PCF8574 I/O Expanders, CD74HC4067 Analog Multiplexer, DHT sensors, DS3231 RTC.
- **Other Sensors and Actuators**: Refer to the `readme.txt` file, section 'Used Components' for a complete list.

# Hardware external
- **Power Supply**: Ensure a stable power supply for the Arduino and connected components.
- **Breadboard and Jumper Wires**: For prototyping and connecting components. Pinned connections should be used for reliable connections. All connections should be documented in the `readme.txt` file, section 'Pin Assignments and Functions'.

# Project Context and Coding Guidelines
1. **Project Structure**: The project follows a standard structure with separate folders for source code, tests, and documentation. All source code should be placed in the `src` directory, while tests should be in the `tests` directory.
    - `src/`: Contains all the source code files.
    - `tests/`: Contains unit tests for the source code.
    - `docs/`: Contains project documentation (if applicable).

2. **Project used Hardware**: The project is aimed to build up an automation system. All hardware in use is listed in the `readme.txt` file. All hardware components should be documented with their specifications and usage instructions.

3. **Project used Software**: The project is built using C/C++ and relies on several libraries, which are maintained in the `platformio.ini` file. Ensure that all dependencies are documented and versioned appropriately.
- **PlatformIO**: The project uses PlatformIO for managing libraries and dependencies. Ensure that the `platformio.ini` file is updated with any new libraries added to the project.

# Coding Guidelines
1. **Version Control**: The project uses Git for version control. All code changes should be committed with clear and descriptive commit messages. Use the following format for commit messages:
   ```
   <type>(<scope>): <subject>

   <body>

   <footer>
   ```
   Where:
   - `<type>` is the type of change (e.g., feat, fix, docs, style, refactor, test, chore).
   - `<scope>` is the area of the codebase affected (e.g., src, tests, docs).
   - `<subject>` is a brief summary of the change.
   - `<body>` is a more detailed explanation of the change (optional).
   - `<footer>` is any additional information (e.g., related issues, breaking changes).

2. **Coding Standards**: All code should adhere to the following coding standards:
   - Use meaningful variable and function names.
   - Write comments to explain complex logic.
   - Follow the DRY (Don't Repeat Yourself) principle.

3. **Testing**: All new features must include unit tests. Tests should be placed in the `tests` directory and follow the naming convention `test_<feature>.py`.
3. **Testing**: All new features must include unit tests. Since this is a C++/PlatformIO project, tests should be written in C++ (e.g., using the Unity framework). Tests should be placed in the `test` directory and follow a convention like `test/test_feature_name/test_main.cpp`.

4. **Documentation**: All public functions and classes must be documented using Doxygen-style comments (`/** ... */`), which is consistent with the existing C++ code. Additionally, any significant changes to the codebase should be reflected in the project's documentation.

5. **Code Reviews**: All code changes must be reviewed by at least one other developer before being merged into the main branch.