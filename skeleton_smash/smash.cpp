#include <iostream>
#include <unistd.h>
#include <signal.h>
#include "Commands.h"
#ifndef RUN_LOCAL
#include <sys/wait.h>
#endif
#include "signals.h"

int main(int argc, char* argv[]) {

    #ifndef RUN_LOCAL

    if(signal(SIGTSTP , ctrlZHandler)==SIG_ERR) {
        perror("smash error: failed to set ctrl-Z handler");
    }

    if(signal(SIGINT , ctrlCHandler)==SIG_ERR) {
        perror("smash error: failed to set ctrl-C handler");
    }
    if(signal(SIGALRM , alarmHandler)==SIG_ERR) {
        perror("smash error: failed to set alarm handler");
    }
    #endif

    SmallShell& smash = SmallShell::getInstance();

    while(!smash.isMFinish()) {
        smash.printPrompt();
        std::string cmd_line;
        std::getline(std::cin, cmd_line);
        smash.executeCommand(cmd_line.c_str());
   }

    return 0;
}

