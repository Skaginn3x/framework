#include <chrono>
#include <filesystem>
#include <fstream>
#include <map>
#include <optional>
#include <string>
#include <system_error>
#include <vector>

#include <boost/asio.hpp>
#include <boost/ut.hpp>
#include <glaze/glaze.hpp>
#include <tfc/confman/file_storage.hpp>

#include <tfc/confman/observable.hpp>
#include <tfc/progbase.hpp>
#include "tfc/confman/detail/retention.hpp"

namespace asio = boost::asio;
namespace ut = boost::ut;
using ut::operator""_test;
using tfc::confman::observable;

struct test_me {
  tfc::confman::observable<int> a{};
  std::string b{};
  struct glaze {
    static constexpr auto value{ glz::object("a", &test_me::a, "b", &test_me::b) };
    static constexpr auto name{ "test_me" };
  };
};

[[maybe_unused]] static auto operator==(test_me const& lhs, test_me const& rhs) noexcept -> bool {
  return lhs.a == rhs.a && lhs.b == rhs.b;
}

using map = std::map<std::string, test_me>;

template <typename storage_t>
class file_testable : public tfc::confman::file_storage<storage_t> {
public:
  using tfc::confman::file_storage<storage_t>::file_storage;

  ~file_testable() {
    std::error_code ignore{};
    std::filesystem::remove(this->file(), ignore);
  }
};

namespace {
auto create_file(const std::filesystem::path& file_path) -> void {
  if (!std::filesystem::exists(file_path)) {
    std::ofstream ofs(file_path);
    ofs << "some text\n";
    ofs.close();
  }
}
}  // namespace

