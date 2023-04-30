#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"
#include <sys/wait.h>

using namespace std;

void ctrlZHandler(int sig_num) {
    cout << "smash: got ctrl-Z" << endl;
    Job* fg_job_ptr = SmallShell::getInstance().getFgJob();
    if (fg_job_ptr == NULL) return;
    int res = kill (fg_job_ptr->m_pid, SIGSTOP);
    if (res == -1){
        perror("smash error: kill failed");
    }
    SmallShell::getInstance().setFgJob(NULL);
    fg_job_ptr->m_state = STOPPED;
    SmallShell::getInstance().getMJobList().addNewJob(fg_job_ptr);
    cout << "smash: process " << fg_job_ptr->m_pid << " was stopped" << endl;
}

void ctrlCHandler(int sig_num) {
    cout << "smash: got ctrl-C" << endl;
    Job* fg_job_ptr = SmallShell::getInstance().getFgJob();
    if (fg_job_ptr == NULL) return;
    int res = kill (fg_job_ptr->m_pid, SIGKILL);
    if (res == -1){
        perror("smash error: kill failed");
    }
    SmallShell::getInstance().setFgJob(NULL);
    cout << "smash: process " << fg_job_ptr->m_pid << " was stopped" << endl;
    delete fg_job_ptr;
}

void alarmHandler(int sig_num) {
    // TODO: Add your implementation
}

