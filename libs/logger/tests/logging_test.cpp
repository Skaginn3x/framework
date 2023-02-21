#include "tfc/logger.hpp"
#include <boost/ut.hpp>
#include <iostream>

namespace tfc {
    static auto init(int argc, char** argv){
        std::cout << "INIT MUCH" << std::endl;
    }
}

auto main (int argc, char** argv) -> int {
    // Initilize framework
    tfc::init(argc, argv);
    tfc::logger::logger foo("key");

    //foo.log<tfc::logger::lvl::info>("Some arguments {}: {}", 1 , 2);

    boost::ut::expect(true);
}
