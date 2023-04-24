#include <iostream>
#include <unistd.h>
#include <signal.h>
#include "Commands.h"
#ifndef RUN_LOCAL
#include <sys/wait.h>
#endif
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

    Job* tmp_1 = new Job(1, 657, BACKGROUND, "cd C:\\Users\\teich");
    Job* tmp_2 = new Job(2, 658, FOREGROUND, "cd C:\\Users");
    Job* tmp_3 = new Job(3, 659, STOPPED, "ls");
    Job* tmp_4 = new Job(4, 660, BACKGROUND, "pwd");

    JobsList jobs_list;


    jobs_list.addNewJob(tmp_1);

    jobs_list.addNewJob(tmp_2);
    jobs_list.addNewJob(tmp_3);
    jobs_list.addNewJob(tmp_4);

    jobs_list.printJobsList();






/*

    SmallShell& smash = SmallShell::getInstance();


    while(true) {
        smash.printPrompt();
        std::string cmd_line;
        std::getline(std::cin, cmd_line);
        smash.executeCommand(cmd_line.c_str());
    }

*/

    return 0;
}

