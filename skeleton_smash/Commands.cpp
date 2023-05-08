//
// Created by teich on 23/04/2023.
//

#include <iostream>
#include <vector>
#include <sstream>
#include <iomanip>
#include "Commands.h"
#ifndef RUN_LOCAL
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <fcntl.h>
#endif
#include <unistd.h>
#include <time.h>
#include "signals.h"


#include <sys/types.h>
#include <sys/stat.h>

//#include <stdint.h>
//#include <sys/sysmacros.h>



using namespace std;

const std::string WHITESPACE = " \n\r\t\f\v";

#if 0
#define FUNC_ENTRY()  \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

#define SMASH_DEF_NAME "smash"
#define JUST_PROMPT 1
#define ANOTHER_ARGS 2
#define DIR_MAX_LEN 200
#define LONG_REDIRECTION_SIGN 2
#define LONG_PIPE_SIGN 2
#define IN_FD_INDEX 0
#define OUT_FD_INDEX 1
#define ERR_FD_INDEX 2
#define FULL_PERMISSIONS 777
#define OCTAL_BASE 8


string _ltrim(const std::string& s)
{
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string& s)
{
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string& s)
{
    return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char* cmd_line, char** args) {
    FUNC_ENTRY()
    int i = 0;
    std::istringstream iss(_trim(string(cmd_line)).c_str());
    for(std::string s; iss >> s; ) {
        args[i] = new char[(s.length()+1)];
        memset(args[i], 0, s.length()+1);
        strcpy(args[i], s.c_str());
        args[++i] = NULL;
    }
    return i;

    FUNC_EXIT()
}

bool _isBackgroundComamnd(const char* cmd_line) {
    const string str(cmd_line);
    return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char* cmd_line) {
    const string str(cmd_line);
    // find last character other than spaces
    unsigned int idx = str.find_last_not_of(WHITESPACE);
    // if all characters are spaces then return
    if (idx == string::npos) {
        return;
    }
    // if the command line does not end with & then return
    if (cmd_line[idx] != '&') {
        return;
    }
    // replace the & (background sign) with space and then remove all tailing spaces.
    cmd_line[idx] = ' ';
    // truncate the command line string up to the last non-space character
    cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}


bool _isComplexCommand (const char* cmd_line) {
    while (*cmd_line != 0) {
        if (*cmd_line == '*' || *cmd_line == '?') {
            return true;
        }
        cmd_line++;
    }
    return false;
}

int _isRedirection(const char* cmd_line){
    for (int i = 0; cmd_line[i] != '\0' ; ++i) {
        if (cmd_line[i]=='>'){
            return i;
        }
    }
    return -1;
}

int _isPipe(const char* cmd_line){
    for (int i = 0; cmd_line[i] != '\0' ; ++i) {
        if (cmd_line[i]=='|'){
            return i;
        }
    }
    return -1;
}

void cmdForBash (char** cmd_source, char* dest) {
    while (*cmd_source != NULL) {
        strcpy(dest, *cmd_source);
        dest += strlen(*cmd_source++);
        *dest++ = ' ';
    }
}

/*-------------------
SmallShell methods:
--------------------*/

SmallShell::SmallShell() :
    m_fg_job(NULL), m_finish(false), m_did_first_cd(false) {
    strcpy(m_prompt, "smash");
    char buff[COMMAND_ARGS_MAX_LENGTH] = {0};
    char* res = getcwd(buff, COMMAND_ARGS_MAX_LENGTH);
    if (res == NULL){
        perror ("smash error: getcwd failed");
    }
    strcpy(m_p_lastPWD, buff);
}

SmallShell::~SmallShell() {
    if(m_fg_job!=NULL){
        delete m_fg_job;
    }
}


