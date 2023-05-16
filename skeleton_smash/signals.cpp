#include <iostream>
#include <signal.h>
#include <unistd.h>
#include "signals.h"
#include "Commands.h"
#include "assert.h"
#ifndef RUN_LOCAL
#include <sys/wait.h>
#endif

using namespace std;

void AlarmList::addProcess(const char* cmd_line, pid_t pid, unsigned int time) {
#ifndef RUN_LOCAL
    if (time == 0) {
        cout << "smash: got an alarm" << endl;
        if (pid > 0) {
            if (kill(pid, SIGKILL) == -1) {
                perror("smash error: kill failed");
                return;
            }
        }
        cout << "smash: " << cmd_line << " timed out!" << endl;
        return;
    }
    unsigned int next_alarm = alarm(0);

    if (next_alarm == 0) {
        assert(m_list.size() == 0);
        m_list.push_back(TimeOutProcess(cmd_line, pid, 0));
        alarm (time);
        return;
    }
    if (time < next_alarm) {
        m_list.begin()->m_time = next_alarm-time;
        m_list.push_front(TimeOutProcess(cmd_line, pid, 0));
        alarm (time);
        return;
    }
    time -= next_alarm;
    for (list<TimeOutProcess>::iterator it = m_list.begin(); it != m_list.end(); it++) {
        if (time < it->m_time) {
            it->m_time -= time;
            m_list.insert(it,TimeOutProcess(cmd_line, pid, time));
            alarm (next_alarm);
            return;
        }
        else time -= it->m_time;
    }
    m_list.push_back(TimeOutProcess(cmd_line, pid, time));
    alarm (next_alarm);
#endif
}

void AlarmList::removeAlarmedProcess() {
    assert(m_list.size() != 0);
#ifndef RUN_LOCAL
    while (m_list.size() > 0) {
        if (m_list.begin()->m_time > 0){
            break;
        }
        SmallShell::getInstance().getMJobList().removeFinishedJobs();
        // wait - to check if the job is running:
        if (waitpid(m_list.begin()->m_pid, NULL, WNOHANG) == 0) {
            if (kill(m_list.begin()->m_pid, SIGKILL) == -1) {
                perror("smash error: kill failed");
            }
            else {
                cout << "smash: " << m_list.begin()->m_cmd_line << " timed out!" << endl;
            }
        }
        m_list.pop_front();
    }
    if (m_list.size() > 0) {
        alarm(m_list.begin()->m_time);
        m_list.begin()->m_time = 0;
    }
#endif
}


void ctrlZHandler(int sig_num) {
    cout << "smash: got ctrl-Z" << endl;
    Job* fg_job_ptr = SmallShell::getInstance().getFgJob();
    if (fg_job_ptr == NULL) return;

    #ifndef RUN_LOCAL
    if (kill (fg_job_ptr->m_pid, SIGSTOP) == -1){
        perror("smash error: kill failed");
        return;
    }
    #endif
    SmallShell::getInstance().setFgJob(NULL);
    fg_job_ptr->m_state = STOPPED;
    if (fg_job_ptr->m_job_id == -1) {
        SmallShell::getInstance().getMJobList().addNewJob(fg_job_ptr);
    }
    else {
        assert (fg_job_ptr->m_job_id > 0);
        SmallShell::getInstance().getMJobList().addOldJob(fg_job_ptr);
    }
    cout << "smash: process " << fg_job_ptr->m_pid << " was stopped" << endl;
}

void ctrlCHandler(int sig_num) {
    cout << "smash: got ctrl-C" << endl;
    Job* fg_job_ptr = SmallShell::getInstance().getFgJob();
    if (fg_job_ptr == NULL) return;
    #ifndef RUN_LOCAL

    if (kill (fg_job_ptr->m_pid, SIGKILL) == -1){
        perror("smash error: kill failed");
        return;
    }
    #endif
    cout << "smash: process " << fg_job_ptr->m_pid << " was killed" << endl;
}

void alarmHandler(int sig_num) {
    cout << "smash: got an alarm" << endl;
    AlarmList::getInstance().removeAlarmedProcess();
}


