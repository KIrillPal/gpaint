#include "TerminalParser.h"
#include "gpaint_exception.h"

CMD_STATUS TerminalParser::getDirCommandHelp(TCommand &cmd) {
    if (cmd[0] == "ls") {
        printf("ls - Outputs list of files and directories existing in directory by argument (or current directory otherwise).\n\n"
               "Usage: ls [directory path]\n\n"
               "Examples:\n"
               "    1. \"ls ../folder\"\n"
               "    2. \"ls\"\n"
               "    3. \"ls /\"\n"
               );
    }
    else if (cmd[0] == "cd") {
        printf("cd - Sets current working directory at path in argument.\n\n"
               "Usage: cd <directory path>\n\n"
               "Examples:\n"
               "    1. \"cd ../folder\"\n"
               "    2. \"cd /home/user/\"\n"
        );
    }
    else if (cmd[0] == "exit" || cmd[0] == "q") {
        printf("Usage: exit\n"
               "Usage: q\n\n"
               "The behaviour of command depends on mode (header color):\n"
               "    1. Directory mode (green): exits entire program.\n"
               "    2. Editor mode (yellow)  : exits editor mode.\n\n"
               "Examples:\n"
               "    1. \"exit\"\n"
               "    2. \"q\"\n"
        );
    }
    else if (cmd[0] == "edit") {
        printf("Usage: edit [file masks]\n"
               "Usage: [file masks]\n\n"
               "Starts editor mode with selected files. Files are selected using arguments - a list of masks.\n\n"
               "File mask: a pattern of bmp file path that program will search to add image.\n"
               "    Pattern may contain symbols '?' - every single character, '*' - every character sequence.\n"
               "    As example, mask \"folder/i*e.?mp\" responds sequences \"folder/image.bmp\" or \"folder/ie.amp\"\n"
               "    but not \"fol/image.bmp\" or \"folder/image.bbmp\".\n"
               "    It's allowed to use mask with folders, as example, \"../*/*.bmp\"\n\n"
               "Examples:\n"
               "    1. \"edit ./image.bmp ./folder-*/?????.bmp\"\n"
               "    2. \"./image.bmp ./folder-*/?????.bmp\" - you can avoid word \"edit\"\n"
               "    3. \"edit *\"\n"
               "    4. \"edit\" - no files will be found\n"
               "    5. \"~/Desktop/all_files/*.bmp\"\n"
               "    6. \"*\"\n"
        );
    }
    else {
        GPAINT_EXCEPTION("unknown command: '%s'", cmd[0].c_str());
        return FAILED;
    }
    return OK;
}

