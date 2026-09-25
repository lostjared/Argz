#include "argz.hpp"

#include <cassert>
#include <sstream>

int main() {
    char program[] = "argz";
    char flags[] = "-ab";
    char short_value[] = "-ooutput.txt";
    char long_value[] = "--input=file.txt";
    char negative[] = "-2";
    char separator[] = "--";
    char positional[] = "--literal";
    char *argv[] = {program, flags, short_value, long_value, separator, negative, positional};
    Argz<std::string> parser(7, argv);
    parser.addOptionSingle('a', "first")
        .addOptionSingle('b', "second")
        .addOptionSingleValue('o', "output")
        .addOptionDoubleValue('i', "input", "input")
        .addOptionSingleValue('n', "number");
    assert(parser.lookUpCode("") == -1);

    Argument<std::string> arg;
    assert(parser.proc(arg) == 'a' && arg.arg_name == "a");
    assert(parser.proc(arg) == 'b' && arg.arg_name == "b");
    assert(parser.proc(arg) == 'o' && arg.arg_value == "output.txt");
    assert(parser.proc(arg) == 'i' && arg.arg_value == "file.txt");
    assert(parser.proc(arg) == '-' && arg.arg_value == "-2");
    assert(parser.proc(arg) == '-' && arg.arg_value == "--literal");
    assert(parser.proc(arg) == -1);
    parser.reset();
    assert(parser.proc(arg) == 'a');

    char missing[] = "--input";
    char next_option[] = "-a";
    char *missing_argv[] = {program, missing, next_option};
    parser.initArgs(3, missing_argv);
    try {
        parser.proc(arg);
        assert(false);
    } catch (const ArgException<std::string> &) {}
    assert(parser.count() == 0);

    char number[] = "-n";
    char number_value[] = "-2.5";
    char *number_argv[] = {program, number, number_value};
    parser.initArgs(3, number_argv);
    assert(parser.proc(arg) == 'n' && arg.arg_value == "-2.5");

    char unknown[] = "--unknown";
    char *unknown_argv[] = {program, unknown};
    parser.initArgs(2, unknown_argv);
    try {
        parser.proc(arg);
        assert(false);
    } catch (const ArgException<std::string> &) {}

    std::ostringstream help;
    parser.help(help);
    assert(help.str().find("--input") != std::string::npos);

    Argz<std::wstring> wide;
    wide.addOptionDoubleValue('x', L"example", L"description");
    std::wostringstream wide_help;
    wide.help(wide_help);
    assert(wide_help.str().find(L"--example") != std::wstring::npos);
}