void SmallShell::changePrompt(const char *prompt) {
    int i=0;
    for (; prompt[i] != '\0'; i++) {
        m_prompt[i] = prompt[i];
    }
    m_prompt[i] = '\0';
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command * SmallShell::CreateCommand(const char* cmd_line) {

    string cmd_s = _trim(string(cmd_line));
    if (cmd_s.size() == 0){
        cout << "empty line" << endl;
        return NULL;
    }
    string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));

    if (_isRedirection(cmd_line) >= 0) {
        char temp[COMMAND_MAX_CHARACTERS] = {0};
        strcpy(temp, cmd_line);
        _removeBackgroundSign(temp);
        return new RedirectionCommand(temp, _isRedirection(cmd_line));
    }
    else if (_isPipe(cmd_line) >= 0) {
        return new PipeCommand(cmd_line, _isPipe(cmd_line));
    }
    else if (firstWord.compare("chprompt") == 0 || firstWord.compare("chprompt&") == 0) {
        return new ChangePrompt(cmd_line);
    }

    else if (firstWord.compare("pwd") == 0 || firstWord.compare("pwd&") == 0) {
        return new GetCurrDirCommand(cmd_line);
    }
    else if (firstWord.compare("showpid") == 0 || firstWord.compare("showpid&") == 0) {
        return new ShowPidCommand(cmd_line);
    }
    //TODO: cd& + second word is NULL
    else if (firstWord.compare("cd") == 0 || firstWord.compare("cd&") == 0) {
        return new ChangeDirCommand(cmd_line);
    }
    else if (firstWord.compare("jobs") == 0 || firstWord.compare("jobs&") == 0) {
        return new JobsCommand(cmd_line);
    }
    else if (firstWord.compare("quit") == 0 || firstWord.compare("quit&") == 0) {
        return new QuitCommand(cmd_line);
    }
    else if (firstWord.compare("kill") == 0 || firstWord.compare("kill&") == 0) {
        return new KillCommand(cmd_line);
    }
    else if (firstWord.compare("fg") == 0 || firstWord.compare("fg&") == 0) {
        return new ForegroundCommand(cmd_line);
    }
    else if (firstWord.compare("bg") == 0 || firstWord.compare("bg&") == 0) {
        return new BackgroundCommand(cmd_line);
    }
    else if (firstWord.compare("setcore") == 0 || firstWord.compare("setcore&") == 0) {
        return new SetcoreCommand(cmd_line);
    }
    else if (firstWord.compare("getfiletype") == 0 || firstWord.compare("getfiletype&") == 0) {
        return new GetFileTypeCommand(cmd_line);
    }
    else if (firstWord.compare("chmod") == 0 || firstWord.compare("chmod&") == 0) {
        return new ChmodCommand(cmd_line);
    }
    else if (firstWord.compare("timeout") == 0 || firstWord.compare("timeout&") == 0) {
        return new TimeoutCommand(cmd_line);
    }
    else {
        return new ExternalCommand(cmd_line);
    }
}

void SmallShell::executeCommand(const char *cmd_line) {
    this->getMJobList().removeFinishedJobs();
    Command* cmd = CreateCommand(cmd_line);
    if (cmd == NULL){
        return;
    }
    cmd->execute();
    delete cmd;
}

void SmallShell::setMPLastPwd(char *lastPwd) {
    strcpy(m_p_lastPWD, lastPwd);
}


const char *SmallShell::getMPLastPwd() const {
    return m_p_lastPWD;
}

JobsList &SmallShell::getMJobList(){
    return m_job_list;
}

void SmallShell::setFgJob(Job* fg_job) {
    m_fg_job = fg_job;
}


Job* SmallShell::getFgJob() const {
    return m_fg_job;
}

void SmallShell::setMFinish(bool mFinish) {
    m_finish = mFinish;
}

bool SmallShell::isMFinish() const {
    return m_finish;
}

bool SmallShell::isMDidFirstCd() const {
    return m_did_first_cd;
}

void SmallShell::setMDidFirstCd(bool mDidFirstCd) {
    m_did_first_cd = mDidFirstCd;
}


/*-------------------
Commands:
--------------------*/

int Command::setCMDLine_R_BG_s(const char *cmd_line) {
    char cmd_line_non_const[COMMAND_MAX_CHARACTERS];

    int i = 0;
    for (; cmd_line[i] != '\0'; ++i) {
        cmd_line_non_const[i]=cmd_line[i];
    }

    cmd_line_non_const[i] = '\0';

    _removeBackgroundSign(cmd_line_non_const);

    return _parseCommandLine(cmd_line_non_const, m_cmd_line);
}

void Command::setMFullCmdLine(const char *cmd_line) {
    strcpy(m_full_cmd_line, cmd_line);
}

