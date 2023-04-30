//
// Created by teich on 23/04/2023.
//

#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <iomanip>
#include "Commands.h"
#ifndef RUN_LOCAL
#include <sys/wait.h>
#endif
#include <unistd.h>
#include <time.h>

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
#define SIGKILL 9


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
        args[i] = (char*)malloc(s.length()+1);
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


void cmdForBash (char** cmd_source, char* dest) {
    while (*cmd_source != NULL) {
        strcpy(dest, *cmd_source);
        dest += strlen(*cmd_source++);
        *dest++ = ' ';
    }
}



// TODO: Add your implementation for classes in Commands.h

SmallShell::SmallShell() :
        m_prompt("smash") {
    char buff[COMMAND_ARGS_MAX_LENGTH] = {0};
    getcwd(buff, COMMAND_ARGS_MAX_LENGTH);

    strcpy(m_p_currPWD, buff);
    strcpy(m_p_lastPWD, buff);
}

SmallShell::~SmallShell() {
// TODO: add your implementation
}


void copyStr (char* src, char* dest) {
    while (*src != '\0') *dest++ = *src++;
    *dest = '\0';
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
    string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));

    if (firstWord.compare("chprompt") == 0) {
        return new ChangePrompt(cmd_line);
    }

    else if (firstWord.compare("pwd") == 0) {
        return new GetCurrDirCommand(cmd_line);
    }
    else if (firstWord.compare("showpid") == 0) {
        return new ShowPidCommand(cmd_line);
    }
    else if (firstWord.compare("cd") == 0) {
        char lastPWD[COMMAND_ARGS_MAX_LENGTH]={0};
        strcpy(lastPWD, SmallShell::getInstance().getMPLastPwd());
        char * p_lastPwd[COMMAND_ARGS_MAX_LENGTH];
        p_lastPwd[1]=lastPWD;
        return new ChangeDirCommand(cmd_line, p_lastPwd);
    }
    else if (firstWord.compare("jobs") == 0) {
        return new JobsCommand(cmd_line);
    }
    else if (firstWord.compare("quit") == 0) {
        return new QuitCommand(cmd_line);
    }
    else if (firstWord.compare("kill") == 0) {
        return new KillCommand(cmd_line);
    }


//  else if ...
//  .....
    else {
        return new ExternalCommand(cmd_line);
    }
    return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
    // TODO: Add your implementation here
    // for example:
    // Command* cmd = CreateCommand(cmd_line);
    // cmd->execute();
    // Please note that you must fork smash process for some commands (e.g., external commands....)


    this->getMJobList().removeFinishedJobs();
    Command* cmd = CreateCommand(cmd_line);
    cmd->execute();
/*
    BuiltInCommand* bi_cmd = dynamic_cast<BuiltInCommand*>(cmd);
    // if the command is Build-In Command, we get a pointer, else we get nullptr
    if (bi_cmd != nullptr){
        // if its Build-In command execute them directly
        bi_cmd->execute();
    }
    else{
        // fork and let your child execute them and wait for him
        pid_t pid = fork();

        if (pid==0){
            cmd->execute();
        }
        else if (pid>0){
            wait(NULL);
        }*/
}

void SmallShell::setMPLastPwd(char *lastPwd) {
    strcpy(m_p_lastPWD, lastPwd);
}

void SmallShell::setMPCurrPwd() {
    getcwd(m_p_currPWD, DIR_MAX_LEN);
}

const char *SmallShell::getMPLastPwd() const {
    return m_p_lastPWD;
}

const char* SmallShell::getMPCurrPwd() const {
    return m_p_currPWD;
}

JobsList &SmallShell::getMJobList(){
    return m_job_list;
}

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

Command::Command(const char *cmd_line): m_is_back_ground(_isBackgroundComamnd(cmd_line)),
                                        m_is_complex(_isComplexCommand(cmd_line)),
                                        m_desc_len_in_words(setCMDLine_R_BG_s(cmd_line))
{
    strcpy(m_full_cmd_line, cmd_line);
}

BuiltInCommand::BuiltInCommand(const char *cmd_line): Command(cmd_line) {}

ExternalCommand::ExternalCommand(const char *cmd_line): Command(cmd_line){}


