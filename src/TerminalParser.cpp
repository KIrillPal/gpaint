#include <fstream>
#include <climits>
#include <regex>
#include <unistd.h>
#include <pwd.h>
#include "TerminalParser.h"
#include "gpaint_exception.h"

const char* DEFAULT_FONT_COLOR = "\e[0m";
const char* ERROR_FONT_COLOR   = "\e[1;31m";

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

    if (command[0] == "help" || command[0] == "-help" || command[0] == "--help") {
        return getHelp();
    }

    if (command.size() >= 2 && (command[1] == "-h" || command[1] == "--help")) {
        if (command[0] == "ls" || command[0] == "cd") {
            TPath try_dir = getRelativePath(command[1]);
            if (!exists(try_dir))
                return getDirCommandHelp(command);
        }
        else return getDirCommandHelp(command);
    }

    if (command[0] == "ls")
        return executeLs(command);
    if (command[0] == "cd")
        return executeCd(command);
    if (command[0] == "exit" || command[0] == "q")
        return END;
    if (command[0] == "edit") {
        command.erase(command.begin());
        return executeEdit(command);
    }
    if (isFileMask(command[0])) {
        return executeEdit(command);
    }
    GPAINT_EXCEPTION("unknown command: '%s'", command[0].c_str());
    return FAILED;
}

bool TerminalParser::isFileMask(std::string mask) {
    if (mask.find_first_of("./\\*^$?") != std::string::npos)
        return true;
    return false;
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
    for (auto arg : args) {
        TPath mask = getUnifiedPath(arg);
        TCommand dirs;
        parsePathToDirs(mask, dirs);

        std::filesystem::directory_entry root_entry;
        getDirectory("", root_entry);
        findFilesByMask(dirs, 0, root_entry);
    }

    if (!selected_files.empty()) {
        if (selected_files.size() == 1)
            selected_format = getRelativePath(selected_files[0]);
        else selected_format = std::to_string(selected_files.size()) + " files";

        printf("Now tracking %zu file%s\n", selected_files.size(), selected_files.size() != 1 ? "s" : "");
        return EDIT;
    } else
        printf("Failed to load files. No files were found with mask '%s'\n",
               getRelativePath(args[0]).c_str());
    return OK;
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

    if (path[0] != '/')
        GPAINT_EXCEPTION("Invalid absolute path '%s'\n", abs_path.c_str())

    while (!path.empty() && path.back() == '/')
        path.pop_back();
    return path;
}

std::string TerminalParser::getRelativePath(std::string path) {
    path = getUnifiedPath(path);
    TCommand path_dirs, sp_dirs;
    parsePathToDirs(path, path_dirs);
    parsePathToDirs(selected_path, sp_dirs);
    std::string relative;
    size_t head = 0, both_head = sp_dirs.size();
    for (; head < sp_dirs.size(); ++head) {
        if (path_dirs.size() <= head || path_dirs[head] != sp_dirs[head]) {
            both_head = head;
            break;
        }
    }

    for (; head < sp_dirs.size(); ++head) {
        relative += "../";
    }

    for (size_t i = both_head; i < path_dirs.size(); ++i) {
        relative += path_dirs[i] + '/';
    }
    if (!relative.empty())
        relative.pop_back();
    return relative;
}

void TerminalParser::parsePathToDirs(std::string path, TCommand& dirs) {
    size_t pos = 0, lpos = 0;
    while ((pos = path.find_first_of('/', lpos)) != std::string::npos) {
        if (pos != 0) {
            dirs.push_back(path.substr(lpos, pos - lpos));
        }
        lpos = pos + 1;
    }
    if (path.size() != lpos)
        dirs.push_back(path.substr(lpos, path.size() - lpos));
}

std::string TerminalParser::parseMaskToRegex(std::string mask) {
    mask = replaceAll(mask, ".", "[.]");
    mask = replaceAll(mask, "*", ".*");
    mask = replaceAll(mask, "?", ".");
    return mask;
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
        if (err.value() == 13)
             GPAINT_EXCEPTION("Permission denied: \'%s\'. Error code: %d", path.c_str(), err.value())
        else GPAINT_EXCEPTION("invalid path \'%s\'. Error code: %d", path.c_str(), err.value())
    }

    if (!directory.is_directory())
        GPAINT_EXCEPTION("Path \'%s\' is a file. Error code: 20", path.c_str())

    return OK;
}

