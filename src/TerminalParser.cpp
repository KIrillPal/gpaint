#include <sstream>
#include <climits>
#include <unistd.h>
#include <pwd.h>
#include "TerminalParser.h"
#include "gpaint_exception.h"


TerminalParser::TerminalParser() : TerminalParser(nullptr) {}

TerminalParser::TerminalParser(const char *start_directory) {
    _user_name = getUserName();
    _host_name = getHostName();
    _home_dir  = getHomeDir();

    if (start_directory == nullptr)
         selected_path = getCurrentDir();
    else selected_path = start_directory;
}

void TerminalParser::parseStringToCommand(std::string input, TCommand &target) {
    std::istringstream input_stream(input);
    std::string buffer;
    while (input_stream >> buffer) {
        target.push_back(buffer);
    }
}

void TerminalParser::parseArgsToCommand(int argc, const char** argv, TCommand &target) {
    for (int i = 0; i < argc; ++i) {
        target.push_back(argv[i]);
    }
}

CMD_STATUS TerminalParser::readDirCommand(TCommand &command) {
    if (command.empty())
        return CMD_STATUS::FAILED;

    if (command[0] == "ls")
        return executeLs(command);
    if (command[0] == "exit" || command[0] == "q")
        return CMD_STATUS::END;
    else {
        setFontColor(ERROR_FONT_COLOR);
        printf("unknown command: '%s'\n", command[0].c_str());
        setFontColor(DEFAULT_FONT_COLOR);
    }

    return CMD_STATUS::OK;
}

CMD_STATUS TerminalParser::executeLs(TCommand &args) {
    std::error_code err;
    std::filesystem::directory_iterator files;
    std::filesystem::path path;

    if (args.size() > 1)
         path = getUnifiedPath(args[1]);
    else path = getUnifiedPath(selected_path);

    files = std::filesystem::directory_iterator(path, err);

    if (err.value() != 0) {
        setFontColor(ERROR_FONT_COLOR);

        if (err.value() == 20) {
            printf("Path \'%s\' is a file. Error code: 20\n", path.c_str());
        }
        else printf("invalid path \'%s\'. Error code: %d\n", path.c_str(), err.value());

        setFontColor(DEFAULT_FONT_COLOR);
        return CMD_STATUS::FAILED;
    }

    for (const auto& file : files)
    {
        if (file.is_directory())
            setFontColor("\e[1;34m");
        else if (!file.path().filename().has_extension())
            setFontColor("\e[1;32m");

        printf("%s  ", file.path().filename().c_str());
        setFontColor(DEFAULT_FONT_COLOR);
    }
    printf("\n");
    return CMD_STATUS::OK;
}

std::string TerminalParser::getUnifiedPath(std::string path) {
    if (path.empty())
        return "/";
    if (path[0] == '~')
        path.replace(0, 1, _home_dir);
    else if (path[0] == '.' && (path.size() < 2 || path[1] == '/'))
        path.replace(0, 1, selected_path);
    else if (path[0] != '/')
        path = std::string(selected_path) + '/' + path;
    return path;
}

void TerminalParser::setFontColor(const char* color_code) {
    printf("%s", color_code);
}

std::string TerminalParser::getHomeDir() {
    passwd *pw = getpwuid(getuid());
    return pw->pw_dir;
}

std::string TerminalParser::getCurrentDir() {
    char current_dir[PATH_MAX];
    if (getcwd(current_dir, PATH_MAX) == NULL)
        GPAINT_EXCEPTION("Failed to get current working directory. Code: %d", errno);
    return current_dir;
}

std::string TerminalParser::getUserName() {
    char user_name[LOGIN_NAME_MAX];
    if (getlogin_r(user_name, LOGIN_NAME_MAX) != 0)
        GPAINT_EXCEPTION("Failed to get username. Code: %d", errno);
    return user_name;
}

std::string TerminalParser::getHostName() {
    char host_name[HOST_NAME_MAX];
    if (gethostname(host_name, HOST_NAME_MAX) != 0)
        GPAINT_EXCEPTION("Failed to get username. Code: %d", errno);
    return host_name;
}

void TerminalParser::outHeader() {
    std::string dir_name;
    if (!dir_name.empty() && dir_name.back() == '/')
         dir_name = selected_path.parent_path().filename();
    else dir_name = selected_path.filename();

    setFontColor("\e[1;32m");
    printf("[%s@%s", _user_name.c_str(), _host_name.c_str());
    setFontColor("\e[1;37m");
    printf(" %s", dir_name.c_str());
    setFontColor("\e[1;32m");
    printf("]$ ");
    setFontColor(DEFAULT_FONT_COLOR);
}