void ExternalCommand::execute() {
#ifndef RUN_LOCAL
    pid_t pid = fork();
    if (pid==0){
        // child
        setpgrp();
        if (m_is_complex) {
            char cmd_for_bash[COMMAND_MAX_CHARACTERS];
            cmdForBash (m_cmd_line, cmd_for_bash);
            execl("/bin/bash", "bash", "-c", cmd_for_bash, NULL);
            /// TODO: errors
        }
        else {
            execv(m_cmd_line[0], m_cmd_line);
            /// TODO: errors
        }
    }
    else if (pid>0){
        Job* new_job = new Job(-1, pid, FOREGROUND, this->m_full_cmd_line);
        // foreground
        if (!m_is_back_ground){
            wait(NULL);
        }
        // background
        else{
            new_job->m_state=BACKGROUND;
            int index = SmallShell::getInstance().getMJobList().addNewJob(new_job);

            // TODO: wait
        }
    }
    // error in fork
    else{

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
    getcwd(buff, DIR_MAX_LEN); //errors?
    cout << buff << endl;
}


ChangeDirCommand::ChangeDirCommand(const char* cmd_line, char** plastPwd): BuiltInCommand(cmd_line){
    strcpy(m_plastPwd, plastPwd[1]);
}

void ChangeDirCommand::execute() {

    char asked_path[COMMAND_ARGS_MAX_LENGTH];
    char curr_pwd[COMMAND_ARGS_MAX_LENGTH];
    char old_pwd[COMMAND_ARGS_MAX_LENGTH];
    strcpy(asked_path, getMCmdLine()[1]);
    strcpy(curr_pwd, SmallShell::getInstance().getMPCurrPwd());
    strcpy(old_pwd, SmallShell::getInstance().getMPLastPwd());

    char *another_args = BuiltInCommand::getMCmdLine()[ANOTHER_ARGS];
    if (another_args) {
        perror("smash error: cd: too many arguments");
    }

    else if (strcmp(asked_path, "-") == 0){
        if (strcmp(old_pwd, curr_pwd) == 0){
            perror("smash error: cd: OLDPWD not set");
        }
        strcpy(asked_path, old_pwd);
    }



    if (chdir(asked_path) != 0){
        perror("smash error: cd failed");
    }
    else{
        SmallShell::getInstance().setMPCurrPwd();
        SmallShell::getInstance().setMPLastPwd(curr_pwd);
    }
}

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
    m_list.push_back(job);
    return new_index;
}

JobsCommand::JobsCommand(const char* cmd_line): BuiltInCommand(cmd_line){}

void JobsCommand::execute() {
    SmallShell::getInstance().getMJobList().printJobsList();
}


void JobsList::printJobsList() {
    this->removeFinishedJobs();
    for (Job* job: m_list) {
        cout << *job << endl;
    }
}

void JobsList::removeFinishedJobs() {
#ifndef RUN_LOCAL
    std::list<Job *> tmp_list;
    for (Job *job: m_list) {
        pid_t res = waitpid(job->m_pid, NULL, WNOHANG);
        if (res) {
            tmp_list.push_back(job);
        }
    }
    for (Job *job: tmp_list) {
        delete job;
        m_list.remove(job);
    }
#endif
}

Job *JobsList::getJobById(int jobId) {
#ifndef RUN_LOCAL
    this->removeFinishedJobs();
    Job* temp;
    for (Job *job: m_list) {
        if(job->m_job_id == jobId){
            temp=job;
        }
    }
    return temp;
#endif
}

void JobsList::killAllJobs() {
#ifndef RUN_LOCAL
    this->removeFinishedJobs();
    for (Job *job: m_list) {
        kill(job->m_pid, SIGKILL);
    }
#endif
}

Job *JobsList::getLastJob() {
    return *(--(this->m_list.end()));
}

QuitCommand::QuitCommand(const char* cmd_line): BuiltInCommand(cmd_line),
                                                m_kill(false),
                                                m_another_args(false){
    char* other_arg;
    int is_kill = strcmp("kill", this->getMCmdLine()[1]);
    if (this->getMCmdLine()[ANOTHER_ARGS] != NULL){
        m_another_args = true;
    }
    if (is_kill == 0){
        m_kill = true;
    }
}


void QuitCommand::execute() {
    /// TODO: another args should ignored means if exist so still do the func or no?
    if (m_kill){
        Job* last_job = SmallShell::getInstance().getMJobList().getLastJob();
        /// TODO: what should we do if there is no jobs? still print?
        int num;
        if (last_job != nullptr){
            num = last_job->m_job_id;
        }
        else{
            num = 0;
        }
        cout << "smash: sending SIGKILL signal to "<< num << " jobs:" << endl;
        SmallShell::getInstance().getMJobList().printJobsList();
        SmallShell::getInstance().getMJobList().killAllJobs();
    }
    exit(EXIT_SUCCESS);
    /// TODO: check if its could fail
}


KillCommand::KillCommand(const char *cmd_line): BuiltInCommand(cmd_line) {}

void KillCommand::execute() {

    int signal_id;
    int job_id;
    char c_signal_id[27];
    char c_job_id[27];

    // check if there are too many args
    if (this->getMCmdLine()[ANOTHER_ARGS+1] != NULL){
        perror("smash error: kill: invalid arguments");
    }

    // check if there is job_id, too few args
    if (this->getMCmdLine()[ANOTHER_ARGS] == NULL){
        perror("smash error: kill: invalid arguments");
    }
    else{
        //get the job_id in int
        for (int i = 0; this->getMCmdLine()[ANOTHER_ARGS][i]!='\0'; ++i) {
            c_job_id[i]=this->getMCmdLine()[ANOTHER_ARGS][i];
        }
        sscanf(c_job_id, "%d", &job_id);
    }

    // get the signal num in int
    if (this->getMCmdLine()[ANOTHER_ARGS-1] != NULL) {
        for (int i = 1; this->getMCmdLine()[ANOTHER_ARGS-1][i]!='\0'; ++i) {
            c_signal_id[i]=this->getMCmdLine()[ANOTHER_ARGS - 1][i];
        }
        sscanf(c_signal_id, "%d", &signal_id);
    }

    // get the specific job
    Job* job = SmallShell::getInstance().getMJobList().getJobById(job_id);
    if (job != nullptr){
        #ifndef RUN_LOCAL
        kill(job->m_pid, signal_id);
        #endif
    }
    else{
        cerr << "smash error: kill: job-id " << c_job_id << " does not exist" << endl;
    }

}