auto main(int argc, char** argv) -> int {
  tfc::base::init(argc, argv);

  std::filesystem::path file_name{ std::filesystem::temp_directory_path() / "file_storage_tests" / "test.me" }; // NOSONAR

  if (!std::filesystem::exists(file_name.parent_path())) {
    std::filesystem::create_directories(file_name.parent_path());
  }

  asio::io_context ctx{};

  "file path"_test = [&] {
    file_testable<test_me> const conf{ ctx, file_name };
    ut::expect(conf.file().string() == file_name);
  };

  "default values"_test = [&] {
    file_testable<test_me> const conf{ ctx, file_name, test_me{ .a = observable<int>{ 1 }, .b = "bar" } };
    ut::expect(1 == conf->a);
    ut::expect("bar" == conf->b);
    ut::expect(1 == conf.value().a);
    ut::expect("bar" == conf.value().b);
  };

  "change it"_test = [&] {
    file_testable<test_me> conf{ ctx, file_name, test_me{ .a = observable<int>{ 1 }, .b = "bar" } };
    uint32_t a_called{};
    conf->a.observe([&a_called](int new_value, int old_value) {
      ut::expect(new_value == 2);
      ut::expect(old_value == 1);
      a_called++;
    });
    conf.make_change()->a = 2;

    ut::expect(a_called == 1);
  };

  "subscript-able"_test = [&] {
    file_testable<test_me> conf{ ctx, file_name };
    uint32_t called{};
    conf.on_change([&called]() { called++; });
    conf.make_change()->a = 1;
    ctx.run_for(std::chrono::milliseconds(1));
    ut::expect(called == 1);
  };

  "verify file"_test = [&] {
    file_testable<test_me> conf{ ctx, file_name, test_me{ .a = observable<int>{ 1 }, .b = "bar" } };
    glz::json_t json{};
    std::string buffer{};
    glz::read_file_json(json, file_name.string(), buffer);
    ut::expect(static_cast<int>(json["a"].get<double>()) == 1);
    ut::expect(json["b"].get<std::string>() == "bar");

    conf.make_change()->a = 2;
    conf.make_change()->b = "test";

    buffer = {};
    glz::read_file_json(json, file_name.string(), buffer);
    ut::expect(static_cast<int>(json["a"].get<double>()) == 2);
    ut::expect(json["b"].get<std::string>() == "test");
  };

  "write to file"_test = [&] {
    file_testable<test_me> const conf{ ctx, file_name };
    glz::json_t json{};
    std::string buffer{};
    glz::read_file_json(json, file_name.string(), buffer);

    uint32_t a_called{};
    conf->a.observe([&a_called](int new_a, int old_a) {
      a_called++;
      ut::expect(new_a == 32);
      ut::expect(old_a == 0);
    });

    json["a"] = 32;
    json["b"] = "test";
    buffer = {};
    std::ignore = glz::write_file_json(json, file_name, buffer);

    ctx.run_for(std::chrono::milliseconds(10));

    ut::expect(a_called == 1);
    ut::expect(conf->a == 32);
    ut::expect(conf->b == "test");
  };

  "change test"_test = [&]() {
    file_testable<map> my_map{ ctx, file_name };
    { my_map.make_change().value()["new_key"] = test_me{ .a = observable<int>{ 42 }, .b = "hello-world" }; }
    ut::expect(my_map.value().at("new_key") == test_me{ .a = observable<int>{ 42 }, .b = "hello-world" });
    ut::expect(my_map->at("new_key") == test_me{ .a = observable<int>{ 42 }, .b = "hello-world" });

    uint32_t called{};
    my_map.on_change([&called]() { called++; });

    ctx.run_for(std::chrono::milliseconds(1));
    ut::expect(called == 1);
  };

  "backup on config change"_test = [&] {
    std::filesystem::path const json_file_name{ file_name.parent_path() / "test_uuid.json" };

    file_testable<test_me> conf{ ctx, json_file_name, test_me{ .a = observable<int>{ 3 }, .b = "bar" } };
    conf.make_change()->a = 2;
    ctx.run_for(std::chrono::milliseconds(1));
    auto files = std::filesystem::directory_iterator(json_file_name.parent_path());

    bool backup_found = false;
    std::filesystem::path found_file;

    for (const auto& file : files) {
      if (file.path().string().find(static_cast<std::string>(json_file_name.parent_path().string() + "/test_uuid_")) !=
          std::string::npos) {
        backup_found = true;
        found_file = file.path();
        break;
      }
    }

    ut::expect(backup_found) << "No backup file found for modified config";

    if (backup_found) {
      glz::json_t backup_json{};
      std::string buffer{};
      glz::read_file_json(backup_json, found_file.string(), buffer);

      ut::expect(backup_json["a"].as<int>() == 3) << backup_json["a"].as<int>();
      ut::expect(backup_json["b"].get<std::string>() == "bar") << backup_json["b"].get<std::string>();
    }
  };

  "testing retention policy without removal"_test = [&] {
    std::filesystem::path const parent_path{ file_name.parent_path() / "retention" };

    std::vector<std::filesystem::path> const paths{ std::filesystem::path{ parent_path / "new_file1.txt" },
                                                    std::filesystem::path{ parent_path / "new_file2.txt" } };

    if (!std::filesystem::exists(parent_path)) {
      std::filesystem::create_directories(parent_path);
    }

    for (const auto& path : paths) {
      create_file(path);
    }

    ctx.run_for(std::chrono::milliseconds(1));

    std::map<std::filesystem::file_time_type, std::filesystem::path> file_times;

    for (const auto& path : paths) {
      file_times[std::filesystem::file_time_type::clock::now() - std::chrono::days(30)] = path.string();
    }

    tfc::confman::remove_json_files_exceeding_retention(file_times, 4, std::chrono::days(20));

    ctx.run_for(std::chrono::milliseconds(1));

    ut::expect(std::distance(std::filesystem::directory_iterator(parent_path), std::filesystem::directory_iterator{}) == 2);

    std::filesystem::remove_all(parent_path);
  };

  "testing retention policy with removal"_test = [&] {
    std::filesystem::path const parent_path{ std::filesystem::temp_directory_path() / "retention" };

    std::vector<std::filesystem::path> const paths{ std::filesystem::path{ parent_path / "new_file1.json" },
                                                    std::filesystem::path{ parent_path / "new_file2.json" },
                                                    std::filesystem::path{ parent_path / "new_file3.json" },
                                                    std::filesystem::path{ parent_path / "new_file4.json" } };

    if (!std::filesystem::exists(parent_path)) {
      std::filesystem::create_directories(parent_path);
    }

    for (const auto& path : paths) {
      create_file(path);
    }

    ctx.run_for(std::chrono::milliseconds(1));

    std::map<std::filesystem::file_time_type, std::filesystem::path> file_times;

    for (const auto& path : paths) {
      file_times[std::filesystem::file_time_type::clock::now() - std::chrono::days(30)] = path.string();
    }

    tfc::confman::remove_json_files_exceeding_retention(file_times, 2, std::chrono::days(20));

    ctx.run_for(std::chrono::milliseconds(1));

    ut::expect(std::distance(std::filesystem::directory_iterator(parent_path), std::filesystem::directory_iterator{}) == 2)
        << std::distance(std::filesystem::directory_iterator(parent_path), std::filesystem::directory_iterator{});

    std::filesystem::remove_all(parent_path);
  };

  "write to file"_test = [&] {
    std::filesystem::path const file_path{ std::filesystem::temp_directory_path() / "glaze file" };
    std::string const file_content = R"(file content: {"username": "me"})";

    auto write_error{ tfc::confman::write_to_file(file_path, file_content) };
    if (write_error) {
      ut::expect(false) << "write error";
    }

    ut::expect(std::filesystem::exists(file_path));
    std::filesystem::remove(file_path);
  };

  "get env variable"_test = [&] {
    std::optional<int> const env = tfc::confman::getenv<int>("TFC_CONFMAN_MIN_RETENTION_COUNT");
    ut::expect(!env.has_value());
  };

  "map creation"_test = [&] {
    std::filesystem::path const parent_path{ std::filesystem::temp_directory_path() / "retention" };

    std::vector<std::filesystem::path> const paths{
      std::filesystem::path{ parent_path / "new_file1.json" },  std::filesystem::path{ parent_path / "new_file2.json" },
      std::filesystem::path{ parent_path / "new_file3.json" },  std::filesystem::path{ parent_path / "new_file4.json" },
      std::filesystem::path{ parent_path / "new.json" },        std::filesystem::path{ parent_path / "new2.json" },
      std::filesystem::path{ parent_path / "n.json" },          std::filesystem::path{ parent_path / "n2.json" },
      std::filesystem::path{ parent_path / "extra_file1.txt" }, std::filesystem::path{ parent_path / "extra_file2.txt" },
      std::filesystem::path{ parent_path / "extra_file.swp" }
    };

    if (!std::filesystem::exists(parent_path)) {
      std::filesystem::create_directories(parent_path);
    }

    for (const auto& path : paths) {
      create_file(path);
    }

    int counter = 0;
    for (auto const& path : paths) {
      std::filesystem::last_write_time(path,
                                       std::filesystem::file_time_type::clock::now() - std::chrono::days(30 + counter++));
    }

    ut::expect(std::distance(std::filesystem::directory_iterator(parent_path), std::filesystem::directory_iterator{}) == 11)
        << std::distance(std::filesystem::directory_iterator(parent_path), std::filesystem::directory_iterator{});

    std::map<std::filesystem::file_time_type, std::filesystem::path> const file_times =
        tfc::confman::get_json_files_by_last_write_time(std::filesystem::path{ parent_path / "new_file.json" }
                                                        // parent_path
        );

    ut::expect(file_times.size() == 4) << file_times.size();

    std::filesystem::remove_all(parent_path);
  };

  // delete entire folder to delete backup files
  std::error_code ignore{};
  std::filesystem::remove_all(file_name.parent_path(), ignore);

  return EXIT_SUCCESS;
}
