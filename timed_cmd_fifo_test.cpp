//
// Copyright 2014 Ettus Research LLC
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

// This tiny program is meant as an example on how to set up UHD
// projects using CMake.
// The program itself only initializes a USRP. For more elaborate examples,
// have a look at the files in host/examples/.

#include <uhd/utils/thread_priority.hpp>
#include <uhd/utils/safe_main.hpp>
#include <uhd/usrp/multi_usrp.hpp>
#include <uhd/types/time_spec.hpp>
#include <boost/program_options.hpp>
#include <boost/format.hpp>
#include <iostream>
#include <chrono> //yes. C++11. That's why.

namespace po = boost::program_options;

int UHD_SAFE_MAIN(int argc, char *argv[]){
    uhd::set_thread_priority_safe();

    //variables to be set by po
    std::string args;

    //setup the program options
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "help message")
        ("args", po::value<std::string>(&args)->default_value(""), "multi uhd device address args")
    ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    //print the help message
    if (vm.count("help")){
        std::cout << boost::format("Mini-example to illustrate cmd queuing (args==%s).") % args << std::endl;
        return ~0;
    }

    //create a usrp device
    std::cout << std::endl;
    std::cout << boost::format("Creating the usrp device with: %s...") % args << std::endl;
    uhd::usrp::multi_usrp::sptr usrp = uhd::usrp::multi_usrp::make(args);

    uhd::time_spec_t next = usrp->get_time_now() + uhd::time_spec_t(0.1);
    const uhd::time_spec_t delta(0.05);
    std::chrono::time_point<std::chrono::high_resolution_clock> clock_now;
    std::chrono::time_point<std::chrono::high_resolution_clock> clock_last = std::chrono::high_resolution_clock::now();
    double delay;
    std::chrono::duration<double> time_diff;
    int last_direct = 0;

    for(int i = 0; i < 100; i++)
    {
        usrp->set_command_time(next);
        usrp->set_rx_gain(i%2);
        clock_now = std::chrono::high_resolution_clock::now();
        time_diff = clock_now - clock_last;
        delay = time_diff.count();
        if(delay < delta.get_real_secs() * 0.1)
        {
            std::cout << "\r";
            last_direct = i;
        }
        else
        {
            std::cout << std::endl;
        }
        std::cout << boost::format("timed command issued: %04d (delay %f)") 
            % i
            % delay;
        next += delta;
        clock_last = std::chrono::high_resolution_clock::now();
    }
    std::cout << "-------------------" << std::endl
        << boost::format("without delay: %d out of %d") % last_direct % 100 << std::endl;

    return EXIT_SUCCESS;
}
