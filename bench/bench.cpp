// Copyright 2023 TikTok Pte. Ltd.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <fstream>
#include <stdexcept>

#include "glog/logging.h"
#include "tclap/CmdLine.h"
#include "verse_bench.h"

#include "network/net_factory.h"
#include "network/network.h"

#include "verse/verse_factory.h"

class LogToFileSink : public google::LogSink {
public:
    explicit LogToFileSink(const std::string& file_name) {
        log_file_.open(file_name, std::ios::out);
    }

    ~LogToFileSink() override {
        log_file_.close();
    }

    void send(google::LogSeverity severity, const char* full_filename, const char* base_filename, int line,
            const struct ::tm* tm_time, const char* message, size_t message_len) override {
        log_file_.write(message, message_len);
        log_file_.put('\n');
        log_file_.flush();
    }

private:
    std::ofstream log_file_;
};

int main(int argc, char** argv) {
    try {
        TCLAP::CmdLine cmd("verse demo", ' ', "0.1");
        // add single value cmd params
        TCLAP::ValueArg<std::size_t> party_arg("p", "party", "which party", false, 0, "std::size_t");
        TCLAP::ValueArg<std::string> test_case_arg("c", "case", "which case to run", false, "all", "string");
        TCLAP::ValueArg<std::size_t> test_number_arg("n", "number", "how many times to run", false, 10, "std::size_t");
        TCLAP::ValueArg<std::string> log_path_arg(
                "", "log_path", "log path of this paty", false, "./verse.log", "std::string");
        // add multi value cmd params

        TCLAP::MultiArg<std::string> host_arg("", "hosts", "host of all party", false, "std::string");
        TCLAP::MultiArg<std::uint16_t> port_arg("", "ports", "port of all party", false, "std::uint16_t");

        // single
        cmd.add(party_arg);
        cmd.add(test_case_arg);
        cmd.add(test_number_arg);
        cmd.add(log_path_arg);

        // multi
        cmd.add(host_arg);
        cmd.add(port_arg);

        // parse
        cmd.parse(argc, argv);

        // get value
        std::size_t party = party_arg.getValue();
        std::string test_case = test_case_arg.getValue();
        std::size_t test_number = test_number_arg.getValue();
        std::string log_path = log_path_arg.getValue();

        std::vector<std::string> host = host_arg.getValue();
        std::vector<std::uint16_t> port = port_arg.getValue();

        // set default value of multi value cmd params
        if (host.empty()) {
            host = {"127.0.0.1", "127.0.0.1"};
        }
        if (port.empty()) {
            port = {8089, 8090};
        }

        if (host.size() != port.size()) {
            throw std::invalid_argument("host and port must have the same size");
        }

        if (host.size() != 2) {
            throw std::invalid_argument("host and port must have 2 elements");
        }

        // init net
        petace::network::NetParams net_params;
        if (party == 0) {
            net_params.remote_addr = host[0];
            net_params.remote_port = port[1];
            net_params.local_port = port[0];
        } else {
            net_params.remote_addr = host[1];
            net_params.remote_port = port[0];
            net_params.local_port = port[1];
        }
        auto net = petace::network::NetFactory::get_instance().build(petace::network::NetScheme::SOCKET, net_params);

        // init log
        // FLAGS_log_dir = log_path;
        google::InitGoogleLogging(log_path.c_str());
        LogToFileSink log_to_file_sink(log_path);
        google::AddLogSink(&log_to_file_sink);

        if (test_case == "np_ot") {
            np_ot_bench(net, party, test_number);
        } else if (test_case == "iknp_ot") {
            iknp_ot_bench(net, party, test_number);
        } else if (test_case == "kkrt_ot") {
            kkrt_ot_bench(net, party, test_number);
        } else if (test_case == "all") {
            np_ot_bench(net, party, test_number);
            iknp_ot_bench(net, party, test_number);
            kkrt_ot_bench(net, party, test_number);
        }
        google::RemoveLogSink(&log_to_file_sink);
        google::ShutdownGoogleLogging();
    } catch (TCLAP::ArgException& e) {
        std::cerr << "err: " << e.error() << " for arg " << e.argId() << std::endl;
        return 1;
    } catch (std::exception& e) {
        std::cerr << "err: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "unknown error" << std::endl;
        return 1;
    }
    return 0;
}
