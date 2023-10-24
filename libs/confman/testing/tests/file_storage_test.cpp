#include <boost/asio.hpp>
#include <boost/ut.hpp>
#include <glaze/glaze.hpp>
#include <tfc/confman/file_storage.hpp>

#include <tfc/confman/observable.hpp>
#include <tfc/progbase.hpp>

#include "inc/file_storage.hpp"

namespace asio = boost::asio;
namespace ut = boost::ut;
using ut::operator""_test;
using ut::operator/;
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

    // delete backup files
    for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(this->file().parent_path())) {
      if (entry.path().string().find("/tmp/test") != std::string::npos) {
        std::filesystem::remove(entry.path());
      }
    }
  }
};

auto create_file(std::filesystem::path file_path) -> void {
  if (!std::filesystem::exists(file_path)) {
    std::ofstream ofs(file_path);
    ofs << "some text\n";
    ofs.close();
  }
}

auto main(int argc, char** argv) -> int {
  tfc::base::init(argc, argv);

  asio::io_context ctx{};
  std::string const file_name{ "/tmp/test.me" };

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
    glz::read_file_json(json, file_name, buffer);
    ut::expect(static_cast<int>(json["a"].get<double>()) == 1);
    ut::expect(json["b"].get<std::string>() == "bar");

    conf.make_change()->a = 2;
    conf.make_change()->b = "test";

    buffer = {};
    glz::read_file_json(json, file_name, buffer);
    ut::expect(static_cast<int>(json["a"].get<double>()) == 2);
    ut::expect(json["b"].get<std::string>() == "test");
  };

  "write to file"_test = [&] {
    file_testable<test_me> const conf{ ctx, file_name };
    glz::json_t json{};
    std::string buffer{};
    glz::read_file_json(json, file_name, buffer);

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
    file_testable<map> my_map{ ctx, "/tmp/test.me" };
    { my_map.make_change().value()["new_key"] = test_me{ .a = observable<int>{ 42 }, .b = "hello-world" }; }
    ut::expect(my_map.value().at("new_key") == test_me{ .a = observable<int>{ 42 }, .b = "hello-world" });
    ut::expect(my_map->at("new_key") == test_me{ .a = observable<int>{ 42 }, .b = "hello-world" });

    uint32_t called{};
    my_map.on_change([&called]() { called++; });

    ctx.run_for(std::chrono::milliseconds(1));
    ut::expect(called == 1);
  };

  "backup on config change"_test = [&] {
    file_testable<test_me> conf{ ctx, "/tmp/test_uuid", test_me{ .a = observable<int>{ 3 }, .b = "bar" } };
    conf.make_change()->a = 2;
    ctx.run_for(std::chrono::milliseconds(1));
    auto files = std::filesystem::directory_iterator("/tmp");

    bool backup_found = false;
    std::string found_file;

    for (const auto& file : files) {
      if (file.path().string().find("/tmp/test_uuid_") != std::string::npos) {
        backup_found = true;
        found_file = file.path().string();
        break;
      }
    }

    ut::expect(backup_found) << "No backup file found for modified config";

    if (backup_found) {
      glz::json_t backup_json{};
      std::string buffer{};
      glz::read_file_json(backup_json, found_file, buffer);

      ut::expect(backup_json["a"].get<double>() == 3) << backup_json["a"].get<double>();
      ut::expect(backup_json["b"].get<std::string>() == "bar");
    }
  };

  "testing retention policy without removal"_test = [&] {
    std::filesystem::path const parent_path{ std::filesystem::temp_directory_path() / "retention" };

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

    tfc::confman::remove_files_exceeding_retention(file_times, 4, std::chrono::days(20));

    ctx.run_for(std::chrono::milliseconds(1));

    ut::expect(std::distance(std::filesystem::directory_iterator(parent_path), std::filesystem::directory_iterator{}) == 2);

    std::filesystem::remove_all(parent_path);
  };

  "testing retention policy with removal"_test = [&] {
    std::filesystem::path const parent_path{ std::filesystem::temp_directory_path() / "retention" };

    std::vector<std::filesystem::path> const paths{ std::filesystem::path{ parent_path / "new_file1.txt" },
                                                    std::filesystem::path{ parent_path / "new_file2.txt" },
                                                    std::filesystem::path{ parent_path / "new_file3.txt" },
                                                    std::filesystem::path{ parent_path / "new_file4.txt" } };

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

    tfc::confman::remove_files_exceeding_retention(file_times, 2, std::chrono::days(20));

    ctx.run_for(std::chrono::milliseconds(1));

    ut::expect(std::distance(std::filesystem::directory_iterator(parent_path), std::filesystem::directory_iterator{}) == 2);

    std::filesystem::remove_all(parent_path);
  };

  "write to file"_test = [&] {
    std::filesystem::path const file_path{ std::filesystem::temp_directory_path() / "glaze file" };
    std::string const file_content = R"(file content: {"username": "newnewnewnewnewnewnew"})";
    tfc::confman::write_to_file(file_path, file_content);
    ut::expect(std::filesystem::exists(file_path));
    std::filesystem::remove(file_path);
  };

  return EXIT_SUCCESS;
}
