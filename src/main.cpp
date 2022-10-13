#include <iostream>
#include "Image.h"
#include "BMP.h"
#include "Filters/gpaint_filters.h"
#include "TerminalParser.h"

int runLineMode(TerminalParser& parser, const char** argv) {
    return 0;
}

int runEditorMode(TerminalParser& parser) {
    CMD_STATUS status;

    do {
        parser.outHeader("\033[1;33m");

        TCommand command;
        std::string terminal_input;
        std::getline(std::cin, terminal_input);

        parser.parseStringToCommand(terminal_input, command);
        status = parser.readEditCommand(command);

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

        parser.parseStringToCommand(terminal_input, command);
        status = parser.readDirCommand(command);
        if (status == CMD_STATUS::EDIT)
            runEditorMode(parser);

    } while (status != CMD_STATUS::END);

    return 0;
}


int main(int argc, const char** argv) {
    TerminalParser terminal;
    runDirectoryMode(terminal);
    return 0;
}