CMD_STATUS TerminalParser::findFilesByMask(TCommand& dirs, size_t root, std::filesystem::directory_entry& rdir) {
    std::string rel_path = getRelativePath(rdir.path());
    bool is_in_cwd = rel_path.empty() || rel_path[0] != '.';

    std::filesystem::directory_iterator files(rdir);
    for (auto file : files)
    {
        TPath filename = file.path().filename();
        std::regex filemask(parseMaskToRegex(dirs[root]));
        if (std::regex_match(filename.c_str(), filemask)) {
            if (root == dirs.size() - 1) {
                if (checkImageExists(file.path()) == OK) {
                    if (!is_in_cwd) {
                        printf("Couldn't add file '%s': file is out of working directory\n", getRelativePath(file.path()).c_str());
                        continue;
                    }
                    selected_files.push_back(file.path());
                    printf("Added file '%s'\n", getRelativePath(file.path()).c_str());
                }
            }
            else {
                std::filesystem::directory_entry target_dir;
                try {
                    getDirectory(file.path(), target_dir);
                    findFilesByMask(dirs, root + 1, target_dir);
                } catch (std::runtime_error& ex) {}
            }
        }
    }
    return OK;
}

CMD_STATUS TerminalParser::checkImageExists(TPath path) {
    try {
        Image img_test;
        BMPReader::loadFromFile(path.c_str(), img_test);
    } catch (std::runtime_error &ex) {
        setFontColor(ERROR_FONT_COLOR);
        printf("Failed to load '%s': %s\n", getRelativePath(path).c_str(), ex.what());
        setFontColor(DEFAULT_FONT_COLOR);
        return FAILED;
    }
    return OK;
}


CMD_STATUS TerminalParser::readEditCommand(TCommand &command) {
    if (command.empty())
        return FAILED;

    if (command[0] == "help" || command[0] == "-help" || command[0] == "--help") {
        return getHelp();
    }

    if (command.size() >= 2 && (command[1] == "-h" || command[1] == "--help")) {
        if (command[0] == "save") {
            TPath try_dir = getRelativePath(command[1]);
            if (!exists(try_dir))
                return getEditCommandHelp(command);
        }
        else return getEditCommandHelp(command);
    }

    if (command[0] == "reset") {
        printf("Cleared %zu filters\n", selected_filters.size());
        selected_filters.clear();
        selected_filtercmds.clear();
        return OK;
    }
    if (command[0] == "status") {
        return executeStatus(command);
    }
    if (command[0] == "pat") {
        return executePat(command);
    }
    if (command[0] == "spat") {
        return executeSpat(command);
    }
    if (command[0] == "undo") {
        return executeUndo(command);
    }
    if (command[0] == "preview") {
        return executePreview(command);
    }
    if (command[0] == "save") {
        return executeSave(command);
    }
    if (command[0] == "exit" || command[0] == "q") {
        selected_files.clear();
        selected_filters.clear();
        selected_filtercmds.clear();
        selected_format = selected_path.filename();
        return END;
    }
    if (readFilter(command) == OK)
        return OK;

    GPAINT_EXCEPTION("unknown command: '%s'", command[0].c_str())
    return FAILED;
}

CMD_STATUS TerminalParser::executeStatus(TCommand &args) {
    printf("Added %zu file%s:\n\e[1;37m",
           selected_files.size(), selected_files.size() != 1 ? "s" : "");

    for (auto &file: selected_files) {
        printf("    %s\n", file.c_str());
    }
    setFontColor(DEFAULT_FONT_COLOR);

    printf("Added %zu filter%s:\n\e[1;37m",
           selected_filters.size(), selected_filters.size() != 1 ? "s" : "");
    for (auto &filter_cmd: selected_filtercmds) {
        printf("    ");
        for (auto &arg: filter_cmd)
            printf("%s ", arg.c_str());
        printf("\n");
    }
    setFontColor(DEFAULT_FONT_COLOR);
    return OK;
}

