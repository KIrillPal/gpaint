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
    CMD_STATUS readLineCommand(TCommand &line);

    bool isFileMask(std::string mask);
    CMD_STATUS executeLs(TCommand &args);
    CMD_STATUS executeCd(TCommand &args);
    CMD_STATUS executeEdit(TCommand &args);
    CMD_STATUS executeSave(TCommand &args);
    CMD_STATUS executeStatus(TCommand &args);
    CMD_STATUS executeUndo(TCommand &args);
    CMD_STATUS executePreview(TCommand &args);
    CMD_STATUS executePat(TCommand &args);
    CMD_STATUS executeSpat(TCommand &args);
    CMD_STATUS transformFile(TPath file_in, TPath file_out);
    void outProgressBar(const char* filename, int result, int total);

    void outHeader(std::string format = "\e[1;32m");
    void previewImage(Image& image);
private:
    std::filesystem::path selected_path;
    std::vector<std::filesystem::path> selected_files;
    std::string selected_format;
    std::string _user_name;
    std::string _host_name;
    std::string _home_dir;

    std::vector<ImageFilter*> selected_filters;
    std::vector<TCommand> selected_filtercmds;

    std::string getUnifiedPath(std::string path);
    std::string getRelativePath(std::string path);
    void parsePathToDirs(std::string path, TCommand& dirs);
    std::string parseMaskToRegex(std::string mask);

    void setFontColor(const char* color_code);
    CMD_STATUS getDirectory(TPath path, std::filesystem::directory_entry& directory);
    CMD_STATUS findFilesByMask(TCommand& dirs, size_t root, std::filesystem::directory_entry& rdir);
    CMD_STATUS checkImageExists(TPath path);

    std::string getHomeDir();
    std::string getCurrentDir();
    std::string getUserName();
    std::string getHostName();

    CMD_STATUS readFilter(TCommand& command);
    CMD_STATUS parseReplaceColor(TCommand& command, ImageFilter*& filter);
    CMD_STATUS parseGauss       (TCommand& command, ImageFilter*& filter);
    CMD_STATUS parseCrop        (TCommand& command, ImageFilter*& filter);
    CMD_STATUS parseResize      (TCommand& command, ImageFilter*& filter);

    CMD_STATUS convertToInt   (std::string input, int& dst);
    CMD_STATUS convertToFloat (std::string input, float& dst);
    CMD_STATUS convertToSize  (std::string input, size_t& dst);
    CMD_STATUS convertToByte  (std::string input, uint8_t& dst);
    std::string replaceAll(std::string str, const std::string& from, const std::string& to);
    bool askToDoOperation();
    CMD_STATUS getDirCommandHelp(TCommand& cmd);
    CMD_STATUS getEditCommandHelp(TCommand& cmd);
    CMD_STATUS getHelp();
};