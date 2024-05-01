# include <vector>
# include <string>
# include <optional>
# include <sstream>
# include <map>
# include <filesystem>
# include <regex>
# include <iostream>
# include <chrono>
# include <boost/filesystem.hpp>
# include <boost/process.hpp>
# include <boost/dll.hpp>
# include <fmt/format.h>
# include <fmt/ranges.h>
# include <zxorm/zxorm.hpp>

using namespace std::literals;

// ssh fingerprint -> username
const std::map<std::string, std::string> Username
{
  { "LNoYfq/SM7l8sFAy325WpC+li+kZl3jwST7TmP72Tz8", "Haonan Chen" },
  { "VJT5wgkb2RcIeVNTA+/NKxokctbYnJ/KgH6IxrKqIGE", "Bin Gong" },
  { "umC3/RB1vS8TQBHsY3IzhOiyqVrOSw2fB3rIpDQSmf4", "Leilei Xiang" },
  { "fdq5k13N2DAzIK/2a1Mm4/ZVsDUgT623TSOXsVswxT8", "Junqi Yao" },
  { "8USxEYi8ePPpLhk5FYBo2udT7/NFmEe8c2+oQajGXzA", "Enming Zhang" },
  { "7bmG24muNsaAZkCy7mQ9Nf2HuNafmvUO+Hf1bId9zts", "Yaping Wu" },
  { "dtx0QxdgFrXn2SYxtIRz43jIAH6rLgJidSdTvuTuews", "Jing Li" },
  { "8crUO9u4JiVqw3COyjXfzZe87s6XZFhvi0LaY0Mv6bg", "Huahan Zhan" },
  { "QkmIYw7rmDEAP+LDWxm6L2/XLnAqTwRUB7B0pxYlOUs", "Na Gao" }
};

// program path, set at start of main, e.g. /gpfs01/.../bin/hpcstat
std::filesystem::path Program;

// run a program, wait until it exit, return its stdout if it return 0, otherwise nullopt
std::optional<std::string> exec(boost::filesystem::path program, std::vector<std::string> args)
{
  namespace bp = boost::process;
  bp::ipstream output;
  auto process = bp::child
    (program, bp::args(args), bp::std_out > output, bp::std_err > stderr, bp::std_in < bp::null);
  process.wait();
  if (process.exit_code() != 0) return std::nullopt;
  std::stringstream ss;
  ss << output.rdbuf();
  return ss.str();
}

// detect ssh fingerprint using ssh-add
// always assume sha256 fingerprint
std::optional<std::vector<std::string>> fingerprints()
{
  auto output =
    exec(Program.replace_filename("ssh-add"), { "-l" });
  if (!output) { std::cerr << "Failed to get ssh fingerprints\n"; return std::nullopt; }
  auto fingerprint = output->substr(0, 47);
  // search for all strings that match the fingerprint pattern: sha256:...
  std::regex pattern(R"r(\b(?:sha|SHA)256:([0-9A-Za-z+/=]{43})\b)r");
  std::smatch match;
  std::vector<std::string> fingerprints;
  for (auto i = std::sregex_iterator(output->begin(), output->end(), pattern); i != std::sregex_iterator(); i++)
    fingerprints.push_back(i->str(1));
  return fingerprints;
}

// get an authenticated fingerprint and username
std::optional<std::pair<std::string, std::string>> authenticated()
{
  auto fps = fingerprints();
  if (!fps) return std::nullopt;
  for (auto& fp : *fps)
    if (Username.contains(fp)) return std::make_pair(fp, Username.at(fp));
  std::cerr << fmt::format("No valid fingerprint found, available fingerprints: {}\n", *fps);
  return std::nullopt;
}

// initialize the database
struct LoginData
{
  unsigned Id = 0; long Time = 0;
  std::string Key, SessionId;
  std::optional<std::string> Subaccount;
  bool Interactive;
};
using LoginTable = zxorm::Table
<
  "login", LoginData,
  zxorm::Column<"id", &LoginData::Id, zxorm::PrimaryKey<>>,
  zxorm::Column<"time", &LoginData::Time>,
  zxorm::Column<"key", &LoginData::Key>,
  zxorm::Column<"session_id", &LoginData::SessionId>,
  zxorm::Column<"sub_account", &LoginData::Subaccount>,
  zxorm::Column<"interactive", &LoginData::Interactive>
>;
struct LogoutData { unsigned Id = 0; long Time = 0; std::string SessionId; };
using LogoutTable = zxorm::Table
<
  "logout", LogoutData,
  zxorm::Column<"id", &LogoutData::Id, zxorm::PrimaryKey<>>,
  zxorm::Column<"time", &LogoutData::Time>, zxorm::Column<"sessionid", &LogoutData::SessionId>
