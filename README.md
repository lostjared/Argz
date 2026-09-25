# Argz
# Command Line Argument Parser in C++20

## Overview
The **Argument Parser** library provides a flexible and robust mechanism for parsing command-line arguments in C++ applications. This library supports both single-character arguments (e.g., `-a`) and double-dash arguments (e.g., `--argument`), with support for Unicode strings and customizable behavior. It is designed to handle complex command-line interfaces efficiently and cleanly.

## Features
- **Wide strings**: The parser and help output also support `std::wstring`. Conversion from `char** argv` widens bytes; it does not decode UTF-8.
- **Single and Double-Dash Arguments**: Easily parse `-a` and `--argument` style options.
- **Flexible Argument Types**:
  - `ARG_SINGLE`: Single dash arguments (e.g., `-a`).
  - `ARG_SINGLE_VALUE`: Single dash arguments with a value (e.g., `-a value`).
  - `ARG_DOUBLE`: Double dash arguments without a value (e.g., `--help`).
  - `ARG_DOUBLE_VALUE`: Double dash arguments with a value (e.g., `--file path/to/file`).
- **Error Handling**: Custom exceptions for robust error reporting.
- **Help Generation**: Automatically generates help text for available arguments.

## Installation

Install with CMake, then link the header-only target:

```sh
cmake -S . -B build
cmake --install build --prefix /your/prefix
```

```cmake
find_package(Argz 1 CONFIG REQUIRED)
target_link_libraries(your_app PRIVATE Argz::argz)
```

The header is installed at `/your/prefix/include/Argz/argz.hpp`. You can also include it directly from the source tree by adding `include` to your compiler's include path.

```cpp
#include <Argz/argz.hpp>
```

## Usage

### 1. Basic Setup
Create an instance of the `Argz` class and initialize it with command-line arguments (`argc` and `argv`).

```cpp
#include <Argz/argz.hpp>
#include <cstdlib>
#include <iostream>
#include <string>

int main(int argc, char** argv) {
    Argz<std::string> parser(argc, argv);
    
    // Define arguments
    parser.addOptionSingle('h', "Display help message")
          .addOptionDoubleValue('f', "file", "Specify a file")
          .addOptionSingle('v', "Verbose output")
          .addOptionDoubleValue('o', "output", "Specify output file");
    
    // Parse arguments
    Argument<std::string> arg;
    std::string  in_file, out_file;
    int value = 0;
    try {
        while((value = parser.proc(arg)) != -1) {
            switch(value) {
                case 'h':
                case 'v':
                    parser.help(std::cout);
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

    if(in_file.length() == 0) {
        std::cout << "Error required input...\n";
        exit(EXIT_FAILURE);
    }
    // use in_file
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
- **Wide strings**: Use `std::wstring` with the `Argz` class and pass a `std::wostream` to `help`.
- **Sorting Help Text**: Automatically organizes arguments for clean and readable help output.

## Contributing
Contributions are welcome! Please open an issue or submit a pull request if you have suggestions for improvements or new features.

## License
This project is licensed under the MIT License. See the LICENSE file for details.
