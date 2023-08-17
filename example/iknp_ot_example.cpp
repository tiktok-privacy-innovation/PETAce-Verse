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

void iknp_ot_example(std::size_t party_id, std::string local_addr, std::size_t local_port, std::string remote_addr,
        std::size_t remote_port) {
    petace::network::NetParams net_params;
    net_params.remote_addr = remote_addr;
    net_params.remote_port = static_cast<std::uint16_t>(remote_port);
    net_params.local_addr = local_addr;
    net_params.local_port = static_cast<std::uint16_t>(local_port);
    auto net = petace::network::NetFactory::get_instance().build(petace::network::NetScheme::SOCKET, net_params);

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
    if (party_id == 0) {
        npot_receiver->receive(net, base_choices, base_recv_ots);
        iknp_sender->set_base_ots(base_choices, base_recv_ots);
        iknp_sender->send(net, send_msgs);

        std::cout << "sender messages" << std::endl;
        for (std::size_t i = 0; i < params.ext_ot_sizes; i++) {
            std::cout << std::hex << send_msgs[i][0][0] << "|" << send_msgs[i][0][1] << "--" << send_msgs[i][1][0]
                      << "|" << send_msgs[i][1][1] << std::endl;
        }
    } else {
        npot_sender->send(net, base_send_ots);
        iknp_receiver->set_base_ots(base_send_ots);
        iknp_receiver->receive(net, ext_choices, recv_msgs);
        std::cout << "reciver messages" << std::endl;
        for (std::size_t i = 0; i < params.ext_ot_sizes; i++) {
            std::cout << std::hex << recv_msgs[i][0] << "|" << recv_msgs[i][1] << "--"
                      << petace::verse::bit_from_blocks(ext_choices, i) << std::endl;
        }
    }
}
