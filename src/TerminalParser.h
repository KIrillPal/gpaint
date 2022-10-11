#pragma once
#include <cstdio>
#include <vector>
#include <string>
#include <filesystem>
#include "Filters/gpaint_filters.h"

typedef std::vector<std::string> TCommand;

enum CMD_STATUS {
    OK     = 0,
    FAILED = 1,
    END    = -1
};

class TerminalParser {
public:
    TerminalParser();
    TerminalParser(const char* start_directory);

    void parseStringToCommand(std::string input, TCommand &target);
    void parseArgsToCommand(int argc, const char** argv, TCommand &target);
    CMD_STATUS readDirCommand(TCommand &command);

    CMD_STATUS executeLs(TCommand &args);
    CMD_STATUS executeCd(TCommand &args);

    void outHeader();

private:
    std::filesystem::path selected_path;
    std::string _user_name;
    std::string _host_name;
    std::string _home_dir;
    const char* DEFAULT_FONT_COLOR = "\e[0m";
    const char* ERROR_FONT_COLOR   = "\e[1;31m";

    std::string getUnifiedPath(std::string path);
    void setFontColor(const char* color_code);
    CMD_STATUS getDirectory(std::filesystem::path path, std::filesystem::directory_entry& directory);

    std::string getHomeDir();
    std::string getCurrentDir();
    std::string getUserName();
    std::string getHostName();
};