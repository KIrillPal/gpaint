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
         selected_path = getUnifiedPath(getCurrentDir());
    else selected_path = getUnifiedPath(start_directory);

    selected_format = selected_path.filename();
}

TerminalParser::~TerminalParser() {
    for (auto filter : selected_filters) {
        delete filter;
    }
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
        return FAILED;

    if (command[0] == "ls")
        return executeLs(command);
    if (command[0] == "cd")
        return executeCd(command);
    if (command[0] == "exit" || command[0] == "q")
        return END;
    if (command[0].size() >= 5) {
        size_t sz = command[0].size();
        if (command[0].substr(sz - 4, 4) == ".bmp") {
            Image test_img;
            BMPReader::loadFromFile("images/img.bmp", test_img);
            return executeEdit(command);
        }
    }
    setFontColor(ERROR_FONT_COLOR);
    printf("unknown command: '%s'\n", command[0].c_str());
    setFontColor(DEFAULT_FONT_COLOR);
    return FAILED;
}

CMD_STATUS TerminalParser::executeLs(TCommand &args) {
    std::filesystem::directory_entry chosen_dir;
    TPath path;

    if (args.size() > 1)
         path = getUnifiedPath(args[1]);
    else path = getUnifiedPath(selected_path);

    if (getDirectory(path, chosen_dir) == FAILED)
        return FAILED;

    std::filesystem::directory_iterator files(chosen_dir);
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
    return OK;
}

CMD_STATUS TerminalParser::executeCd(TCommand &args) {
    std::filesystem::directory_entry chosen_dir;
    TPath path;

    if (args.size() > 1)
         path = getUnifiedPath(args[1]);
    else path = getUnifiedPath(selected_path);

    if (getDirectory(path, chosen_dir) == FAILED)
        return FAILED;

    selected_path = getUnifiedPath(chosen_dir.path());
    selected_format = selected_path.filename();
    return OK;
}

template<typename T>
struct remove_extend {
    static T v;
};

template<typename T>
struct remove_extend<T[]> {
    static T v;
};

CMD_STATUS TerminalParser::executeEdit(TCommand &args) {
    TPath source = getUnifiedPath(args[0]);
    if (source == selected_path)
        return FAILED;
    try {
        selected_files.push_back(source);
        selected_format = source.filename();
        printf("Added file '%s'\n", source.c_str());

    } catch (std::runtime_error &ex) {
        printf("Failed to load image: %s\n", ex.what());
        return FAILED;
    }
    return EDIT;
}

