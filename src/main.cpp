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
# include <fmt/format.h>
# include <fmt/ranges.h>
# include <zxorm/zxorm.hpp>

using namespace std::literals;

// ssh fingerprint -> username
const std::map<std::string, std::string> Username
{
  { "LNoYfq/SM7l8sFAy325WpC+li+kZl3jwST7TmP72Tz8", "Haonan Chen" }
};

// program path, set at start of main, e.g. /gpfs01/.../bin/hpcstat
std::filesystem::path Program;

// run a program, wait until it exit, return its stdout and stderr if it return 0, otherwise nullopt
std::optional<std::string> exec(boost::filesystem::path program, std::vector<std::string> args)
{
  namespace bp = boost::process;
  bp::ipstream output;
  auto process = bp::child(program, bp::args(args), bp::std_out > output, bp::std_err > bp::null);
  process.wait();
  if (process.exit_code() != 0) return std::nullopt;
  std::stringstream ss;
  ss << output.rdbuf();
  return ss.str();
}

// detect ssh fingerprint using ssh-add
std::optional<std::vector<std::string>> fingerprints()
{
  auto output =
    exec(Program.replace_filename("ssh-add"), { "-l" });
  if (!output) return std::nullopt;
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
  return std::nullopt;
}

// initialize the database
// std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
struct LoginData
{
  unsigned Id = 0;
  long Time = 0;
  std::string Key;
  std::string SessionId;
  bool Interactive;
};
using LoginTable = zxorm::Table
<
  "login", LoginData,
  zxorm::Column<"id", &LoginData::Id, zxorm::PrimaryKey<>>,
  zxorm::Column<"time", &LoginData::Time>,
  zxorm::Column<"key", &LoginData::Key>,
  zxorm::Column<"session_id", &LoginData::SessionId>
>;
struct LogoutData
{
  unsigned Id = 0;
  long Time = 0;
  std::string SessionId;
};
using LogoutTable = zxorm::Table
<
  "logout", LogoutData,
  zxorm::Column<"id", &LogoutData::Id, zxorm::PrimaryKey<>>,
  zxorm::Column<"time", &LogoutData::Time>,
  zxorm::Column<"session_id", &LogoutData::SessionId>
>;
// struct SubmitData
// {
//   unsigned Id;
//   long Time;
//   std::string Key;
//   std::string SessionId;
//   unsigned JobId;
//   std::string JobWorkdir;
//   std::string JobCommand;
// };
// struct FinishData
// {
//   unsigned Id;
//   long Time;
//   unsigned JobId;
//   std::string JobResult;
//   double CpuTime;
// };
void initdb()
{
  auto dbfile = Program.replace_filename("hpcstat.db").string();
  zxorm::Connection<LoginTable, LogoutTable> conn(dbfile.c_str());
  conn.create_tables();
}
void writedb(auto value)
{
  auto dbfile = Program.replace_filename("hpcstat.db").string();
  zxorm::Connection<LoginTable, LogoutTable> conn(dbfile.c_str());
  value.Time = std::chrono::duration_cast<std::chrono::seconds>
    (std::chrono::system_clock::now().time_since_epoch()).count();
  conn.insert_record(value);
}

bool interactive()
{
  return isatty(fileno(stdin));
}

// get value of XDG_SESSION_ID
std::optional<std::string> session_id()
{
  auto value = std::getenv("XDG_SESSION_ID");
  if (!value) return std::nullopt; else return value;
}

int main(int argc, const char** argv)
{
  std::vector<std::string> args(argv, argv + argc);
  Program = args[0];

  if (args.size() == 1)
  {
    std::cout << "Usage: hpcstat initdb|login|logout|submitjob|finishjob\n";
    return 1;
  }
  else if (args[1] == "initdb")
    initdb();
  else if (args[1] == "login")
  {
    auto key = authenticated();
    if (!key)
    {
      auto fps = fingerprints();
      if (!fingerprints())
        std::cerr << "Failed to get ssh fingerprints\n";
      else
        std::cerr << fmt::format("No valid fingerprint found, available fingerprints: {}\n", *fps);
      return 1;
    }
    auto session = session_id();
    if (!session)
    {
      std::cerr << "Failed to get session id\n";
      return 1;
    }
    writedb(LoginData{.Key = key->first, .SessionId = *session, .Interactive = interactive()});
  }
  else if (args[1] == "logout")
  {
    auto session = session_id();
    if (!session)
    {
      std::cerr << "Failed to get session id\n";
      return 1;
    }
    writedb(LogoutData{.SessionId = *session});
  }
  else if (args[1] == "submitjob")
  {
    // TODO
  }
  else if (args[1] == "finishjob")
  {
    // TODO
  }
  else
  {
    std::cerr << "Unknown command\n";
    return 1;
  }

  return 0;
}
