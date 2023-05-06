#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <list>

#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)
#define COMMAND_MAX_CHARACTERS (80)

//#define RUN_LOCAL

class Command {
    // TODO: Add your data members
protected:
    bool m_is_back_ground;
    bool m_is_complex;
    char* m_cmd_line[COMMAND_MAX_ARGS+1];
    int m_desc_len_in_words;
    char m_full_cmd_line[COMMAND_MAX_CHARACTERS+1];
public:
  Command(const char* cmd_line);
  virtual ~Command(); // TODO: delete m_cmd_line
  virtual void execute() = 0;
  int setCMDLine_R_BG_s(const char* cmd_line);
  //virtual void prepare();
  //virtual void cleanup();

  // TODO: Add your extra methods if needed
};

class BuiltInCommand : public Command {
 public:
  BuiltInCommand(const char* cmd_line);
  char* const *getMCmdLine() const;

    int getMDescLenInWords() const;

  virtual ~BuiltInCommand() = default;
};

class ExternalCommand : public Command {
public:
  ExternalCommand(const char* cmd_line);
  virtual ~ExternalCommand() = default;
  void execute() override;
};

class PipeCommand : public Command {
  // TODO: Add your data members
  Command* m_write_cmd;
  Command* m_read_cmd;
  bool m_error;
 public:
  PipeCommand(const char* cmd_line, int separate);
  virtual ~PipeCommand();
  void execute() override;
};

class RedirectionCommand : public Command {
    // TODO: Add your data members
    bool m_append;
    Command* m_cmd;
    char m_path[COMMAND_MAX_CHARACTERS];
 public:
  explicit RedirectionCommand(const char* cmd_line, int separate);
  virtual ~RedirectionCommand();
  void execute() override;
  //void prepare() override;
  //void cleanup() override;
};


class ChangePrompt : public BuiltInCommand{
 public:
    ChangePrompt(const char* cmd_line);
    virtual ~ChangePrompt() = default;
    void execute() override;
    void fillNewPrompt (char* prompt_new);
};

class ChangeDirCommand : public BuiltInCommand {
// TODO: Add your data members public:
public:
  ChangeDirCommand(const char* cmd_line);
  virtual ~ChangeDirCommand() = default;
  void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand {
 public:
  GetCurrDirCommand(const char* cmd_line);
  virtual ~GetCurrDirCommand() = default;
  void execute() override;
};

class ShowPidCommand : public BuiltInCommand {
 public:
  ShowPidCommand(const char* cmd_line);
  virtual ~ShowPidCommand() = default;
  void execute() override;
};

class JobsList;
class QuitCommand : public BuiltInCommand {
// TODO: Add your data members
  bool m_kill;
public:
  QuitCommand(const char* cmd_line);
  virtual ~QuitCommand() = default;
  void execute() override;
};

enum STATE{FOREGROUND, BACKGROUND, STOPPED};

struct Job{
    int m_job_id;
    int m_pid;
    STATE m_state;
    char m_full_cmd_line[COMMAND_MAX_CHARACTERS + 1];
    time_t m_insert_time;

    Job(int job_id, int pid, STATE state, const char* cmd_line);
    ~Job() = default;
    Job(Job const&) = delete;
    void operator=(Job const&) = delete;
    friend std::ostream& operator<<(std::ostream& os, const Job& job);
    void print2() const;
};


class JobsList {
    std::list<Job*> m_list;
public:
  class JobEntry {
   // TODO: Add your data members
  };
 // TODO: Add your data members
 public:
  JobsList() = default;
  ~JobsList();
  int addNewJob(Job* job);
  void printJobsList();
  void killAllJobs();
  void removeFinishedJobs();
  Job* getJobById(int jobId);
  void removeJobById(int jobId);
  Job* getLastJob();
  int getSize() const;
  Job* getLastStoppedJob();
  // TODO: Add extra methods or modify exisitng ones as needed
};

class JobsCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  JobsCommand(const char* cmd_line);
  virtual ~JobsCommand() = default;
  void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  ForegroundCommand(const char* cmd_line);
  virtual ~ForegroundCommand() {}
  void execute() override;
};

class BackgroundCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  BackgroundCommand(const char* cmd_line);
  virtual ~BackgroundCommand() {}
  void execute() override;
};

class TimeoutCommand : public BuiltInCommand {
/* Bonus */
// TODO: Add your data members
 public:
  explicit TimeoutCommand(const char* cmd_line);
  virtual ~TimeoutCommand() {}
  void execute() override;
};

class ChmodCommand : public BuiltInCommand {
  // TODO: Add your data members
 public:
  ChmodCommand(const char* cmd_line);
  virtual ~ChmodCommand() = default;
  void execute() override;
};

class GetFileTypeCommand : public BuiltInCommand {
  // TODO: Add your data members
 public:
  GetFileTypeCommand(const char* cmd_line);
  virtual ~GetFileTypeCommand() = default;
  void execute() override;
};

class SetcoreCommand : public BuiltInCommand {
  // TODO: Add your data members
 public:
  SetcoreCommand(const char* cmd_line);
  virtual ~SetcoreCommand() {}
  void execute() override;
};

class KillCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  KillCommand(const char* cmd_line);
  virtual ~KillCommand() = default;
  void execute() override;
};

class SmallShell {
 private:
  // TODO: Add your data members
  char m_prompt[COMMAND_MAX_CHARACTERS];
  char m_p_lastPWD[COMMAND_ARGS_MAX_LENGTH];
  bool m_did_first_cd;
  JobsList m_job_list;
  Job* m_fg_job;
  bool m_finish;
  SmallShell();
 public:
  Command *CreateCommand(const char* cmd_line);
  SmallShell(SmallShell const&)      = delete; // disable copy ctor
  void operator=(SmallShell const&)  = delete; // disable = operator
  static SmallShell& getInstance() // make SmallShell singleton
  {
    static SmallShell instance; // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
  }
  ~SmallShell();
  void changePrompt (const char* prompt);
  void printPrompt() const {std::cout << m_prompt << "> ";}
  void executeCommand(const char* cmd_line);

    const char *getMPLastPwd() const;
    void setMPLastPwd(char* lastPwd);

    bool isMDidFirstCd() const;

    void setMDidFirstCd(bool mDidFirstCd);

    void setMFinish(bool mFinish);

    bool isMFinish() const;

    JobsList& getMJobList();
    void setFgJob(Job* fg_job);
    Job* getFgJob() const;


    // TODO: add extra methods as needed
};

#endif //SMASH_COMMAND_H_
