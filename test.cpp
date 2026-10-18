#include "argz.hpp"

#include <cstdlib>
#include <initializer_list>
#include <iostream>
#include <sstream>

#define CHECK(condition)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               \
    do {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               \
        if (!(condition)) {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            \
            std::cerr << __FILE__ << ':' << __LINE__ << ": check failed: " #condition << '\n';                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         \
            std::exit(EXIT_FAILURE);                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   \
        }                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              \
    } while (false)

struct CommandLine {
    std::vector<std::string> storage;
    std::vector<char *> argv;

    CommandLine(std::initializer_list<const char *> tokens) : storage{"argz"} {
        for (const auto *token : tokens)
            storage.emplace_back(token);
        for (auto &token : storage)
            argv.push_back(token.data());
    }

    template <typename String> void load(Argz<String> &parser) { parser.initArgs(static_cast<int>(argv.size()), argv.data()); }
};

template <typename String> String text(const char *value) {
    String result;
    while (*value)
        result += static_cast<typename String::value_type>(*value++);
    return result;
}

template <typename String> void check_same(const Argument<String> &a, const Argument<String> &b) {
    CHECK(a.arg_name == b.arg_name);
    CHECK(a.arg_letter == b.arg_letter);
    CHECK(a.arg_value == b.arg_value);
    CHECK(a.arg_type == b.arg_type);
    CHECK(a.desc == b.desc);
}

template <typename String> Argz<String> options() {
    Argz<String> parser;
    parser.addOptionSingle('a', text<String>("first")).addOptionSingle('b', text<String>("second")).addOptionSingleValue('n', text<String>("number")).addOptionDouble(256, text<String>("verbose"), text<String>("verbose flag")).addOptionDoubleValue(257, text<String>("number"), text<String>("long number"));
    return parser;
}

template <typename String> void parsing_errors() {
    auto parser = options<String>();
    Argument<String> arg;
    // Seed every output field so partial overwrites cannot pass unnoticed.
    CommandLine seed{"--number=original"};
    seed.load(parser);
    CHECK(parser.proc(arg) == 257);
    const auto previous = arg;

    struct ErrorCase {
        const char *token;
        const char *following;
        const char *message;
        bool clustered;
    };
    const ErrorCase cases[] = {
        {"-n", "", "Expected Value", false},
        {"-n", "-a", "Expected Value", false},
        {"-an", "-b", "Expected Value", true},
        {"--number", "--verbose", "Expected Value", false},
        {"--number=", "", "Expected Value", false},
        {"--verbose=yes", "", "Option does not take a value", false},
        {"--unknown", "", "Error argument: --unknown switch not found", false},
        {"-az", "", "Error argument: -az switch not found", true},
    };
    for (const auto &test : cases) {
        CommandLine command{test.token, test.following};
        command.load(parser);
        if (test.clustered)
            CHECK(parser.proc(arg) == 'a');
        arg = previous;
        // Retrying an error must fail at the same cursor and preserve all fields.
        for (int attempt = 0; attempt < 2; ++attempt) {
            bool caught = false;
            try {
                parser.proc(arg);
            } catch (const ArgException<String> &error) {
                caught = true;
                CHECK(error.text() == text<String>(test.message));
            }
            CHECK(caught);
            CHECK(parser.count() == 0);
            check_same(arg, previous);
        }
    }
    for (const auto *token : {"-n", "-an", "--number"}) {
        CommandLine command{token};
        command.load(parser);
        if (std::string(token) == "-an")
            CHECK(parser.proc(arg) == 'a');
        arg = previous;
        bool caught = false;
        try {
            parser.proc(arg);
        } catch (const ArgException<String> &error) {
            caught = true;
            CHECK(error.text() == text<String>("Expected Value"));
        }
        CHECK(caught);
        CHECK(parser.count() == 0);
        check_same(arg, previous);
    }
}

