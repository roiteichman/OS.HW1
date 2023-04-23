#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "Commands.h"
#include "signals.h"

int main(int argc, char* argv[]) {


    /*
    if(signal(SIGTSTP , ctrlZHandler)==SIG_ERR) {
        perror("smash error: failed to set ctrl-Z handler");
    }
    if(signal(SIGINT , ctrlCHandler)==SIG_ERR) {
        perror("smash error: failed to set ctrl-C handler");
    }
*/
    //TODO: setup sig alarm handler

    SmallShell& smash = SmallShell::getInstance();


    while(true) {
        smash.printPrompt();
        std::string cmd_line;
        std::getline(std::cin, cmd_line);
        smash.executeCommand(cmd_line.c_str());
    }

/*
    const char* cmd_line = "cd C:\\Users\\teich\\";
    char* last_path = "C:\\Users";

    ChangeDirCommand change(cmd_line, &last_path);
    change.execute();
    for (int i = 0; (last_path[i]) ; ++i) {
        std::cout << last_path[i];
    }
*/

/*    const char* cmd_line = "print.exe you are very good!";
    ExternalCommand w(cmd_line)
    w.execute();
*/

    return 0;
}