std::string TerminalParser::getUnifiedPath(std::string path) {
    if (path[0] == '~')
        path.replace(0, 1, _home_dir);
    else if (path[0] == '.' && (path.size() < 2 || path[1] == '/'))
        path.replace(0, 1, selected_path);
    else if (path[0] != '/')
        path = std::string(selected_path) + '/' + path;

    std::string abs_path = path;

    size_t up_pos = path.find("/..");
    while (up_pos != std::string::npos) {
        size_t l_pos = path.find_last_of("/", up_pos - 1);
        up_pos = path.find("/..");
        if (l_pos != std::string::npos) {
            path.erase(l_pos, up_pos - l_pos + 3);
        }
        else break;
        up_pos = path.find("/..");
    }

    size_t sm_pos = path.find("/./");
    while (sm_pos != std::string::npos) {
        path.erase(sm_pos, 2);
        sm_pos = path.find("/./");
    }

    if (path[0] != '/') {
        setFontColor(ERROR_FONT_COLOR);
        printf("Invalid absolute path '%s'\n", abs_path.c_str());
        setFontColor(DEFAULT_FONT_COLOR);
        return selected_path;
    }

    while (!path.empty() && path.back() == '/')
        path.pop_back();
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

void TerminalParser::outHeader(std::string format) {
    std::string dir_name = selected_format;
    if (dir_name.empty())
        dir_name = "/";

    setFontColor(format.c_str());
    printf("[%s@%s", _user_name.c_str(), _host_name.c_str());
    setFontColor("\e[1;37m");
    printf(" %s", selected_format.c_str());
    setFontColor(format.c_str());
    printf("]$ ");
    setFontColor(DEFAULT_FONT_COLOR);
}

CMD_STATUS TerminalParser::getDirectory(TPath path, std::filesystem::directory_entry& directory) {
    std::error_code err;
    if (path.empty())
        path += '/';
    directory = std::filesystem::directory_entry(path, err);

    if (err.value() != 0) {
        setFontColor(ERROR_FONT_COLOR);
        if (err.value() == 13) {
            printf("Permission denied: \'%s\'. Error code: %d\n", path.c_str(), err.value());
        }
        else printf("invalid path \'%s\'. Error code: %d\n", path.c_str(), err.value());

        setFontColor(DEFAULT_FONT_COLOR);
        return FAILED;
    }

    if (!directory.is_directory()) {
        setFontColor(ERROR_FONT_COLOR);
        printf("Path \'%s\' is a file. Error code: 20\n", path.c_str());
        setFontColor(DEFAULT_FONT_COLOR);
        return FAILED;
    }

    return OK;
}




CMD_STATUS TerminalParser::readEditCommand(TCommand &command) {
    if (command.empty())
        return FAILED;

    if (command[0] == "reset") {

    }
    if (command[0] == "fstatus") {

    }
    if (command[0] == "cstatus") {

    }
    if (command[0] == "ldf") {
        
    }
    if (command[0] == "svf") {

    }
    if (command[0] == "cstatus") {

    }
    if (command[0] == "save") {
        return executeSave(command);
    }
    if (command[0] == "exit" || command[0] == "q") {
        selected_files.clear();
        selected_filters.clear();
        selected_format = selected_path.filename();
        return END;
    }
    if (readFilter(command) == OK)
        return OK;

    setFontColor(ERROR_FONT_COLOR);
    printf("unknown command: '%s'\n", command[0].c_str());
    setFontColor(DEFAULT_FONT_COLOR);
    return FAILED;
}

CMD_STATUS TerminalParser::executeSave(TCommand &args) {
    if (args.size() < 2) {
        setFontColor(ERROR_FONT_COLOR);
        printf("too few arguments. Format: save <destination name>\n");
        setFontColor(DEFAULT_FONT_COLOR);
        return FAILED;
    }

    TPath destination = getUnifiedPath(args[1]);
    printf("got %s\n", destination.c_str());
    if (destination == selected_path)
        return FAILED;
    if (destination.filename().extension() != ".bmp") {
        setFontColor(ERROR_FONT_COLOR);
        printf("Invalid extension '%s'. Could save 'bmp' only\n",
               destination.filename().extension().c_str());
        setFontColor(DEFAULT_FONT_COLOR);
        return FAILED;
    }

    int transformed = 0;
    for (auto file : selected_files) {
        transformed += (transformFile(file, destination) == OK);
    }

    printf("Successfully saved %d file%c\n",
           transformed, selected_files.size() > 1 ? 's' : ' ');
    return OK;
}

CMD_STATUS TerminalParser::transformFile(TPath file_in, TPath file_out) {
    try {
        Image image;
        size_t transformed = 0, total = selected_filters.size() + 2;
        outProgressBar(file_in.filename().c_str(), transformed, total);

        BMPReader::loadFromFile(file_in.c_str(), image);
        RGBColor pix = image.GetPixel(0, 0);
        pix = image.GetPixel(425, 639);
        outProgressBar(file_in.filename().c_str(), ++transformed, total);
        for (auto filter : selected_filters)
        {
            filter->transform(image);
            outProgressBar(file_in.filename().c_str(), ++transformed, total);
        }

        BMPReader::saveToFile(file_out.c_str(), image);
        outProgressBar(file_in.filename().c_str(), ++transformed, total);

    } catch (std::runtime_error& ex) {
        printf("\n'%s': Process failed. %s", file_out.c_str(), ex.what());
        return FAILED;
    }
    printf("\n");
    return OK;
}

void TerminalParser::outProgressBar(const char* filename, int result, int total) {
    const size_t TASKBAR_SIZE = 20;
    int percent = result * 100 / total;

    if (result > 0)
        printf("\r");

    printf("'%s': Process %3d%% [\e[1;37m", filename, percent);

    int filled = TASKBAR_SIZE * result / total;

    printf("%s\e[1;0m%s\e[0m]",
           std::string( filled, '|').c_str(),
           std::string( TASKBAR_SIZE - filled, '|' ).c_str()
    );
    fflush(stdout);
}

CMD_STATUS TerminalParser::readFilter(TCommand& command) {

    ImageFilter* filter = nullptr;
    if (command[0] == "negative") {
        filter = new Filters::Negative();
    }
    else if (command[0] == "replace-color") {
        if (parseReplaceColor(command, filter) == FAILED)
            return OK;
    }
    else if (command[0] == "clarify") {
        filter = new Filters::Clarify();
    }
    else if (command[0] == "gauss") {
        if (parseGauss(command, filter) == FAILED)
            return OK;
    }

    if (filter == nullptr)
        return FAILED;

    selected_filters.push_back(filter);
    printf("Added filter '%s'\n", command[0].c_str());
    return OK;
}

CMD_STATUS TerminalParser::parseReplaceColor(TCommand& command, ImageFilter*& filter) {
    if (command.size() < 7) {
        setFontColor(ERROR_FONT_COLOR);
        printf("too few arguments. Format: replace-color <R1> <G1> <B1> <R2> <G2> <B2>");
        setFontColor(DEFAULT_FONT_COLOR);
        return FAILED;
    }
    RGBColor from(RGB::Null);
    RGBColor to  (RGB::Null);
    uint8_t channels[6];
    for (size_t iclr = 0; iclr < 6; ++iclr) {
        if (convertToByte(command[iclr + 1], channels[iclr]) == FAILED)
            return FAILED;
    }

    from.R = channels[0];
    from.G = channels[1];
    from.B = channels[2];
    to.R = channels[0];
    to.G = channels[1];
    to.B = channels[2];

    filter = new Filters::ReplaceColor(from, to);
    return OK;
}

CMD_STATUS TerminalParser::parseGauss(TCommand &command, ImageFilter *&filter) {
    if (command.size() < 2) {
        setFontColor(ERROR_FONT_COLOR);
        printf("too few arguments. Format: gauss <dispersion> [<matrix-size>]");
        setFontColor(DEFAULT_FONT_COLOR);
        return FAILED;
    }
    float dispersion;
    if (convertToFloat(command[1], dispersion) == FAILED)
        return FAILED;

    if (command.size() < 3) {
        filter = new Filters::Gauss(dispersion);
        return OK;
    }

    size_t matrix_size;
    if (convertToSize(command[2], matrix_size) == FAILED)
        return FAILED;

    filter = new Filters::Gauss(dispersion, matrix_size);
    return OK;
}

CMD_STATUS TerminalParser::convertToInt(std::string input, int &dst) {
    int got = 0;
    if (sscanf(input.c_str(), "%d%n", &dst, &got) == 1 && got == input.size())
        return OK;

    setFontColor(ERROR_FONT_COLOR);
    printf("argument \"%s\" must be an integer\n", input.c_str());
    setFontColor(DEFAULT_FONT_COLOR);
    return FAILED;
}

CMD_STATUS TerminalParser::convertToFloat(std::string input, float &dst) {
    int got = 0;
    if (sscanf(input.c_str(), "%f%n", &dst, &got) == 1 && got == input.size())
        return OK;

    setFontColor(ERROR_FONT_COLOR);
    printf("argument \"%s\" must be a float integer\n", input.c_str());
    setFontColor(DEFAULT_FONT_COLOR);
    return FAILED;
}

CMD_STATUS TerminalParser::convertToSize(std::string input, size_t &dst) {
    int got = 0;
    if (sscanf(input.c_str(), "%zu%n", &dst, &got) == 1 && got == input.size())
        return OK;

    setFontColor(ERROR_FONT_COLOR);
    printf("argument \"%s\" must be a size type\n", input.c_str());
    setFontColor(DEFAULT_FONT_COLOR);
    return FAILED;
}

CMD_STATUS TerminalParser::convertToByte(std::string input, uint8_t &dst) {
    int got = 0;
    if (sscanf(input.c_str(), "%hhu%n", &dst, &got) == 1 && got == input.size())
        return OK;

    setFontColor(ERROR_FONT_COLOR);
    printf("argument \"%s\" must have a byte type (0...255)\n", input.c_str());
    setFontColor(DEFAULT_FONT_COLOR);
    return FAILED;
}