Command::Command(const char *cmd_line): m_is_back_ground(_isBackgroundComamnd(cmd_line)),
                                        m_is_complex(_isComplexCommand(cmd_line)),
                                        m_desc_len_in_words(setCMDLine_R_BG_s(cmd_line)),
                                        m_alarm_time(-1)
{
    strcpy(m_full_cmd_line, cmd_line);
}

Command::~Command() noexcept {
    for (int i = 0; m_cmd_line[i]!=NULL ; ++i) {
        delete[] m_cmd_line[i];
    }
}

void Command::setMAlarmTime(int mAlarmTime) {
    m_alarm_time = mAlarmTime;
}

BuiltInCommand::BuiltInCommand(const char *cmd_line): Command(cmd_line) {}

ExternalCommand::ExternalCommand(const char *cmd_line): Command(cmd_line){}


void ExternalCommand::execute() {
#ifndef RUN_LOCAL
    pid_t pid = fork();
    if (pid==0){
        // child
        int res = setpgrp();
	    if (res == -1) perror ("smash error: setpgrp failed");

        if (m_is_complex) {
            char cmd_for_bash[COMMAND_MAX_CHARACTERS];
            cmdForBash (m_cmd_line, cmd_for_bash);
            int result = execl("/bin/bash", "bash", "-c", cmd_for_bash, NULL);
            if (result == -1){
                perror("smash error: execl failed");
                //exit(EXIT_FAILURE);
                SmallShell::getInstance().setMFinish(true);
            }
        }
        else {
            int res = execvp(m_cmd_line[0], m_cmd_line);
            if (res == -1){
                perror("smash error: execvp failed");
                //exit(EXIT_FAILURE);
                SmallShell::getInstance().setMFinish(true);
            }
        }
    }
    else if (pid>0){
        Job* new_job = new Job(-1, pid, FOREGROUND, this->m_full_cmd_line);

        if (m_alarm_time>=0){
            AlarmList::getInstance().addProcess(new_job->m_full_cmd_line, new_job->m_pid, m_alarm_time);
        }

        // foreground
        if (!m_is_back_ground){
            SmallShell::getInstance().setFgJob(new_job);
            int res = waitpid(new_job->m_pid ,NULL, WUNTRACED);
	        if (res == -1){
                perror("smash error: waitpid failed");
            }
	        if (new_job->m_state != STOPPED) {
		        delete new_job; // TODO if the job only stopped
		        SmallShell::getInstance().setFgJob(NULL);
	        }

        }
        // background
        else{
            new_job->m_state=BACKGROUND;
            int index = SmallShell::getInstance().getMJobList().addNewJob(new_job);
        }
    }
    else{
        perror ("smash error: fork failed");
    }
#endif
}

char *const *BuiltInCommand::getMCmdLine() const {
    return m_cmd_line;
}

int BuiltInCommand::getMDescLenInWords() const {
    return m_desc_len_in_words;
}

ChangePrompt::ChangePrompt(const char *cmd_line): BuiltInCommand(cmd_line) {}

void ChangePrompt::fillNewPrompt(char* prompt_new) {

    // from the second word copy letter by letter to new sentence just the first word after it
    int first_word_after_command=1;
    // width is the num of words in the prompt include the Command
    // that is why we start from index 1 to jump over the Command
    int last=0;
    while (!last){
        int j = 0;
        int length = BuiltInCommand::getMDescLenInWords();
        if (length != JUST_PROMPT){
            for (; BuiltInCommand::getMCmdLine()[first_word_after_command][j] != '\0' ; ++j) {
                prompt_new[j] = BuiltInCommand::getMCmdLine()[first_word_after_command][j];

            }
            prompt_new[j] = ' ';
            last=j;
        }
        else{
            break;
        }
    }
    prompt_new[last] = '\0';

}



void ChangePrompt::execute() {
    char prompt_new[COMMAND_MAX_CHARACTERS];
    fillNewPrompt(prompt_new);
    if (prompt_new[0] == '\0') {
        SmallShell::getInstance().changePrompt(SMASH_DEF_NAME);
    }
    else
        SmallShell::getInstance().changePrompt(prompt_new);
}


ShowPidCommand::ShowPidCommand(const char *cmd_line): BuiltInCommand(cmd_line) {}

void ShowPidCommand::execute() {
    cout << "smash pid is " << getpid() << endl;
}


GetCurrDirCommand::GetCurrDirCommand(const char *cmd_line): BuiltInCommand(cmd_line) {}

