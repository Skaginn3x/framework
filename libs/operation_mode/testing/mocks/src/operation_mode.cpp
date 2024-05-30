#include "../inc/public/tfc/mocks/operation_mode.hpp"

#include <bits/fs_dir.h>
#include <fmt/printf.h>
#include <ranges>

namespace {
// clang-format off
  PRAGMA_CLANG_WARNING_PUSH_OFF(-Wexit-time-destructors)
  PRAGMA_CLANG_WARNING_PUSH_OFF(-Wglobal-constructors)
  thread_local tfc::operation::uuid_t next_uuid_;
  thread_local std::vector<tfc::operation::callback_item> callbacks_{};
  thread_local tfc::operation::mode_e current_mode_{ tfc::operation::mode_e::unknown };
  PRAGMA_CLANG_WARNING_POP
  PRAGMA_CLANG_WARNING_POP
// clang-format on
}  // namespace

namespace tfc::operation {
mock_interface::mock_interface(asio::io_context&, std::string_view, std::string_view) {}

mock_interface::mock_interface(mock_interface&&) noexcept {}

auto mock_interface::operator=(mock_interface&&) noexcept -> mock_interface& {
  return *this;
}

void mock_interface::set(mode_e new_mode) const {
  auto old_mode = current_mode_;
  current_mode_ = new_mode;
  const_cast<mock_interface*>(this)->mode_update_impl(update_message{ .new_mode = new_mode, .old_mode = old_mode });
}

auto mock_interface::get_next_uuid() -> uuid_t {
  return next_uuid_++;
}

void mock_interface::stop(const std::string_view /*reason*/) const {
  set(mode_e::stopped);
}

std::error_code mock_interface::remove_callback(uuid_t uuid) {
  auto number_of_erased_items{ std::erase_if(callbacks_, [uuid](auto const& item) -> bool { return item.uuid == uuid; }) };
  if (number_of_erased_items == 0) {
    return std::make_error_code(std::errc::argument_out_of_domain);
  }
  return {};
}
std::error_code mock_interface::remove_all_callbacks() {
  callbacks_.clear();
  return {};
}
std::error_code mock_interface::reset() {
  auto err = remove_all_callbacks();
  set(mode_e::unknown);
  return err;
}
uuid_t mock_interface::append_callback_impl(mode_e mode_value,
                                            transition_e transition,
                                            std::function<void(new_mode_e, old_mode_e)> callback) {
  uuid_t const uuid{ get_next_uuid() };
  callbacks_.emplace_back(callback_item{ .mode = mode_value, .transition = transition, .callback = callback, .uuid = uuid });
  return uuid;
}
void mock_interface::mode_update_impl(update_message const update_msg) noexcept {
  constexpr auto make_transition_filter{ [](transition_e trans) noexcept {
    return [trans](callback_item const& itm) { return itm.transition == trans; };
  } };
  constexpr auto make_mode_filter{ [](mode_e mode) noexcept {
    return [mode](callback_item const& itm) { return itm.mode == mode; };
  } };

  const auto invoke{ [update_msg](callback_item const& itm) noexcept {
    if (itm.callback) {
      try {
        std::invoke(itm.callback, update_msg.new_mode, update_msg.old_mode);
      } catch ([[maybe_unused]] std::exception const& exc) {
        fmt::println(stderr, R"(Exception from callback id: "{}", what: "{}")", itm.uuid, exc.what());
      }
    }
  } };

  // Call leave first
  for (auto& callback : callbacks_ | std::views::filter(make_transition_filter(transition_e::leave)) |
                            std::views::filter(make_mode_filter(update_msg.old_mode))) {
    invoke(callback);
  }
  // Call enter second
  for (auto& callback : callbacks_ | std::views::filter(make_transition_filter(transition_e::enter)) |
                            std::views::filter(make_mode_filter(update_msg.new_mode))) {
    invoke(callback);
  }
}
}  // namespace tfc::operation
