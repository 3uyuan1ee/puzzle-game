# Puzzle Game Contributing Guide

Thank you for your interest in contributing to Puzzle Game! This document provides guidelines and instructions for contributors.

## 🤝 How to Contribute

### Reporting Bugs
- Use the [GitHub Issues](https://github.com/yourusername/puzzle-game/issues) page
- Include steps to reproduce, expected behavior, and actual behavior
- Provide system information (OS, Qt version, etc.)

### Suggesting Features
- Create an issue with the "enhancement" label
- Describe the feature in detail and explain why it would be useful
- Consider implementation complexity and user benefit

### Code Contributions
1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Make your changes
4. Add tests if applicable
5. Update documentation
6. Submit a pull request

## 📝 Code Style

### C++ Guidelines
- Use **C++17** features when appropriate
- Follow Qt coding conventions
- Use smart pointers and modern C++ constructs
- Keep functions small and focused
- Use descriptive variable and function names
- Add comments for complex logic

### Naming Conventions
- **Classes**: PascalCase (`MyClass`)
- **Functions**: camelCase (`myFunction()`)
- **Variables**: camelCase (`myVariable`)
- **Constants**: UPPER_SNAKE_CASE (`MY_CONSTANT`)
- **Files**: lowercase_with_underscores (`my_class.h`)

### Qt Specific
- Use Qt's signal-slot mechanism with modern syntax
- Prefer Qt containers over STL when possible
- Use Qt's memory management (QObject parent-child relationships)
- Follow Qt's resource system for images and other assets

## 🧪 Testing

### Running Tests
```bash
# Build with test configuration
qmake PuzzleGame.pro CONFIG+=test
make

# Run tests
./tests/run_tests.sh
```

### Writing Tests
- Create unit tests for new features
- Test edge cases and error conditions
- Use Qt Test framework
- Aim for high code coverage

## 📚 Documentation

### Code Documentation
- Use Doxygen-style comments for classes and public methods
- Document parameters, return values, and possible exceptions
- Include usage examples for complex functionality

### User Documentation
- Update README.md for significant features
- Add inline help text for new UI elements
- Update screenshots and guides as needed

## 🔄 Pull Request Process

### Before Submitting
- Ensure your code compiles without warnings
- Run all tests and ensure they pass
- Update documentation
- Check code formatting
- Test your changes on multiple platforms if possible

### Pull Request Template
```markdown
## Description
Brief description of changes

## Type of Change
- [ ] Bug fix
- [ ] New feature
- [ ] Breaking change
- [ ] Documentation update

## Testing
- [ ] Code compiles successfully
- [ ] All tests pass
- [ ] Manual testing performed

## Checklist
- [ ] Documentation updated
- [ ] Code follows style guidelines
- [ ] No breaking changes
- [ ] Ready for review
```

## 🏗️ Development Setup

### Environment Setup
1. Install Qt 6.6 or higher
2. Clone the repository
3. Install development dependencies
4. Build the project

### IDE Configuration
- **Qt Creator**: Recommended IDE
- **VS Code**: With Qt extension
- **CLion**: With Qt plugin

## 📋 Issue Labels

- **bug**: Bug reports
- **enhancement**: Feature requests
- **documentation**: Documentation issues
- **good first issue**: Good for newcomers
- **help wanted**: Needs community help
- **question**: General questions

## 🎯 Release Process

### Versioning
- Follow Semantic Versioning (MAJOR.MINOR.PATCH)
- MAJOR: Breaking changes
- MINOR: New features
- PATCH: Bug fixes

### Release Checklist
- Update version number
- Update changelog
- Create release tag
- Build binaries for all platforms
- Update documentation
- Publish release

## 📞 Community

### Getting Help
- GitHub Issues: For bug reports and feature requests
- Discussions: For general questions and discussions
- Email: For private inquiries

### Code of Conduct
- Be respectful and inclusive
- Focus on constructive feedback
- Help newcomers learn and grow
- Follow open source best practices

---

Happy coding! 🎉