
#include <sdbusplus/asio/connection.hpp>

#include <tfc/logger.hpp>
#include <tfc/snitch/details/dbus_client.hpp>
#include <tfc/snitch/details/snitch_impl.hpp>
#include <tfc/progbase.hpp>

namespace tfc::snitch::detail {

alarm_impl::alarm_impl(std::shared_ptr<sdbusplus::asio::connection> conn,
                       std::string_view unique_id,
                       std::string_view description,
                       std::string_view details,
                       bool resettable,
                       level_e lvl,
                       std::unordered_map<std::string, std::string>&& default_args)
    : given_id_{ unique_id }, description_{ description }, details_{ details }, resettable_{ resettable }, lvl_{ lvl }, tfc_id_{ fmt::format("{}.{}.{}", base::get_exe_name(), base::get_proc_name(), given_id_) }, default_values_{ std::move(default_args) },
      conn_{ std::move(conn) }, dbus_client_{ std::make_unique<dbus_client>(conn_) },
      logger_{ std::make_unique<logger::logger>(fmt::format("snitch.{}", given_id_)) },
      retry_timer_{ conn_->get_io_context() } {
  register_alarm();
}

void alarm_impl::on_try_reset(std::function<void()> callback) {
  dbus_client_->on_try_reset_alarm([this, callback](api::alarm_id_t id) {
    if (activation_id_.has_value() && id == alarm_id_) {
      std::invoke(callback);
    }
  });
  dbus_client_->on_try_reset_all_alarms([this, callback] {
    if (activation_id_.has_value()) {
      std::invoke(callback);
    }
  });
}

void alarm_impl::set(std::string_view description_formatted, std::string_view details_formatted, std::unordered_map<std::string, std::string>&& args, std::function<void(std::error_code)>&& on_set_finished) {
  if (activation_id_) {
    logger_->info("alarm already active with id: {}, not doing anything", activation_id_.value());
    return;
  }

  logger_->debug("Description: '{}'", description_formatted);
  logger_->debug("Details: '{}'", details_formatted);

  auto params{ default_values_ };
  params.merge(std::move(args));

  set(std::move(params), std::move(on_set_finished));
}

void alarm_impl::set(std::unordered_map<std::string, std::string>&& params, std::function<void(std::error_code)>&& on_set_finished) {
  auto const now = decltype(retry_timer_)::clock_type::now();
  if (now > retry_timer_.expiry()) {
    retry_timer_.cancel();
  }
  if (alarm_id_) {
    dbus_client_->set_alarm(alarm_id_.value(), params, [this, cb = std::move(on_set_finished)](std::error_code const& ec, api::activation_id_t id) {
      if (ec) {
        logger_->error("Failed to set alarm: {}", ec.message());
        // todo: should we call test here?
      }
      else {
        activation_id_ = id;
      }
      std::invoke(cb, ec);
    });
  } else {
    retry_timer_.expires_after(std::chrono::seconds(1));
    retry_timer_.async_wait([this, params_mv = std::move(params), cb = std::move(on_set_finished)](std::error_code const& ec) mutable {
      if (ec) {
        logger_->info("Retry set timer failed: {}", ec.message());
        return;
      }
      logger_->debug("Retrying set alarm");
      set(std::move(params_mv), std::move(cb));
    });
  }
}

void alarm_impl::reset() {
  if (!activation_id_) {
    logger_->info("alarm already inactive, not doing anything");
    return;
  }
  dbus_client_->reset_alarm(activation_id_.value(), [this](std::error_code const& ec) {
    if (ec) {
      logger_->error("Failed to reset alarm: {}", ec.message());
      return;
    }
    activation_id_.reset();
  });
}


void alarm_impl::register_alarm() {
  dbus_client_->register_alarm(tfc_id_, description_, details_, resettable_, lvl_,
                               [this](std::error_code const& ec, api::alarm_id_t id) {
                                 if (ec) {
                                   logger_->error("Failed to register alarm: {}", ec.message());
                                   return;
                                 }
                                 alarm_id_ = id;
                               });
}

}  // namespace tfc::snitch::detail
