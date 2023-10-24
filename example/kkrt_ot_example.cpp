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

#include <iostream>
#include <vector>

#include "example.h"

#include "network/net_factory.h"

#include "verse/util/common.h"
#include "verse/verse_factory.h"

void kkrt_ot_example(std::size_t party_id, std::string local_addr, std::size_t local_port, std::string remote_addr,
        std::size_t remote_port) {
    petace::network::NetParams net_params;
    net_params.remote_addr = remote_addr;
    net_params.remote_port = static_cast<std::uint16_t>(remote_port);
    net_params.local_addr = local_addr;
    net_params.local_port = static_cast<std::uint16_t>(local_port);
    auto net = petace::network::NetFactory::get_instance().build(petace::network::NetScheme::SOCKET, net_params);

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
    if (party_id == 0) {
        npot_receiver->receive(net, base_choices, base_recv_ots);
        kkrt_sender->set_base_ots(base_choices, base_recv_ots);
        kkrt_sender->send(net, params.ext_ot_sizes);

        std::cout << "sender messages" << std::endl;
        petace::verse::block output;
        for (std::size_t i = 0; i < 16; i++) {
            std::cout << std::dec << "ot index: " << i << std::endl;
            for (std::size_t j = 0; j < 16; j++) {
                kkrt_sender->encode(i, _mm_set_epi64x(0, j), output);
                std::cout << std::hex << output[0] << "|" << output[1] << std::endl;
            }
            std::cout << std::endl;
        }
    } else {
        npot_sender->send(net, base_send_ots);
        kkrt_receiver->set_base_ots(base_send_ots);
        kkrt_receiver->receive(net, ext_choices, recv_msgs);
        std::cout << "reciver messages" << std::endl;
        for (std::size_t i = 0; i < 16; i++) {
            std::cout << std::dec << "ot index: " << i << std::endl;
            std::cout << std::hex << recv_msgs[i][0] << "|" << recv_msgs[i][1] << "--" << ext_choices[i][0]
                      << std::endl;
        }
    }
}
