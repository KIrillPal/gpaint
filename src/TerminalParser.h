#pragma once
#include <cstdio>
#include <vector>
#include <string>
#include <filesystem>
#include "Filters/gpaint_filters.h"
#include "BMP.h"

typedef std::vector<std::string> TCommand;
typedef std::filesystem::path TPath;

enum CMD_STATUS {
    OK     = 0,
    FAILED = 1,
    END    = -1,
    EDIT   = 2
};

class TerminalParser {
public:
    TerminalParser();
    TerminalParser(const char* start_directory);
    ~TerminalParser();

    void parseStringToCommand(std::string input, TCommand &target);
    void parseArgsToCommand(int argc, const char** argv, TCommand &target);
    CMD_STATUS readDirCommand (TCommand &command);
    CMD_STATUS readEditCommand(TCommand &command);

    CMD_STATUS executeLs(TCommand &args);
    CMD_STATUS executeCd(TCommand &args);
    CMD_STATUS executeEdit(TCommand &args);
    CMD_STATUS executeSave(TCommand &args);
    CMD_STATUS transformFile(TPath file_in, TPath file_out);
    void outProgressBar(const char* filename, int result, int total);

    void outHeader(std::string format = "\e[1;32m");
private:
    std::filesystem::path selected_path;
    std::vector<std::filesystem::path> selected_files;
    std::string selected_format;
    std::string _user_name;
    std::string _host_name;
    std::string _home_dir;

    std::vector<ImageFilter*> selected_filters;
    const char* DEFAULT_FONT_COLOR = "\e[0m";
    const char* ERROR_FONT_COLOR   = "\e[1;31m";

    std::string getUnifiedPath(std::string path);
    void setFontColor(const char* color_code);
    CMD_STATUS getDirectory(TPath path, std::filesystem::directory_entry& directory);

    std::string getHomeDir();
    std::string getCurrentDir();
    std::string getUserName();
    std::string getHostName();

    CMD_STATUS readFilter(TCommand& command);
    CMD_STATUS parseReplaceColor(TCommand& command, ImageFilter*& filter);
    CMD_STATUS parseGauss       (TCommand& command, ImageFilter*& filter);

    CMD_STATUS convertToInt   (std::string input, int& dst);
    CMD_STATUS convertToFloat (std::string input, float& dst);
    CMD_STATUS convertToSize  (std::string input, size_t& dst);
    CMD_STATUS convertToByte  (std::string input, uint8_t& dst);
};