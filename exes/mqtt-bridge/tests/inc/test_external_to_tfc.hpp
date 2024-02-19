#pragma once

namespace tfc::mqtt {
    class test_external_to_tfc {
    public:
        test_external_to_tfc() = default;

        auto test() -> bool;

        auto test_last_word(std::string input_string, std::optional<std::string> output_string) -> bool;
    };
} // namespace tfc::mqtt
