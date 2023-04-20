#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"
#include <unistd.h>

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

// TODO: Add your implementation for classes in Commands.h 

SmallShell::SmallShell() :
    m_prompt("smash") {}

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

    // For example:
/*
  else if ...
  .....
  else {
    return new ExternalCommand(cmd_line);
  }
  */
  return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
  // TODO: Add your implementation here
  // for example:
  // Command* cmd = CreateCommand(cmd_line);
  // cmd->execute();
  // Please note that you must fork smash process for some commands (e.g., external commands....)


    Command* cmd = CreateCommand(cmd_line);
    BuiltInCommand* bi_cmd = dynamic_cast<BuiltInCommand*>(cmd);
    // if the command is Build-In Command, we get a pointer, else we get nullptr
    if (bi_cmd != nullptr){
        // if its Build-In command execute them directly
        bi_cmd->execute();
    }
    else{
        /*
        // fork and let your child execute them and wait for him
        pid_t pid = fork();

        if (pid==0){
            cmd->execute();
        }
        else if (pid>0){
            wait(NULL);
        }*/
    }
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

Command::Command(const char *cmd_line): m_is_back_ground(_isBackgroundComamnd(cmd_line)), m_desc_len_in_words(
        setCMDLine_R_BG_s(cmd_line)) {
}

BuiltInCommand::BuiltInCommand(const char *cmd_line): Command(cmd_line) {}

ExternalCommand::ExternalCommand(const char *cmd_line): Command(cmd_line){}

void ExternalCommand::execute() {
    pid_t pid = fork();
    if (pid==0){
        // child
        execv(m_cmd_line[0], m_cmd_line);
    }
    else if (pid>0){
        if (!m_is_back_ground){
            wait(NULL);
        }
    }
    else{

    }
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
        for (; BuiltInCommand::getMCmdLine()[first_word_after_command][j] != '\0' ; ++j) {
            prompt_new[j] = BuiltInCommand::getMCmdLine()[first_word_after_command][j];

        }
        prompt_new[j] = ' ';
        last=j;
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

#define DIR_MAX_LEN 200

void GetCurrDirCommand::execute() {
    char buff[DIR_MAX_LEN] = {0};
    getcwd(buff, DIR_MAX_LEN); //errors?
    cout << buff << endl;
}