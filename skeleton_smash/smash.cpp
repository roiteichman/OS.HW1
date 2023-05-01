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
    #endif

    //TODO: setup sig alarm handler


    SmallShell& smash = SmallShell::getInstance();


    //while(true) {
        smash.printPrompt();
        std::string cmd_line;
        std::getline(std::cin, cmd_line);
        smash.executeCommand(cmd_line.c_str());
        if (smash.isMFinish()){
            exit(EXIT_SUCCESS);
        }
   //}



    return 0;
}

