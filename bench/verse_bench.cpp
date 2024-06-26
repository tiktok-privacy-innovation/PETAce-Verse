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

#include "verse_bench.h"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <stdexcept>

#include "glog/logging.h"

#include "verse/util/common.h"

double get_unix_timestamp() {
    std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();

    std::chrono::duration<double> duration_since_epoch =
            std::chrono::duration_cast<std::chrono::duration<double>>(now.time_since_epoch());

    return duration_since_epoch.count();
}

void np_ot_bench(const std::shared_ptr<petace::network::Network>& net, std::size_t party_id, std::size_t test_number) {
    try {
        petace::verse::VerseParams params;
        params.base_ot_sizes = 128;

        std::vector<petace::verse::block> base_recv_ots;
        std::vector<std::array<petace::verse::block, 2>> base_send_ots;
        std::vector<petace::verse::block> base_choices;
        base_choices.emplace_back(petace::verse::read_block_from_dev_urandom());

        auto npot_sender = petace::verse::VerseFactory<petace::verse::BaseOtSender>::get_instance().build(
                petace::verse::OTScheme::NaorPinkasSender, params);
        auto npot_receiver = petace::verse::VerseFactory<petace::verse::BaseOtReceiver>::get_instance().build(
                petace::verse::OTScheme::NaorPinkasReceiver, params);

        double begin = get_unix_timestamp();
        LOG(INFO) << std::fixed << "case np_ot_" << params.base_ot_sizes << "_bench"
                  << " begin " << begin << " " << test_number;

        for (size_t i = 0; i < test_number; i++) {
            if (party_id == 0) {
                npot_sender->send(net, base_send_ots);
            } else {
                npot_receiver->receive(net, base_choices, base_recv_ots);
            }
        }

        double end = get_unix_timestamp();

        LOG(INFO) << std::fixed << "case np_ot_" << params.base_ot_sizes << "_bench"
                  << " end " << end << " " << end - begin << "s " << net->get_bytes_sent() << " "
                  << net->get_bytes_received();
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
    }
}

void iknp_ot_bench(
        const std::shared_ptr<petace::network::Network>& net, std::size_t party_id, std::size_t test_number) {
    try {
        petace::verse::VerseParams params;
        params.base_ot_sizes = 128;
        params.ext_ot_sizes = 1024;
        std::vector<petace::verse::block> base_recv_ots;
        std::vector<std::array<petace::verse::block, 2>> base_send_ots;
        std::vector<petace::verse::block> base_choices;
        std::vector<petace::verse::block> ext_choices;
        base_choices.emplace_back(petace::verse::read_block_from_dev_urandom());
        for (std::size_t i = 0; i < params.ext_ot_sizes / sizeof(petace::verse::block); i++) {
            ext_choices.emplace_back(petace::verse::read_block_from_dev_urandom());
        }

        auto npot_sender = petace::verse::VerseFactory<petace::verse::BaseOtSender>::get_instance().build(
                petace::verse::OTScheme::NaorPinkasSender, params);
        auto npot_receiver = petace::verse::VerseFactory<petace::verse::BaseOtReceiver>::get_instance().build(
                petace::verse::OTScheme::NaorPinkasReceiver, params);
        auto iknp_sender = petace::verse::VerseFactory<petace::verse::OtExtSender>::get_instance().build(
                petace::verse::OTScheme::IknpSender, params);
        auto iknp_receiver = petace::verse::VerseFactory<petace::verse::OtExtReceiver>::get_instance().build(
                petace::verse::OTScheme::IknpReceiver, params);

        std::vector<std::array<petace::verse::block, 2>> send_msgs;
        std::vector<petace::verse::block> recv_msgs;

        double begin = get_unix_timestamp();
        LOG(INFO) << std::fixed << "case iknp_ot_" << params.base_ot_sizes << "_" << params.ext_ot_sizes << "_bench"
                  << " begin " << begin << " " << test_number;

        if (party_id == 0) {
            npot_receiver->receive(net, base_choices, base_recv_ots);
            iknp_sender->set_base_ots(base_choices, base_recv_ots);
            for (size_t i = 0; i < test_number; i++) {
                iknp_sender->send(net, send_msgs);
            }
        } else {
            npot_sender->send(net, base_send_ots);
            iknp_receiver->set_base_ots(base_send_ots);
            for (size_t i = 0; i < test_number; i++) {
                iknp_receiver->receive(net, ext_choices, recv_msgs);
            }
        }

        double end = get_unix_timestamp();

        LOG(INFO) << std::fixed << "case np_ot_" << params.base_ot_sizes << "_" << params.ext_ot_sizes << "_bench"
                  << " end " << end << " " << end - begin << "s " << net->get_bytes_sent() << " "
                  << net->get_bytes_received();
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
    }
}

void kkrt_ot_bench(
        const std::shared_ptr<petace::network::Network>& net, std::size_t party_id, std::size_t test_number) {
    try {
        petace::verse::VerseParams params;
        params.base_ot_sizes = 512;
        params.ext_ot_sizes = 1024;
        std::vector<petace::verse::block> base_recv_ots;
        std::vector<std::array<petace::verse::block, 2>> base_send_ots;
        std::vector<petace::verse::block> base_choices;
        std::vector<petace::verse::block> ext_choices;
        for (std::size_t i = 0; i < 4; i++) {
            base_choices.emplace_back(petace::verse::read_block_from_dev_urandom());
        }
        for (std::size_t i = 0; i < params.ext_ot_sizes; i++) {
            ext_choices.emplace_back(_mm_set_epi64x(0, i));
        }

        auto npot_sender = petace::verse::VerseFactory<petace::verse::BaseOtSender>::get_instance().build(
                petace::verse::OTScheme::NaorPinkasSender, params);
        auto npot_receiver = petace::verse::VerseFactory<petace::verse::BaseOtReceiver>::get_instance().build(
                petace::verse::OTScheme::NaorPinkasReceiver, params);
        auto kkrt_sender = petace::verse::VerseFactory<petace::verse::NcoOtExtSender>::get_instance().build(
                petace::verse::OTScheme::KkrtSender, params);
        auto kkrt_receiver = petace::verse::VerseFactory<petace::verse::NcoOtExtReceiver>::get_instance().build(
                petace::verse::OTScheme::KkrtReceiver, params);

        std::vector<petace::verse::block> recv_msgs;

        double begin = get_unix_timestamp();
        LOG(INFO) << std::fixed << "case kkrt_ot_" << params.base_ot_sizes << "_" << params.ext_ot_sizes << "_bench"
                  << " begin " << begin << " " << test_number;

        if (party_id == 0) {
            npot_receiver->receive(net, base_choices, base_recv_ots);
            kkrt_sender->set_base_ots(base_choices, base_recv_ots);
            for (size_t i = 0; i < test_number; i++) {
                kkrt_sender->send(net, params.ext_ot_sizes);
            }
        } else {
            npot_sender->send(net, base_send_ots);
            kkrt_receiver->set_base_ots(base_send_ots);
            for (size_t i = 0; i < test_number; i++) {
                kkrt_receiver->receive(net, ext_choices, recv_msgs);
            }
        }

        double end = get_unix_timestamp();

        LOG(INFO) << std::fixed << "case kkrt_ot_" << params.base_ot_sizes << "_" << params.ext_ot_sizes << "_bench"
                  << " end " << end << " " << end - begin << "s " << net->get_bytes_sent() << " "
                  << net->get_bytes_received();
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
    }
}