>;
struct SubmitJobData
  { unsigned Id = 0; long Time = 0; int JobId; std::string Key, SessionId, Subaccount, SubmitDir, JobCommand; };
using SubmitJobTable = zxorm::Table
<
  "submitjob", SubmitJobData,
  zxorm::Column<"id", &SubmitJobData::Id, zxorm::PrimaryKey<>>,
  zxorm::Column<"time", &SubmitJobData::Time>,
  zxorm::Column<"job_id", &SubmitJobData::JobId>,
  zxorm::Column<"key", &SubmitJobData::Key>,
  zxorm::Column<"session_id", &SubmitJobData::SessionId>,
  zxorm::Column<"sub_account", &SubmitJobData::Subaccount>,
  zxorm::Column<"submit_dir", &SubmitJobData::SubmitDir>,
  zxorm::Column<"job_command", &SubmitJobData::JobCommand>
>;
struct FinishJobData { unsigned Id = 0; long Time = 0; int JobId; std::string JobResult; double CpuTime; };
using FinishJobTable = zxorm::Table
<
  "finishjob", FinishJobData,
  zxorm::Column<"id", &FinishJobData::Id, zxorm::PrimaryKey<>>,
  zxorm::Column<"time", &FinishJobData::Time>,
  zxorm::Column<"job_id", &FinishJobData::JobId>,
  zxorm::Column<"job_result", &FinishJobData::JobResult>,
  zxorm::Column<"cpu_time", &FinishJobData::CpuTime>
>;
struct QueryJobData { unsigned Id = 0; int JobId; };
using QueryJobTable = zxorm::Table
<
  "queryjob", QueryJobData,
  zxorm::Column<"id", &QueryJobData::Id, zxorm::PrimaryKey<>>, zxorm::Column<"job_id", &QueryJobData::JobId>
>;
void initdb()
{
  auto dbfile = Program.replace_filename("hpcstat.db").string();
  zxorm::Connection<LoginTable, LogoutTable, SubmitJobTable, FinishJobTable, QueryJobTable>
    conn(dbfile.c_str());
  conn.create_tables();
}
void writedb(auto value)
{
  auto dbfile = Program.replace_filename("hpcstat.db").string();
  zxorm::Connection<LoginTable, LogoutTable, SubmitJobTable, FinishJobTable, QueryJobTable>
    conn(dbfile.c_str());
  value.Time = std::chrono::duration_cast<std::chrono::seconds>
    (std::chrono::system_clock::now().time_since_epoch()).count();
  conn.insert_record(value);
}

bool interactive() { return isatty(fileno(stdin)); }

// get value of XDG_SESSION_ID
std::optional<std::string> session_id()
{
  if (auto value = std::getenv("XDG_SESSION_ID"); !value)
    { std::cerr << "Failed to get session id\n"; return std::nullopt; }
  else return value;
}

// get value of HPCSTAT_SUBACCOUNT
std::optional<std::string> subaccount()
  { if (auto value = std::getenv("HPCSTAT_SUBACCOUNT"); value) return value; else return std::nullopt; }

int main(int argc, const char** argv)
{
  std::vector<std::string> args(argv, argv + argc);
  Program = boost::dll::program_location().string();

  if (args.size() == 1) { std::cout << "Usage: hpcstat initdb|login|logout|submitjob|finishjob\n"; return 1; }
  else if (args[1] == "initdb")
    initdb();
  else if (args[1] == "login")
  {
    if (auto key = authenticated(); !key) return 1;
    else if (auto session = session_id(); !session) return 1;
    else writedb(LoginData
      {.Key = key->first, .SessionId = *session, .Subaccount = subaccount(), .Interactive = interactive()});
  }
  else if (args[1] == "logout")
  {
    if (auto session = session_id(); !session) return 1;
    else writedb(LogoutData{.SessionId = *session});
  }
  else if (args[1] == "submitjob")
  {
    if (args.size() < 4) { std::cerr << "Usage: hpcstat submitjob <jobid> <submitdir> <jobcommand>\n"; return 1; }
    if (auto key = authenticated(); !key) return 1;
    else if (auto session = session_id(); !session) return 1;
    else writedb(SubmitJobData
    {
      .JobId = std::stoi(args[2]), .Key = key->first, .SessionId = *session,
      .SubmitDir = std::filesystem::current_path().string(),
      .JobCommand = [&]
        { std::stringstream ss; for (int i = 3; i < args.size(); i++) ss << args[i] << " "; return ss.str(); }()
    });
  }
  else if (args[1] == "finishjob")
  {
    
  }
  else
  {
    std::cerr << "Unknown command\n";
    return 1;
  }

  return 0;
}
