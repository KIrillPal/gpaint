#include <iostream>
#include "Image.h"
#include "BMP.h"
#include "Filters/gpaint_filters.h"
#include "TerminalParser.h"

const char* DEFAULT_FONT_COLOR_ = "\e[0m";
const char* ERROR_FONT_COLOR_   = "\e[1;31m";

int runLineMode(TerminalParser& parser, int argc, const char** argv) {
    CMD_STATUS status;
    TCommand command;
    try {
        parser.parseArgsToCommand(argc, argv, command);
        status = parser.readLineCommand(command);
    } catch (std::runtime_error& ex) {
        printf("%s", ERROR_FONT_COLOR_);
        printf("%s\n", ex.what());
        printf("%s", DEFAULT_FONT_COLOR_);
        status = FAILED;
    }
    return 0;
}

int runEditorMode(TerminalParser& parser) {
    CMD_STATUS status;

    do {
        parser.outHeader("\033[1;33m");

        TCommand command;
        std::string terminal_input;
        std::getline(std::cin, terminal_input);

        try {
            parser.parseStringToCommand(terminal_input, command);
            status = parser.readEditCommand(command);
        } catch (std::runtime_error& ex) {
            printf("%s", ERROR_FONT_COLOR_);
            printf("%s\n", ex.what());
            printf("%s", DEFAULT_FONT_COLOR_);
            status = FAILED;
        }

    } while (status != CMD_STATUS::END);

    return 0;
}

int runDirectoryMode(TerminalParser& parser) {
    CMD_STATUS status;

    do {
        parser.outHeader();

        TCommand command;
        std::string terminal_input;
        std::getline(std::cin, terminal_input);
        try {
            parser.parseStringToCommand(terminal_input, command);
            status = parser.readDirCommand(command);
        } catch (std::runtime_error& ex) {
            printf("%s", ERROR_FONT_COLOR_);
            printf("%s\n", ex.what());
            printf("%s", DEFAULT_FONT_COLOR_);
            status = FAILED;
        }
        if (status == CMD_STATUS::EDIT)
            runEditorMode(parser);

    } while (status != CMD_STATUS::END);

    return 0;
}


int main(int argc, const char** argv) {
    TerminalParser terminal;
    if (argc == 1) {
        runDirectoryMode(terminal);
    } else {
        runLineMode(terminal, argc, argv);
    }
    return 0;
}