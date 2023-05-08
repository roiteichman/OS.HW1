#ifndef SMASH__SIGNALS_H_
#define SMASH__SIGNALS_H_

#include "Commands.h"
using namespace std;


void ctrlZHandler(int sig_num);
void ctrlCHandler(int sig_num);
void alarmHandler(int sig_num);


class AlarmList {
    struct TimeOutProcess;
    list<TimeOutProcess> m_list;
    AlarmList()                     = default;
public:
    ~AlarmList()                    = default;
    AlarmList(AlarmList const&)      = delete; // disable copy ctor
    void operator=(AlarmList const&)  = delete; // disable = operator
    static AlarmList& getInstance() // make AlarmList singleton
    {
        static AlarmList instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }
    void addProcess(const char* cmd_line, pid_t pid, unsigned int time);
    void removeAlarmedProcess();
};

struct AlarmList::TimeOutProcess{
    char m_cmd_line[COMMAND_MAX_CHARACTERS];
    pid_t m_pid;
    unsigned int m_time;
    TimeOutProcess(const char* cmd_line, pid_t pid , unsigned int time) : m_pid(pid), m_time(time)
            {
                strcpy(m_cmd_line, cmd_line);
            }
};


#endif //SMASH__SIGNALS_H_
