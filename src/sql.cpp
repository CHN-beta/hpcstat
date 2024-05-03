# include <filesystem>
# include <set>
# include <hpcstat/sql.hpp>
# include <hpcstat/env.hpp>
# include <range/v3/range.hpp>
# include <range/v3/view.hpp>

namespace hpcstat::sql
{
  std::string serialize(auto data)
  {
    auto [serialized_data_byte, out] = zpp::bits::data_out();
    out(data).or_throw();
    static_assert(sizeof(char) == sizeof(std::byte));
    return { reinterpret_cast<char*>(serialized_data_byte.data()), serialized_data_byte.size() };
  }
  template std::string serialize(LoginData);
  template std::string serialize(SubmitJobData);
  template std::string serialize(FinishJobData);
  std::optional<zxorm::Connection<LoginTable, LogoutTable, SubmitJobTable, FinishJobTable>> connect()
  {
    if (auto datadir = env::env("HPCSTAT_DATADIR", true); !datadir)
      return std::nullopt;
    else
    {
      auto dbfile = std::filesystem::path(*datadir) / "hpcstat.db";
      return std::make_optional<zxorm::Connection<LoginTable, LogoutTable, SubmitJobTable, FinishJobTable>>
        (dbfile.c_str());
    }
  }
  bool initdb()
    { if (auto conn = connect(); !conn) return false; else { conn->create_tables(); return true; } }
  bool writedb(auto value)
    { if (auto conn = connect(); !conn) return false; else { conn->insert_record(value); return true; } }
  template bool writedb(LoginData);
  template bool writedb(LogoutData);
  template bool writedb(SubmitJobData);
  template bool writedb(FinishJobData);
  std::optional<std::set<unsigned>> finishjob_remove_existed(std::map<unsigned, std::string> jobid_submit_time)
  {
    if (auto conn = connect(); !conn) return std::nullopt;
    else
    {
      auto all_job = jobid_submit_time | ranges::views::keys | ranges::to<std::vector<unsigned>>;
      auto not_logged_job = all_job | ranges::to<std::set<unsigned>>;
      for (auto it : conn->select_query<FinishJobData>()
        .order_by<FinishJobTable::field_t<"id">>(zxorm::order_t::DESC)
        .where_many(FinishJobTable::field_t<"id">().in(all_job))
        .exec())
        if (jobid_submit_time[it.JobId] == it.SubmitTime)
          not_logged_job.erase(it.JobId);
      return not_logged_job;
    }
  }
}
