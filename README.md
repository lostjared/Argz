# Command Line Argument Parser (Unicode Support) in C++20

## Overview
The **Argument Parser** library provides a flexible and robust mechanism for parsing command-line arguments in C++ applications. This library supports both single-character arguments (e.g., `-a`) and double-dash arguments (e.g., `--argument`), with support for Unicode strings and customizable behavior. It is designed to handle complex command-line interfaces efficiently and cleanly.

## Features
- **Supports Unicode**: Fully compatible with `std::wstring` for wide-character string handling.
- **Single and Double-Dash Arguments**: Easily parse `-a` and `--argument` style options.
- **Flexible Argument Types**:
  - `ARG_SINGLE`: Single dash arguments (e.g., `-a`).
  - `ARG_SINGLE_VALUE`: Single dash arguments with a value (e.g., `-a value`).
  - `ARG_DOUBLE`: Double dash arguments without a value (e.g., `--help`).
  - `ARG_DOUBLE_VALUE`: Double dash arguments with a value (e.g., `--file path/to/file`).
- **Error Handling**: Custom exceptions for robust error reporting.
- **Help Generation**: Automatically generates help text for available arguments.

## Installation

Include the `argz.hpp` header file in your project and compile it with your application. The library has no additional dependencies outside of the C++ standard library.

```cpp
#include "argz.hpp"
```

## Usage

### 1. Basic Setup
Create an instance of the `Argz` class and initialize it with command-line arguments (`argc` and `argv`).

```cpp
#include "argz.hpp"
#include <iostream>
#include <string>

int main(int argc, char** argv) {
    Argz<std::string> parser(argc, argv);
    
    // Define arguments
    parser.addOptionSingle('h', "Display help message")
          .addOptionDouble('f', "file", "Specify a file")
          .addOptionSingleValue('v', "Verbose output")
          .addOptionDoubleValue('o', "output", "Specify output file");
    
    // Parse arguments
    Argument<std::string> arg;
    try {
        while((value = parser.proc(arg)) != -1) {
            switch(value) {
                case 'h':
                case 'v':
                    argz.help(std::cout);
                    exit(EXIT_SUCCESS);
                    break;
                case 'f':
                    in_file = arg.arg_value;
                    break;
                case 'o':
                    out_file = arg.arg_value;
                    break;
            }
        }
    } catch (const ArgException<std::string>& e) {
        std::cerr << e.text() << std::endl;
    }

    return 0;
}
```

### 2. Adding Arguments
Define the expected arguments using the `addOption` methods:
- `addOptionSingle`: Add a single-dash argument (e.g., `-h`).
- `addOptionSingleValue`: Add a single-dash argument with a value (e.g., `-v value`).
- `addOptionDouble`: Add a double-dash argument (e.g., `--help`).
- `addOptionDoubleValue`: Add a double-dash argument with a value (e.g., `--output value`).

### 3. Processing Arguments
Use the `proc` method in a loop to process each command-line argument. This method populates an `Argument` object with the parsed argument name, value, and type.

### 4. Generating Help Text
Call the `help` method to output a list of available arguments and their descriptions.

```cpp
parser.help(std::cout);
```

## Error Handling
The library uses `ArgException` to report errors, such as missing required values or unrecognized arguments. Catch these exceptions to handle errors gracefully.

```cpp
try {
    // Argument parsing code
} catch (const ArgException<std::string>& e) {
    std::cerr << e.text() << std::endl;
}
```

## Advanced Features
- **Unicode Support**: Use `std::wstring` with the `Argz` class for wide-character applications.
- **Sorting Help Text**: Automatically organizes arguments for clean and readable help output.

## Contributing
Contributions are welcome! Please open an issue or submit a pull request if you have suggestions for improvements or new features.

## License
This project is licensed under the MIT License. See the LICENSE file for details.