CMD_STATUS TerminalParser::getEditCommandHelp(TCommand &cmd) {
    if (cmd[0] == "exit" || cmd[0] == "q") {
        printf("Usage: exit\n"
               "Usage: q\n\n"
               "The behaviour of command depends on mode (header color):\n"
               "    1. Directory mode (green): exits entire program.\n"
               "    2. Editor mode (yellow)  : exits editor mode.\n\n"
               "Examples:\n"
               "    1. \"exit\"\n"
               "    2. \"q\"\n"
        );
    }
    else if (cmd[0] == "reset") {
        printf("reset - Clears all selected filters.\n"
               "Usage: reset\n"
               "Examples:\n"
               "    1. \"reset\"\n"
        );
    }
    else if (cmd[0] == "status") {
        printf("status - Shows list of tracking files and applied filters.\n"
               "Usage: status\n"
               "Examples:\n"
               "    1. \"status\"\n"
        );
    }
    else if (cmd[0] == "pat") {
        printf("pat - Loads transforming pattern from file and apply.\n\n"
               "Usage: pat <pattern's path>\n"
               "    Pattern is a list of filters that you can apply to the current images with one command - pat.\n"
               "    <pattern's path> - argument contains path of the pattern you want to apply.\n"
               "    You can make such file from your filter list using command \"spat\"\n\n"
               "    Filters are loading from the pattern one-by-one. If the file p.pat contains 3 filters,\n"
               "    3 filters will be applied to the images. If some filters have invalid format, they won't be applied only.\n\n"
               "Examples:\n"
               "    1. \"pat ../patterns/1.pat\"\n"
               "    2. \"pat best-pattern-ever.txt\"\n"
        );
    }
    else if (cmd[0] == "spat") {
        printf("spat - Saves the list of already applied filters to the file.\n\n"
               "Usage: spat <new pattern's path>\n"
               "    Pattern is a list of filters that you can apply to the current images with command \"pat\".\n"
               "    <new pattern's path> - argument contains path of the file you want to save filter list to.\n"
               "    Also you can add pattern to your filter list using command \"pat\"\n\n"
               "Note: If the path contains folders that doesn't exist, they will be created.\n\n"
               "Examples:\n"
               "    1. \"spat ../patterns/1.pat\"\n"
               "    2. \"pat best-pattern-ever.txt\"\n"
        );
    }
    else if (cmd[0] == "undo") {
        printf("undo - Cancels the last applied filter.\n\n"
               "Usage: undo\n"
               "Usage: undo [filter]\n\n"
               "    [filter] - is a name of filter to remove. As example, \"clarify\";\"gauss\";\"negative\"...\n"
               "    If has argument [filter], it would find last applied filter [filter] and remove.\n"
               "    Otherwise removes last applied filter.\n\n"
               "Examples:\n"
               "    1. \"undo\"\n"
               "    2. \"undo resize\"\n"
        );
    }
    else if (cmd[0] == "preview") {
        printf("preview - Previews image(s)\n\n"
               "Usage: preview\n"
               "Usage: preview [applied image path]\n\n"
               "    [applied image path] - is a path to the image you want to preview.\n"
               "    If hasn't argument, tries to preview all the tracking images.\n"
               "    If more than 5 images are tracking, it asks the permission to out all of them.\n\n"
               "Examples:\n"
               "    1. \"preview\"\n"
               "    2. \"preview images/img.bmp\"\n"
        );
    }
    else if (cmd[0] == "save") {
        printf("save - Saves all the tracking images.\n\n"
               "Usage: save\n"
               "Usage: save [destination image]\n"
               "Usage: save [destination folder]\n\n"
               "    The behaviour of the command depends of the amount of tracking images.\n"
               "    1) If one image is selected, it allowed to save to folder or image.\n"
               "    2) If several, it can save only to the folder.\n"
               "        if selected image has relative path ./[rel] and destination folder ./[folder], \n"
               "        new path of changed image is ./[folder]/[rel].\n"
               "    [destination image] - is a path to the image you want to save to. It must have extension 'bmp'.\n\n"
               "    If hasn't argument, tries to save all files with replacing.\n"
               "Note: If destination path contains folders that doesn't exist, they will be created.\n"
               "Note: If some files are going to replace another, asks to continue.\n\n"
               "Examples:\n"
               "1) one selected image\n"
               "    1. \"save\"\n"
               "    2. \"save new-folder/new-image.bmp\"\n"
               "2) several images\n"
               "    1. \"save\"\n"
               "    2. \"save new-folder/\"\n"
        );
    }
    else {
        GPAINT_EXCEPTION("unknown command: '%s'", cmd[0].c_str());
        return FAILED;
    }
    return OK;
}

CMD_STATUS TerminalParser::getHelp() {
    printf("Gpaint - image terminal editor.\n\n"
           "Usage: gpaint\n"
           "Usage: gpaint [arguments]\n\n"
           "If hasn't arguments, it enters editor mode.\n"
           "    The program has 2 basic modes (header color):\n"
           "    1) directory mode(green)\n"
           "    Shows current working directory. Using commands:\n"
           "        ls [folder]          - shows folders in current directory or in the [folder] path by argument.\n"
           "        cd <folder>          - moves current directory to the <folder>\n"
           "        edit [image mask]... - enters editor mode with selected images by mask list.\n"
           "            Full information in \"edit --help\"\n"
           "        [image mask]...      - the same as \"edit [image mask]...\"\n"
           "        exit                 - exits program.\n"
           "        <command> -h | <command> --help - outs help.\n\n"

           "    2) editor mode(yellow)\n"
           "    Shows tracking image (if only one selected) or number of tracking files.\n"
           "    Program is tracking images by its paths, you can delete and modify tracking files outside the program.\n"
           "    Using commands:\n"
           "        reset         - Clears all applied filters.\n"
           "        status        - prints all tracking files and applied filters.\n"
           "        pat <file>    - loads pattern and applies to the images.\n"
           "        spat <file>   - saves applied filters to the pattern.\n"
           "        undo [filter] - Cancels the last filter [filter] or just last filter.\n"
           "        save [path]   - Saves image(s) to the [file/folder].\n"
           "            Full information in \"save -h\"\n"
           "        exit          - exits editor mode.\n"
           "        <command> -h | <command> --help - outs help.\n\n"
    );
    return OK;
}