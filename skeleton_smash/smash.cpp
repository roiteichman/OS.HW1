#include <iostream>
#include <unistd.h>
//#include <sys/wait.h>
#include <signal.h>
#include "Commands.h"
#include "signals.h"

int main(int argc, char* argv[]) {


    /*if(signal(SIGTSTP , ctrlZHandler)==SIG_ERR) {
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

    const char* cmd_line = "chprompt abcd is the 4th letters in english &";
    ChangePrompt x(cmd_line);
    char sentence[34];
    x.fillNewPrompt(sentence);
    x.execute();

/*
    for (int i = 0; i < 34; ++i) {
        std::cout << sentence[i];
    }
    std::cout << ">" << std::endl;

*/
    ShowPidCommand y(cmd_line);
    y.execute();
    GetCurrDirCommand z(cmd_line);
    z.execute();
    return 0;
}