void GetCurrDirCommand::execute() {
    char buff[DIR_MAX_LEN] = {0};
    char* res = getcwd(buff, DIR_MAX_LEN);
    if (res == NULL) perror ("smash error: getcwd failed");
    cout << buff << endl;
}


ChangeDirCommand::ChangeDirCommand(const char* cmd_line): BuiltInCommand(cmd_line){
}

void ChangeDirCommand::execute() {

    if (m_cmd_line[1] == NULL ||  strcmp(m_cmd_line[1], "-") && m_cmd_line[ANOTHER_ARGS] != NULL
        || m_cmd_line[1] != NULL && m_cmd_line[ANOTHER_ARGS] != NULL ){
        cerr << "smash error: cd: invalid arguments" << endl;
        return;
    }


    char asked_path[COMMAND_ARGS_MAX_LENGTH];
    char old_pwd[COMMAND_ARGS_MAX_LENGTH];
    strcpy(asked_path, m_cmd_line[1]);
    strcpy(old_pwd, SmallShell::getInstance().getMPLastPwd());


    if (strcmp(asked_path, "-") == 0){
        if (!SmallShell::getInstance().isMDidFirstCd()){
            cerr << "smash error: cd: OLDPWD not set"<< endl;
            return;
        }
        else{
            // mean there was at least one cd command
            strcpy(asked_path, old_pwd);
        }
    }

    int res = chdir(asked_path);
    if (res != 0){
        perror("smash error: cd failed");
        return;
    }
    else{
        //means cd succeed
        SmallShell::getInstance().setMDidFirstCd(true);
        SmallShell::getInstance().setMPLastPwd(old_pwd);
    }

}

/*
 * SPECIAL COMMANDS
 */

RedirectionCommand::RedirectionCommand(const char *cmd_line, int separate): Command(cmd_line),
    m_append(cmd_line[separate+1]=='>'),
    m_cmd(NULL){
    int separate_len = m_append ? LONG_REDIRECTION_SIGN : 1;
    strcpy(m_path,_trim(string(cmd_line+separate+separate_len)).c_str());
    char temp_cmd[COMMAND_MAX_CHARACTERS];
    strcpy(temp_cmd,cmd_line);
    temp_cmd[separate]='\0';
    _removeBackgroundSign(temp_cmd);
    m_cmd = SmallShell::getInstance().CreateCommand(temp_cmd);
}

RedirectionCommand::~RedirectionCommand() noexcept {
    delete m_cmd;
}

void RedirectionCommand::execute() {
    #ifndef RUN_LOCAL
    int new_screen_fd = dup(1);
    if (new_screen_fd == -1){
        perror("smash error: dup failed");
    }
    int res = close(1);
    if (res == -1){
        perror("smash error: close failed");
    }
    int res2 = open(m_path, m_append ? (O_WRONLY | O_CREAT | O_APPEND) : (O_WRONLY | O_CREAT), 0666);
    if (res2 == -1){
        perror("smash error: open failed");
    }
    m_cmd->execute();
    int res3 = dup2(new_screen_fd, 1);
    if (res3 == -1){
        perror("smash error: dup2 failed");
    }
    int res4 = close(new_screen_fd);
    if (res4 == -1){
        perror("smash error: close failed");
    }
    #endif
}

PipeCommand::PipeCommand(const char *cmd_line, int separate): Command(cmd_line),
m_error(cmd_line[separate+1]=='&'), m_read_cmd(NULL), m_write_cmd(NULL){
    int separate_len = m_error ? LONG_PIPE_SIGN : 1;
    char temp_cmd[COMMAND_MAX_CHARACTERS];
    strcpy(temp_cmd,cmd_line);
    temp_cmd[separate]='\0';
    _removeBackgroundSign(temp_cmd);
    _removeBackgroundSign(temp_cmd+separate+separate_len);
    m_write_cmd = SmallShell::getInstance().CreateCommand(temp_cmd);
    m_read_cmd = SmallShell::getInstance().CreateCommand(temp_cmd+separate+separate_len);
}

PipeCommand::~PipeCommand() noexcept {
    delete m_write_cmd;
    delete m_read_cmd;
}

