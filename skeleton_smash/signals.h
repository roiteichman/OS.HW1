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
    AlarmList();
public:
    ~AlarmList() = default;
    AlarmList(AlarmList const&)      = delete; // disable copy ctor
    void operator=(AlarmList const&)  = delete; // disable = operator
    static AlarmList& getInstance() // make AlarmList singleton
    {
        static AlarmList instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }
    void addProcess(Job* job, unsigned int time);
    void removeAlarmedProcess();
};

struct AlarmList::TimeOutProcess{
    Job* m_job;
    unsigned int m_time;
    TimeOutProcess(Job* job, unsigned int time) :
            m_job(job), m_time(time){};
};


#endif //SMASH__SIGNALS_H_