CMD_STATUS TerminalParser::executeUndo(TCommand &args) {
    if (selected_filters.empty()) {
        printf("Nothing to undo. No filters added\n");
        return OK;
    }
    std::string target = selected_filtercmds.back()[0];
    if (args.size() > 1) {
        target = args[1];
    }

    for (int i = selected_filters.size() - 1; i >= 0; --i) {
        if (selected_filtercmds[i][0] == target)
        {
            std::string command;
            for (auto& arg : selected_filtercmds[i])
                command += arg + ' ';
            command.pop_back();

            printf("Filter '%s' was cancelled\n", command.c_str());

            selected_filters.erase(selected_filters.begin() + i);
            selected_filtercmds.erase(selected_filtercmds.begin() + i);
            return OK;
        }
    }
    printf("Nothing to undo. No filters '%s' was found\n", target.c_str());
    return OK;
}

CMD_STATUS TerminalParser::executeSave(TCommand &args) {
    TPath path = "";
    if (args.size() > 1)
         path = getUnifiedPath(args[1]);

    bool is_dir = !path.has_extension();
    if (!is_dir && selected_files.size() > 1)
        GPAINT_EXCEPTION("Destination must be a folder because several files are saving");

    if (!is_dir && path != "" && selected_files.size() == 1) {

        if (path.filename().extension() != ".bmp") {
            GPAINT_EXCEPTION("Invalid extension '%s'. Could save 'bmp' only",
                             path.filename().extension().c_str());
            return FAILED;
        }
    }

    int rewrite_dests = 0;
    int outer_saving  = 0;
    std::vector<TPath> destinations(selected_files.size());
    if (!is_dir)
        destinations[0] = path;
    else
    {
        for (size_t i = 0; i < selected_files.size(); ++i) {
            TPath inner_path = getRelativePath(selected_files[i]);

            if (path != "")
                 destinations[i] = (std::string) path + "/" + (std::string) inner_path;
            else destinations[i] = selected_files[i];

            if (exists(destinations[i]))
                ++rewrite_dests;
        }
    }

    if (rewrite_dests > 0) {
        printf("%zu file%s will be rewritten. Are you sure? Answer [Y/n]",
               selected_files.size(), selected_files.size() != 1 ? "s" : "");
        if (!askToDoOperation()) return OK;
    }

    int transformed = 0;
    for (size_t i = 0; i < selected_files.size(); ++i) {
        TPath destination = destinations[i];
        TPath inner_path = getRelativePath(selected_files[i]);

        std::error_code err;
        std::filesystem::create_directories(destination.parent_path(), err);
        if (!exists(destination.parent_path())) {
            printf("'%s': Process failed. Couldn't create directory %s\n",
                   inner_path.c_str(), destination.parent_path().c_str());
            continue;
        }

        transformed += (transformFile(selected_files[i], destination) == OK);
    }

    printf("Successfully saved %d file%c\n",
           transformed, transformed != 1 ? 's' : ' ');
    return OK;
}

CMD_STATUS TerminalParser::executePreview(TCommand &args) {
    const size_t MAX_SIMPLE_PREVIEW = 5;
    const size_t PREVIEW_WIDTH = 30;

    TPath path = "";
    std::vector<TPath> sources;

    if (args.size() > 1)
         path = getUnifiedPath(args[1]);

    if (path != "") {
        sources.push_back(path);
    }
    else {
        if (selected_files.size() > MAX_SIMPLE_PREVIEW) {
            printf("%zu files will be previewed. Are you sure? Answer [Y/n]", selected_files.size());
            char answer = std::getchar();

            if (answer != 'Y' && answer != 'y') {
                printf("Operation cancelled\n");
                return OK;
            }
        }
        for (auto file : selected_files)
            sources.push_back(file);
    }

    for (auto path : sources)
    {
        if (sources.size() > 1)
            printf("%s:\n", getRelativePath(path).c_str());

        try {
            Image image;
            BMPReader::loadFromFile(path.c_str(), image);
            size_t new_width = std::min(image.GetWidth(), PREVIEW_WIDTH);
            size_t new_height = image.GetHeight() * new_width / image.GetWidth();
            Filters::Resize resize(2 * new_width, new_height);
            resize.transform(image);
            previewImage(image);

        } catch (std::runtime_error &ex) {
            printf("'%s': %s\n", path.c_str(), ex.what());
        }
    }
    return OK;
}