void PipeCommand::execute() {
#ifndef RUN_LOCAL
    int my_pipe[2];
    int res = pipe(my_pipe);
    if (res == -1){
        perror("smash error: pipe failed");
    }

    int out = m_error ? ERR_FD_INDEX : OUT_FD_INDEX;


    int stdOutOrErr = dup(out);
    if (stdOutOrErr == -1){
        perror("smash error: dup failed");
    }

    int res3 = dup2(my_pipe[OUT_FD_INDEX], out);
    if (res3 == -1){
        perror("smash error: dup2 failed");
    }
    m_write_cmd->execute();
    // need to close the entrance to the pipe before make the other cmd
    // else the pipe is open after execv and the other cmd wait for input
    int res4 = close(my_pipe[OUT_FD_INDEX]);
    if (res4 == -1){
        perror("smash error: close failed");
    }
    int res5 = dup2(stdOutOrErr, out);
    if (res5 == -1){
        perror("smash error: dup2 failed");
    }
    int keyboard = dup(IN_FD_INDEX);
    if (keyboard == -1){
        perror("smash error: dup failed");
    }
    int res6 = dup2(my_pipe[IN_FD_INDEX], IN_FD_INDEX);
    if (res6 == -1){
        perror("smash error: dup2 failed");
    }
    m_read_cmd->execute();
    int res7 = dup2(keyboard, IN_FD_INDEX);
    if (res7 == -1){
        perror("smash error: dup2 failed");
    }
    int res8 = close(my_pipe[IN_FD_INDEX]);
    if (res8 == -1){
        perror("smash error: close failed");
    }
#endif
}


/*--------------------
Job struct:
---------------------*/


Job::Job(int job_id, int pid, STATE state, const char* cmd_line): m_job_id(job_id), m_pid(pid),
m_state(state), m_insert_time(time(NULL)) {
    strcpy(m_full_cmd_line, cmd_line);
}

std::ostream& operator<<(ostream& os, const Job& job) {
    os << "[" << job.m_job_id << "] " << job.m_full_cmd_line << " : " << job.m_pid;
    os << " " << (difftime(time(NULL), job.m_insert_time)) << " secs";
    if (job.m_state == STOPPED){
        os << " (stopped)";
    }
    return os;
}

void Job::print2() const {
    cout << this->m_full_cmd_line << " : " << m_pid << endl;
}

void Job::print3() const {
    cout << this->m_pid << ": " << this->m_full_cmd_line << endl;
}

/*--------------------
JobsList class:
---------------------*/


JobsList::~JobsList() {
    for (Job* job: m_list) {
        delete job;
    }
}

int JobsList::addNewJob(Job* job){
    this->removeFinishedJobs();
    int new_index = 1;
    if (m_list.size() > 0){
        new_index = (*(--m_list.end()))->m_job_id+1;
    }
    job->m_job_id=new_index;
    job->m_insert_time=time(NULL);
    m_list.push_back(job);
    return new_index;
}

void JobsList::addOldJob (Job* job) {
    this->removeFinishedJobs();
    job->m_insert_time=time(NULL);
    list<Job*>::iterator it = m_list.begin();
    for (; it != m_list.end() ; it++) {
        if ((*it)->m_job_id > job->m_job_id) break;
    }
    m_list.insert(it, job);
}

JobsCommand::JobsCommand(const char* cmd_line): BuiltInCommand(cmd_line){}

void JobsCommand::execute() {
    SmallShell::getInstance().getMJobList().printJobsListForJobsCommand();
}


void JobsList::printJobsListForJobsCommand() {
    this->removeFinishedJobs();
    for (Job* job: m_list) {
        cout << *job << endl;
    }
}

Job *JobsList::popMinPid() {
    Job* min_pid = m_list.front();
    for (Job* job: m_list) {
        if (job->m_pid < min_pid->m_pid){
            min_pid = job;
        }
    }
    m_list.remove(min_pid);
    return min_pid;
}

void JobsList::removeFinishedJobs() {
#ifndef RUN_LOCAL
    std::list<Job*> tmp_list;
    for (Job* job: m_list) {
        pid_t res = waitpid(job->m_pid, NULL, WNOHANG);
	    if (res == -1){
            perror("smash error: waitpid failed");
        }
        else if (res){
            tmp_list.push_back(job);
        }
    }

    for (Job* job: tmp_list) {
        delete job;
        m_list.remove(job);
    }
#endif
}


