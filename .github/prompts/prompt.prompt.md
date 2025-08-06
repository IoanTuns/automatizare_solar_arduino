---
mode: ask
---
Define the task to achieve, including specific requirements, constraints, and success criteria.

# Task Definition
The task is to develop a robust and scalable automation system using the provided hardware components and coding guidelines. The system should be capable of integrating various sensors and actuators, ensuring compatibility and reliability through comprehensive testing.
The project should follow the established coding standards and project structure to maintain clarity and ease of maintenance.
# Requirements
1. **Hardware Integration**: Implement code to interface with the Arduino UNO R4 WiFi and the various sensors and actuators listed in the `readme.txt` file. Ensure that all hardware components are documented with their specifications and usage instructions.
2. **Software Development**: Write C/C++ code that adheres to the coding guidelines provided in the project context. This includes meaningful variable names, proper documentation, and adherence to the DRY principle.
3. **Testing**: Develop comprehensive unit tests for all new features. Tests should cover various scenarios and edge cases, ensuring the reliability of the codebase. Place tests in the `tests` directory and follow the naming convention `test_<feature>.py`.
4. **Documentation**: Update the project documentation to reflect any new features or changes made to the codebase. All public functions and classes must be documented using docstrings, and significant changes should be reflected in the project's documentation.
5. **Version Control**: Use Git for version control, following the commit message format specified in the project context. Ensure that all code changes are committed with clear and descriptive messages, and that the main branch is protected by requiring code reviews before merging.
6. **Code Reviews**: Submit all code changes for review by at least one other developer before merging into the main branch. Ensure that feedback is addressed and incorporated into the codebase.
# Constraints
1. **Time Constraints**: The project must be completed within the specified timeline. Regular progress updates should be provided to ensure the project stays on track.
2. **Resource Constraints**: The project must operate within the available hardware and software resources. Any limitations should be documented and taken into account during development.
3. **Quality Constraints**: The project must meet the established quality standards, including code readability, maintainability, and performance. All code must be thoroughly tested and reviewed before deployment.
# Success Criteria
1. **Functional Requirements**: The automation system should successfully integrate all specified hardware components and perform the required automation tasks as outlined in the project goals.
2. **Code Quality**: The code should adhere to the coding guidelines and standards specified in the project context. It should be well-structured, maintainable, and easy to read.
3. **Testing Coverage**: All new features should be accompanied by comprehensive unit tests that cover various scenarios and edge cases. The tests should pass successfully, ensuring the reliability of the codebase.
4. **Documentation**: The project documentation should be up-to-date and reflect all changes made   to the codebase. All public functions and classes should be documented, and significant changes should be clearly outlined in the documentation.
5. **Code Review Process**: All code changes should be reviewed by at least one other developer before being merged into the main branch. Feedback should be addressed, and the code should meet the established quality standards.
6. **Project Milestones**: The project should meet all specified milestones and deliverables within the agreed-upon timeline. Regular progress updates should be provided to ensure transparency and accountability throughout the development process.
# Additional Notes
- Ensure that all hardware connections are documented in the `readme.txt` file, section 'Pin Assignments and Functions'.
- Use pinned connections for reliable hardware connections, and document any changes made to the hardware setup.
- Regularly update the `platformio.ini` file with any new libraries added to the project, ensuring that all dependencies are versioned appropriately.
- Maintain a clear and organized project structure to facilitate collaboration and future development.