CMD_STATUS TerminalParser::executePat(TCommand &args) {
    if (args.size() == 1)
        GPAINT_EXCEPTION("invalid format. Format: pat <pat file>");

    TPath path = getUnifiedPath(args[1]);
    std::ifstream fin(path);

    if (!fin.is_open())
        GPAINT_EXCEPTION("Failed to load pattern: Couldn't open file '%s'", path.c_str());

    printf("Loading pattern:\n");

    std::string row;
    while (std::getline(fin, row)) {
        TCommand cmd;
        parseStringToCommand(row, cmd);
        try {
            readEditCommand(cmd);
        } catch (std::runtime_error &ex) {
            printf("%s", ERROR_FONT_COLOR);
            printf("%s\n", ex.what());
            printf("%s", DEFAULT_FONT_COLOR);
        }
    }

    printf("Pattern was loaded\n");

    fin.close();
    return OK;
}

CMD_STATUS TerminalParser::executeSpat(TCommand &args) {
    if (args.size() == 1)
    GPAINT_EXCEPTION("invalid format. Format: spat <path to save>");

    TPath path = getUnifiedPath(args[1]);
    std::error_code err;
    std::filesystem::create_directories(path.parent_path(), err);
    std::ofstream fout(path);

    if (!fout.is_open())
        GPAINT_EXCEPTION("Failed to save pattern: Couldn't create file '%s'", path.c_str());

    for (auto filter_cmd : selected_filtercmds) {
        for (auto arg : filter_cmd) {
            fout << arg << ' ';
        }
        fout << '\n';
    }

    printf("Pattern was saved at '%s'\n", path.c_str());
    fout.close();
    return OK;
}

CMD_STATUS TerminalParser::transformFile(TPath file_in, TPath file_out) {
    try {
        Image image;
        size_t transformed = 0, total = selected_filters.size() + 2;
        outProgressBar(file_in.filename().c_str(), transformed, total);

        BMPReader::loadFromFile(file_in.c_str(), image);
        outProgressBar(file_in.filename().c_str(), ++transformed, total);
        for (auto filter : selected_filters)
        {
            filter->transform(image);
            outProgressBar(file_in.filename().c_str(), ++transformed, total);
        }

        BMPReader::saveToFile(file_out.c_str(), image);
        outProgressBar(file_in.filename().c_str(), ++transformed, total);

    } catch (std::runtime_error& ex) {
        printf("\n'%s': Process failed. %s\n", file_out.c_str(), ex.what());
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

void TerminalParser::previewImage(Image& image) {
    size_t width  = image.GetWidth();
    size_t height = image.GetHeight();
    RGBColor** pixels = image.GetPixelArray();

    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            RGBColor& c = pixels[height - y - 1][x];
            printf("\x1b[38;2;%d;%d;%dm█\x1b[0m", c.R, c.G, c.B);
        }
        printf("\n");
    }
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
    else if (command[0] == "gray") {
        filter = new Filters::Gray();
    }
    else if (command[0] == "edges") {
        filter = new Filters::Edges();
    }
    else if (command[0] == "sobel") {
        filter = new Filters::Sobel();
    }
    else if (command[0] == "median") {
        filter = new Filters::Median();
    }
    else if (command[0] == "crop") {
        if (parseCrop(command, filter) == FAILED)
            return OK;
    }
    else if (command[0] == "resize") {
        if (parseResize(command, filter) == FAILED)
            return OK;
    }

    if (filter == nullptr)
        return FAILED;

    selected_filtercmds.push_back(command);
    selected_filters.push_back(filter);
    printf("Added filter '%s'\n", command[0].c_str());
    return OK;
}

CMD_STATUS TerminalParser::readLineCommand(TCommand &line) {
    size_t head = 1;
    TPath destination_path = "";

    if (line.size() < 2)
        GPAINT_EXCEPTION("invalid format. Format: gpaint <path mask> [attributes]")

     if (line[1] == "--help" || line[1] == "-h") {
         return getHelp();
     }

    std::string source = line[head];
    TCommand cmd = {source};
    executeEdit(cmd);
    printf("%s\n", line[head].c_str());

    ++head;

    while (head != line.size()) {
        if (line[head][0] != '-')
            GPAINT_EXCEPTION("invalid attribute '%s' format. Must begin with '-' character.", line[head].c_str());
        cmd = {line[head].erase(0, 1)};
        ++head;

        while (head != line.size() && line[head][0] != '-') {
            cmd.push_back(line[head]);
            ++head;
        }

        if (cmd[0] == "o") {
            if (cmd.size() < 2)
                GPAINT_EXCEPTION("invalid attribute '-o' format. Format: -o <destination file>");
            destination_path = getUnifiedPath(cmd[1]);
            continue;
        }

        readEditCommand(cmd);
    }

    if (destination_path.empty() || destination_path == source) {
        printf("1 file will be rewritten. Are you sure? Answer [Y/n]");
        char answer = std::getchar();

        if (answer == 'Y' || answer == 'y') {
            cmd = {"save", source};
            return executeSave(cmd);
        }
        printf("Operation cancelled\n");
        return OK;
    }

    cmd = {"save", destination_path};
    return executeSave(cmd);
}