Job *JobsList::getJobById(int jobId) {
    Job* temp = NULL;
#ifndef RUN_LOCAL
    this->removeFinishedJobs();
#endif
    for (Job *job: m_list) {
        if(job->m_job_id == jobId){
            temp=job;
        }
    }
    return temp;
}

void JobsList::removeJobById(int job_id) {
    for (Job *job_ptr: m_list) {
        if (job_ptr->m_job_id == job_id) {
            m_list.remove(job_ptr);
            return;
        }
    }
    cerr << "the job by id " << job_id << " is not exist" << endl;
    assert (false);
}

void JobsList::killAllJobs() {
#ifndef RUN_LOCAL
    this->removeFinishedJobs();
    for (Job *job: m_list) {
        kill(job->m_pid, SIGKILL);
    }
#endif
}

Job* JobsList::getLastJob() {
    if (m_list.size() == 0){
        return NULL;
    }
    return *(--m_list.end());
}

int JobsList::getSize() const{
    return m_list.size();
}

Job* JobsList::getLastStoppedJob() {
    for (list<Job*>::iterator it=m_list.end() ; it != m_list.begin(); ) {
        if ((*--it)->m_state == STOPPED) return *it;
    }
    return NULL;
}


/*
 * Quit Command
 */
QuitCommand::QuitCommand(const char* cmd_line): BuiltInCommand(cmd_line),
                                                m_kill(false){
    for (int i = 1; this->getMCmdLine()[i]!=NULL; ++i) {
        if (!strcmp("kill", this->getMCmdLine()[i])){
            m_kill = true;
            break;
        }
    }
}


void QuitCommand::execute() {
    if (m_kill){

        int size = SmallShell::getInstance().getMJobList().getSize();

        cout << "smash: sending SIGKILL signal to "<< size << " jobs:" << endl;

        SmallShell::getInstance().getMJobList().killAllJobs();

        while (size){
        Job* job = SmallShell::getInstance().getMJobList().popMinPid();
        job->print3();
        size--;
        delete job;
        }
        assert(SmallShell::getInstance().getMJobList().getSize()==0);
    }
    SmallShell::getInstance().setMFinish(true);
}

int _isNum (char* c) {
    if  (*c =='\0'){
        return 0;
    }
    bool sign = false;
    if (*c == '-') {
        sign = true;
        c++;
        if (*c = '\0'){
            return 0;
        }
    }
    for (char* p = c; *p != '\0'; p++) {
        if (*p < '0' || *p > '9'){
            return 0;
        }
    }
    return sign ? -1 : 1;
}

/*
 * kill Command
 */

KillCommand::KillCommand(const char *cmd_line): BuiltInCommand(cmd_line) {}

void KillCommand::execute() {

    int signal_id;
    int job_id;
    Job* job_ptr = NULL;

    // check if there are too many args
    if (this->getMCmdLine()[ANOTHER_ARGS+1] != NULL){
        cerr <<"smash error: kill: invalid arguments" << endl;
        return;
    }

    // check if there is job_id, too few args
    if (this->getMCmdLine()[ANOTHER_ARGS] == NULL || this->getMCmdLine()[1] == NULL){
        cerr <<"smash error: kill: invalid arguments"<< endl;
        return;
    }


    //get the job_id in int

    if (m_cmd_line[1][0]!='-' || !_isNum (m_cmd_line[1]+1) || !_isNum (m_cmd_line[ANOTHER_ARGS])) {
        cerr << "smash error: fg: invalid arguments" << endl;
        return;
    }
    job_id = stoi(string(m_cmd_line[ANOTHER_ARGS]));
    job_ptr = SmallShell::getInstance().getMJobList().getJobById(job_id);
    if (job_ptr == NULL) {
        cerr << "smash error: kill: job-id " << job_id << " does not exist" << endl;
        return;
    }
    signal_id = stoi(string(m_cmd_line[1]+1));


    #ifndef RUN_LOCAL
    int res = kill(job_ptr->m_pid, signal_id);
    if (res==-1){
        perror("smash error: kill failed");
    }
    else{
        if (signal_id == SIGSTOP || signal_id == SIGTSTP){
            job_ptr->m_state=STOPPED;
        }
        else if(signal_id == SIGCONT){
            job_ptr->m_state=BACKGROUND;
        }
    }
    #endif
}