template <typename String> void parsing_compatibility() {
    auto parser = options<String>();
    Argument<String> arg;
    CommandLine mixed{"-abn42", "--verbose", "--number=-file", "plain", "", "-", "--", "-a", "--number=3"};
    mixed.load(parser);
    CHECK(parser.proc(arg) == 'a' && parser.count() == 0);
    CHECK(parser.proc(arg) == 'b' && parser.count() == 0);
    CHECK(parser.proc(arg) == 'n' && arg.arg_value == text<String>("42") && parser.count() == 1);
    CHECK(parser.proc(arg) == 256 && arg.arg_value.empty() && parser.count() == 2);
    CHECK(parser.proc(arg) == 257 && arg.arg_value == text<String>("-file"));
    for (const auto *value : {"plain", "", "-", "-a", "--number=3"}) {
        CHECK(parser.proc(arg) == '-');
        CHECK(arg.arg_type == ArgType::ARG_NONE);
        CHECK(arg.arg_letter == 0); // Preserve the historical positional field value.
        CHECK(arg.arg_name == text<String>(value) && arg.arg_value == text<String>(value));
        CHECK(arg.desc.empty());
    }
    CHECK(parser.count() == 9);
    auto last = arg;
    CHECK(parser.proc(arg) == -1);
    CHECK(parser.proc(arg) == -1);
    check_same(arg, last);
    parser.reset();
    CHECK(parser.count() == 0 && parser.proc(arg) == 'a');

    for (const auto *value : {"-2", "-2.5", "-.5", "-1.", "-2e-3", "+4", "file.txt"}) {
        CommandLine command{"-n", value, "--number", value};
        command.load(parser);
        CHECK(parser.proc(arg) == 'n' && arg.arg_value == text<String>(value));
        CHECK(parser.count() == 2);
        CHECK(parser.proc(arg) == 257 && arg.arg_value == text<String>(value));
        CHECK(parser.count() == 4 && parser.proc(arg) == -1);
    }
    for (const auto *value : {"-", "--", "-.", "-1e", "-1e+", "-1.2.3", "-1e2e3"}) {
        CommandLine command{"-n", value};
        command.load(parser);
        bool caught = false;
        try {
            parser.proc(arg);
        } catch (const ArgException<String> &) {
            caught = true;
        }
        CHECK(caught && parser.count() == 0);
    }

    CommandLine cluster{"-ab"};
    cluster.load(parser);
    CHECK(parser.proc(arg) == 'a');
    auto copied = parser;
    Argz<String> assigned;
    assigned = parser;
    auto moved = std::move(copied);
    Argz<String> move_assigned;
    move_assigned = std::move(assigned);
    CHECK(parser.proc(arg) == 'b');
    CHECK(moved.proc(arg) == 'b');
    CHECK(move_assigned.proc(arg) == 'b');

    CommandLine terminated{"--", "-a", "-b"};
    terminated.load(parser);
    CHECK(parser.proc(arg) == '-');
    copied = parser;
    CHECK(copied.proc(arg) == '-' && arg.arg_value == text<String>("-b"));
    copied.reset();
    CHECK(copied.proc(arg) == '-' && arg.arg_value == text<String>("-a"));
    cluster.load(parser); // Reinitialization clears the terminator and cluster state.
    CHECK(parser.proc(arg) == 'a');
    cluster.load(parser);
    CHECK(parser.proc(arg) == 'a');
    parser.initArgs(0, nullptr);
    CHECK(parser.proc(arg) == -1 && parser.count() == 0);
    parser.initArgs(3, nullptr);
    CHECK(parser.proc(arg) == '-' && arg.arg_value.empty());
    CHECK(parser.proc(arg) == '-' && arg.arg_value.empty());
    CHECK(parser.proc(arg) == -1);
    CHECK(parser.lookUpCode(text<String>("number")) == 257);
    parser.addOptionSingle(257, text<String>("replacement"));
    CHECK(parser.lookUpCode(text<String>("number")) == -1);

    std::basic_ostringstream<typename String::value_type> help;
    auto help_parser = options<String>();
    help_parser.help(help);
    CHECK(help.str() == text<String>("-a         first\n-b         second\n-n         number\n--verbose  verbose flag\n--number   long number\n"));
}

struct AllocationControl {
    static inline bool fail = false;
};

template <typename T> struct TestAllocator {
    using value_type = T;
    TestAllocator() = default;
    template <typename U> TestAllocator(const TestAllocator<U> &) {}
    T *allocate(std::size_t count) {
        if (AllocationControl::fail)
            throw std::bad_alloc();
        return std::allocator<T>{}.allocate(count);
    }
    void deallocate(T *pointer, std::size_t count) { std::allocator<T>{}.deallocate(pointer, count); }
    bool operator==(const TestAllocator &) const { return true; }
};