CMD_STATUS TerminalParser::parseReplaceColor(TCommand& command, ImageFilter*& filter) {
    if (command.size() < 7)
        GPAINT_EXCEPTION("too few arguments. Format: replace-color <R1> <G1> <B1> <R2> <G2> <B2>")

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
    if (command.size() < 2)
        GPAINT_EXCEPTION("too few arguments. Format: gauss <dispersion> [<matrix-size>]")

    float dispersion;
    if (convertToFloat(command[1], dispersion) == FAILED)
        return FAILED;

    if (dispersion <= 0)
        GPAINT_EXCEPTION("dispersion must be positive value");

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

CMD_STATUS TerminalParser::parseCrop(TCommand& command, ImageFilter*& filter) {
    if (command.size() < 3)
        GPAINT_EXCEPTION("too few arguments. Format: crop <size x> <size y> [<pos x> <pos y>]")

    size_t size_x, size_y, pos_x = 0, pos_y = 0;
    if (convertToSize(command[1], size_x) == FAILED)
        return FAILED;

    if (convertToSize(command[2], size_y) == FAILED) {
        return FAILED;
    }

    if (size_x == 0 || size_y == 0)
        GPAINT_EXCEPTION("size components must be positive");

    if (command.size() >= 5) {
        if (convertToSize(command[3], pos_x) == FAILED)
            return FAILED;
        if (convertToSize(command[3], pos_y) == FAILED)
            return FAILED;
    }

    filter = new Filters::Crop(size_x, size_y, pos_x, pos_y);
    return OK;
}

CMD_STATUS TerminalParser::parseResize(TCommand& command, ImageFilter*& filter) {
    if (command.size() < 3)
    GPAINT_EXCEPTION("too few arguments. Format: resize <size x> <size y>")

    size_t size_x, size_y;
    if (convertToSize(command[1], size_x) == FAILED)
        return FAILED;

    if (convertToSize(command[2], size_y) == FAILED) {
        return FAILED;
    }

    if (size_x == 0 || size_y == 0)
    GPAINT_EXCEPTION("size components must be positive");

    filter = new Filters::Resize(size_x, size_y);
    return OK;
}


CMD_STATUS TerminalParser::convertToInt(std::string input, int &dst) {
    int got = 0;
    if (sscanf(input.c_str(), "%d%n", &dst, &got) == 1 && got == input.size())
        return OK;

    GPAINT_EXCEPTION("argument \"%s\" must be an integer", input.c_str())
    return FAILED;
}

CMD_STATUS TerminalParser::convertToFloat(std::string input, float &dst) {
    int got = 0;
    if (sscanf(input.c_str(), "%f%n", &dst, &got) == 1 && got == input.size())
        return OK;

    GPAINT_EXCEPTION("argument \"%s\" must be a float integer", input.c_str())
    return FAILED;
}

CMD_STATUS TerminalParser::convertToSize(std::string input, size_t &dst) {
    int got = 0;
    int64_t signed_input;
    if (sscanf(input.c_str(), "%zu%n", &signed_input, &got) == 1 && got == input.size()) {
        if (signed_input < 0)
            GPAINT_EXCEPTION("argument \"%s\" must be positive", input.c_str());
        dst = signed_input;
        return OK;
    }

    GPAINT_EXCEPTION("argument \"%s\" must be a size type", input.c_str())
    return FAILED;
}

CMD_STATUS TerminalParser::convertToByte(std::string input, uint8_t &dst) {
    int got = 0;
    if (sscanf(input.c_str(), "%hhu%n", &dst, &got) == 1 && got == input.size())
        return OK;

    GPAINT_EXCEPTION("argument \"%s\" must have a byte type (0...255)", input.c_str())
    return FAILED;
}

std::string TerminalParser::replaceAll(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
    return str;
}

bool TerminalParser::askToDoOperation() {
    char answer = std::getchar();

    if (answer != 'Y' && answer != 'y') {
        printf("Operation cancelled\n");
        return false;
    }
    return true;
}