ForegroundCommand::ForegroundCommand(const char *cmd_line): BuiltInCommand(cmd_line) {}



void ForegroundCommand::execute() {
    Job* job_ptr = NULL;
    if (m_cmd_line[1] == NULL) {
        job_ptr = SmallShell::getInstance().getMJobList().getLastJob();
        if (job_ptr == NULL) {
            cerr << "smash error: fg: jobs list is empty" << endl;
            return;
        }
    }
    else {
        if (m_cmd_line[ANOTHER_ARGS] != NULL || !_isNum (m_cmd_line[1])) {
            cerr << "smash error: fg: invalid arguments" << endl;
            return;
        }
        int job_id = stoi(string(m_cmd_line[1]));
        job_ptr = SmallShell::getInstance().getMJobList().getJobById(job_id);
        if (job_ptr == NULL) {
            cerr << "smash error: fg: job-id " << job_id << " does not exist" << endl;
            return;
        }
    }
    SmallShell::getInstance().getMJobList().removeJobById(job_ptr->m_job_id);
    assert(SmallShell::getInstance().getFgJob() == NULL);
    SmallShell::getInstance().setFgJob(job_ptr);
    job_ptr->print2();
    #ifndef RUN_LOCAL
    if (job_ptr->m_state == STOPPED) {
        int res = kill (job_ptr->m_pid, SIGCONT);
        if (res == -1){
            perror("smash error: kill failed");
        }
    }
    int res = waitpid(job_ptr->m_pid, NULL, WUNTRACED);
    if (res == -1){
        perror("smash error: waitpid failed");
    }
    #endif
    if (job_ptr->m_state != STOPPED) {
        assert(job_ptr->m_state == FOREGROUND);
        delete job_ptr;
        SmallShell::getInstance().setFgJob(NULL);
    }
}



BackgroundCommand::BackgroundCommand(const char *cmd_line): BuiltInCommand(cmd_line) {}


void BackgroundCommand::execute() {
    Job* job_ptr = NULL;
    if (m_cmd_line[1] == NULL) {
        job_ptr = SmallShell::getInstance().getMJobList().getLastStoppedJob();
        if (job_ptr == NULL) {
            cerr << "smash error: bg: there is no stopped jobs to resume" << endl;
            return;
        }
    }
    else {
        if (m_cmd_line[2] != NULL || !_isNum (m_cmd_line[1])) {
            cerr << "smash error: bg: invalid arguments" << endl;
            return;
        }
        int job_id = stoi(string(m_cmd_line[1]));
        job_ptr = SmallShell::getInstance().getMJobList().getJobById(job_id);
        if (job_ptr == NULL) {
            cerr << "smash error: bg: job-id " << job_id << " does not exist" << endl;
            return;
        }
        if (job_ptr->m_state != STOPPED) {
            assert (job_ptr->m_state == BACKGROUND);
            cerr << "smash error: bg: job-id " << job_id << " is already running in the background" << endl;
            return;
        }
    }
    job_ptr->print2();
    job_ptr->m_state = BACKGROUND;
    #ifndef RUN_LOCAL
    int res = kill (job_ptr->m_pid, SIGCONT);
    if (res == -1) perror("smash error: kill failed");
    #endif
}

SetcoreCommand::SetcoreCommand(const char *cmd_line): BuiltInCommand(cmd_line) {}

void SetcoreCommand::execute() {
    if (m_cmd_line[ANOTHER_ARGS+1] != NULL || m_cmd_line[1] == NULL || m_cmd_line[ANOTHER_ARGS] == NULL) {
        cerr << "smash error: setcore: invalid arguments" << endl;
        return;
    }
    if (!_isNum (m_cmd_line[1]) || !_isNum (m_cmd_line[ANOTHER_ARGS])) {
        cerr << "smash error: setcore: invalid arguments" << endl;
        return;
    }
    int job_id = stoi(string(m_cmd_line[1]));
    int core_num = stoi(string(m_cmd_line[2]));
    Job* job_ptr = SmallShell::getInstance().getMJobList().getJobById(job_id);
    if (job_ptr == NULL) {
        cerr << "smash error: setcore: job-id " << job_id << " does not exist" << endl;
        return;
    }
#ifndef RUN_LOCAL
    if (core_num < 0 || core_num >= get_nprocs_conf()) {
        cerr << "smash error: setcore: invalid core number" << endl;
        return;
    }
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(core_num,&set);
    int res = sched_setaffinity(job_ptr->m_pid, sizeof(set), &set);
    if (res == -1){
        perror("smash error: sched_setaffinity failed");
    }
#endif
}

