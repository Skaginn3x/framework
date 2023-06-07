#pragma once

#include <chrono>
#include <cstddef>
#include <functional>
#include <new>
#include <ranges>
#include <span>

#include <boost/asio/io_context.hpp>

#include <tfc/logger.hpp>

#include "tfc/ec/soem_interface.hpp"

namespace tfc::ec::devices {
    template<typename setting_t>
    concept setting_c = requires {
        setting_t::index;
        setting_t::value;
    };

    class base {
    public:
        virtual ~base();

        // Default behaviour no data processing
        virtual void process_data(std::span<std::byte>, std::span<std::byte>) = 0;

        // Default behaviour, no setup
        virtual auto setup() -> int { return 1; }

        void set_sdo_write_cb(auto &&cb) { sdo_write_ = std::forward<decltype(cb)>(cb); }

        auto sdo_write(ecx::index_t idx,
                       ecx::complete_access_t acc,
                       std::span<std::byte> const &data,
                       std::chrono::microseconds timeout) const -> ecx::working_counter_t {
            if (sdo_write_) {
                return std::invoke(sdo_write_, idx, acc, data, timeout);
            }
            logger_.warn("Sdo write callback not set");
            return {};
        }

        template<std::integral integral_t>
        auto sdo_write(ecx::index_t idx, integral_t value) const -> ecx::working_counter_t {
            if (sdo_write_) {
                std::span data{std::launder(reinterpret_cast<std::byte *>(&value)), sizeof(value)};
                return std::invoke(sdo_write_, idx, false, data, ecx::constants::timeout_safe);
            }
            logger_.warn("Sdo write callback not set");
            return {};
        }

        template<setting_c setting_t>
        auto sdo_write(setting_t &&in) const {
            if constexpr (std::is_enum_v<decltype(setting_t::value)>) {
                return base::sdo_write(in.index, std::to_underlying(in.value));
            } else {
                return base::sdo_write(in.index, in.value);
            }
        }

    protected:
        explicit base(uint16_t slave_index) : slave_index_(slave_index),
                                              logger_(fmt::format("Ethercat slave {}", slave_index)) {}

        const uint16_t slave_index_{};
        tfc::logger::logger logger_;

    private:
        std::function<
                ecx::working_counter_t(ecx::index_t, ecx::complete_access_t, std::span<std::byte>,
                                       std::chrono::microseconds)>
                sdo_write_{};
    };

    class default_device final : public base {
    public:
        ~default_device() final;

        explicit default_device(uint16_t const slave_index) : base(slave_index) {}

        void process_data(std::span<std::byte>, std::span<std::byte>) noexcept final {}

        auto setup() -> int final { return 1; }
    };
}  // namespace tfc::ec::devices