void failed_reinitialization() {
    using String = std::basic_string<char, std::char_traits<char>, TestAllocator<char>>;
    Argz<String> parser;
    CommandLine original{"first", "second"};
    original.load(parser);
    Argument<String> arg;
    CHECK(parser.proc(arg) == '-' && arg.arg_value == "first");
    CommandLine replacement{"a replacement argument large enough to require a string allocation"};
    AllocationControl::fail = true;
    bool caught = false;
    try {
        replacement.load(parser);
    } catch (const std::bad_alloc &) {
        caught = true;
    }
    AllocationControl::fail = false;
    CHECK(caught);
    CHECK(parser.count() == 1);
    CHECK(parser.proc(arg) == '-' && arg.arg_value == "second");
    CHECK(parser.proc(arg) == -1);
    parser.reset();
    CHECK(parser.proc(arg) == '-' && arg.arg_value == "first");
}

void common_options() {
    for (bool long_options : {false, true}) {
        CommandLine command{long_options ? "--path" : "-p", "assets", long_options ? "--resolution" : "-r", "1920x1080", long_options ? "--fullscreen" : "-f", "--filename=input.dat", "--texture=texture.png", long_options ? "--shader-path" : "-S", "shaders", "--camera", "2", "--resource", "resource.dat", "--resource_path", "resources"};
        int argc = static_cast<int>(command.argv.size());
        const auto args = proc_args(argc, command.argv.data());
        CHECK(argc == static_cast<int>(command.argv.size()));
        CHECK(args.path == "assets" && args.width == 1920 && args.height == 1080);
        CHECK(args.fullscreen && args.filename == "input.dat" && args.texture == "texture.png");
        CHECK(args.shaderPath == "shaders" && args.camera_index == 2);
        CHECK(args.resource == "resource.dat" && args.resource_path == "resources");
    }
}

void original_examples() {
    char program[] = "argz";
    char flags[] = "-ab";
    char short_value[] = "-ooutput.txt";
    char long_value[] = "--input=file.txt";
    char negative[] = "-2";
    char separator[] = "--";
    char positional[] = "--literal";
    char *argv[] = {program, flags, short_value, long_value, separator, negative, positional};
    Argz<std::string> parser(7, argv);
    parser.addOptionSingle('a', "first").addOptionSingle('b', "second").addOptionSingleValue('o', "output").addOptionDoubleValue('i', "input", "input").addOptionSingleValue('n', "number");
    CHECK(parser.lookUpCode("") == -1);

    Argument<std::string> arg;
    CHECK(parser.proc(arg) == 'a' && arg.arg_name == "a");
    CHECK(parser.proc(arg) == 'b' && arg.arg_name == "b");
    CHECK(parser.proc(arg) == 'o' && arg.arg_value == "output.txt");
    CHECK(parser.proc(arg) == 'i' && arg.arg_value == "file.txt");
    CHECK(parser.proc(arg) == '-' && arg.arg_value == "-2");
    CHECK(parser.proc(arg) == '-' && arg.arg_value == "--literal");
    CHECK(parser.proc(arg) == -1);
    parser.reset();
    CHECK(parser.proc(arg) == 'a');

    char missing[] = "--input";
    char next_option[] = "-a";
    char *missing_argv[] = {program, missing, next_option};
    parser.initArgs(3, missing_argv);
    try {
        parser.proc(arg);
        CHECK(false);
    } catch (const ArgException<std::string> &) {
    }
    CHECK(parser.count() == 0);

    char number[] = "-n";
    char number_value[] = "-2.5";
    char *number_argv[] = {program, number, number_value};
    parser.initArgs(3, number_argv);
    CHECK(parser.proc(arg) == 'n' && arg.arg_value == "-2.5");

    char unknown[] = "--unknown";
    char *unknown_argv[] = {program, unknown};
    parser.initArgs(2, unknown_argv);
    try {
        parser.proc(arg);
        CHECK(false);
    } catch (const ArgException<std::string> &) {
    }

    std::ostringstream help;
    parser.help(help);
    CHECK(help.str().find("--input") != std::string::npos);

    Argz<std::wstring> wide;
    wide.addOptionDoubleValue('x', L"example", L"description");
    std::wostringstream wide_help;
    wide.help(wide_help);
    CHECK(wide_help.str().find(L"--example") != std::wstring::npos);
}

int main() {
    original_examples();
    failed_reinitialization();
    common_options();
    parsing_errors<std::string>();
    parsing_errors<std::wstring>();
    parsing_compatibility<std::string>();
    parsing_compatibility<std::wstring>();
}