GetFileTypeCommand::GetFileTypeCommand(const char *cmd_line): BuiltInCommand(cmd_line) {}

void printType (const struct stat& sb) {
    switch (sb.st_mode & S_IFMT) {
        case S_IFREG:  cout << "regular file";              break;
        case S_IFDIR:  cout << "directory";                 break;
        case S_IFCHR:  cout << "character device";          break;
        case S_IFBLK:  cout << "block device";              break;
        case S_IFIFO:  cout << "FIFO";                      break;
#ifndef RUN_LOCAL
        case S_IFLNK:  cout << "symbolic link";             break;
        case S_IFSOCK: cout << "socket";                    break;
#endif
        default:       cout << "unknown?\n";                break;
    }
}

void GetFileTypeCommand::execute() {
    if (m_cmd_line[1] == NULL || m_cmd_line[ANOTHER_ARGS] != NULL) {
        cerr << "smash error: gettype: invalid arguments" << endl;
        return;
    }
    struct stat sb;

    int res = stat(m_cmd_line[1], &sb);

    if (res == -1){
        perror("smash error: stat failed");
    }

    cout << m_cmd_line[1] << "'s type is \"";
    printType(sb);
    cout << "\" and takes up " << sb.st_size << " bytes" << endl;
}

ChmodCommand::ChmodCommand(const char *cmd_line): BuiltInCommand(cmd_line) {}

void ChmodCommand::execute() {
    if (m_cmd_line[1] == NULL || !_isNum(m_cmd_line[1])
        || m_cmd_line[ANOTHER_ARGS] == NULL || m_cmd_line[ANOTHER_ARGS + 1] != NULL
        || _isNum(m_cmd_line[1]) && stoi(m_cmd_line[1]) < 0
        || _isNum(m_cmd_line[1]) && stoi(m_cmd_line[1]) > FULL_PERMISSIONS) {
        cerr << "smash error: chmod: invalid arguments" << endl;
        return;
    }

    int numInOctal = 0;
    int hundreds_digit = m_cmd_line[1][0]-'0';
    int tens_digit = m_cmd_line[1][1]-'0';
    int unity_digit = m_cmd_line[1][2]-'0';

    numInOctal += unity_digit;
    numInOctal += tens_digit*OCTAL_BASE;
    numInOctal += hundreds_digit*(OCTAL_BASE*OCTAL_BASE);

    int res = chmod(m_cmd_line[ANOTHER_ARGS], numInOctal);

    if (res == -1){
        perror("smash error: chmod failed");
    }
}

/*
 * Bonus
 */

TimeoutCommand::TimeoutCommand(const char *cmd_line):BuiltInCommand(cmd_line), m_cmd(NULL), m_sec(-1) {
    if(_isNum(m_cmd_line[1])){
        m_sec=stoi(string(m_cmd_line[1]));
    }
    if (m_cmd_line[ANOTHER_ARGS] == NULL){
        return;
    }

    string cmd;
    for (int i = ANOTHER_ARGS; m_cmd_line[i] != NULL; i++) {
        cmd += " ";
        cmd += m_cmd_line[i];
    }
    if (m_is_back_ground){
        cmd += "&";
    }
    m_cmd = SmallShell::getInstance().CreateCommand(cmd.c_str());
    m_cmd->setMFullCmdLine(this->m_full_cmd_line);
}

void TimeoutCommand::execute() {
    if (m_sec < 0 || m_cmd == NULL){
        cerr << "smash error: timeout: invalid arguments" << endl;
        return;
    }
    m_cmd->setMAlarmTime(m_sec);

    ExternalCommand* tmp = dynamic_cast<ExternalCommand*>(m_cmd);
    if (m_cmd == nullptr){
     // Build_In Command
        AlarmList::getInstance().addProcess(m_full_cmd_line, -1, m_sec);
        // TODO we need this?:
        //if (m_sec == 0) {
        //    return;
        //}
    }
    m_cmd->execute();

}