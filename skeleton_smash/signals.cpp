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

void AlarmList::addProcess(Job* job, unsigned int time) {
#ifndef RUN_LOCAL
    if (time == 0) {
        //TODO
        return;
    }
    unsigned int next_alarm = alarm(0);

    if (next_alarm == 0) {
        assert(m_list.size() == 0);
        m_list.push_back(TimeOutProcess(job, 0));
        cout << "alarm in: " << time << " secs" << endl;
        alarm (time);
        return;
    }
    if (time < next_alarm) {
        m_list.begin()->m_time = next_alarm-time;
        m_list.push_front(TimeOutProcess(job, time));
        alarm (time);
        return;
    }
    time -= next_alarm;
    for (list<TimeOutProcess>::iterator it = m_list.begin(); it != m_list.end(); it++) {
        if (time < it->m_time) {
            it->m_time -= time;
            m_list.insert(it,TimeOutProcess(job, time));
            alarm (next_alarm);
            return;
        }
        else time -= it->m_time;
    }
    m_list.push_back(TimeOutProcess(job, time));
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
        kill(m_list.begin()->m_job->m_pid, SIGKILL);
        cout << "smash: " << m_list.begin()->m_job->m_full_cmd_line << " timed out!" << endl;
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
    int res = kill (fg_job_ptr->m_pid, SIGSTOP);
    if (res == -1){
        perror("smash error: kill failed");
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

    int res = kill (fg_job_ptr->m_pid, SIGKILL);
    if (res == -1){
        perror("smash error: kill failed");
    }
    #endif
    SmallShell::getInstance().setFgJob(NULL);
    cout << "smash: process " << fg_job_ptr->m_pid << " was stopped" << endl;
}

void alarmHandler(int sig_num) {
    cout << "smash: got an alarm" << endl;
    AlarmList::getInstance().removeAlarmedProcess();
}


