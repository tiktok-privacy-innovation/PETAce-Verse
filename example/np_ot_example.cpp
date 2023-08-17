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

void np_ot_example(std::size_t party_id, std::string local_addr, std::size_t local_port, std::string remote_addr,
        std::size_t remote_port) {
    petace::network::NetParams net_params;
    net_params.remote_addr = remote_addr;
    net_params.remote_port = static_cast<std::uint16_t>(remote_port);
    net_params.local_addr = local_addr;
    net_params.local_port = static_cast<std::uint16_t>(local_port);
    auto net = petace::network::NetFactory::get_instance().build(petace::network::NetScheme::SOCKET, net_params);

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

    if (party_id == 0) {
        npot_sender->send(net, base_send_ots);
        std::cout << "sender messages" << std::endl;
        for (std::size_t i = 0; i < params.base_ot_sizes; i++) {
            std::cout << std::hex << base_send_ots[i][0][0] << "|" << base_send_ots[i][0][1] << "--"
                      << base_send_ots[i][1][0] << "|" << base_send_ots[i][1][1] << std::endl;
        }
    } else {
        npot_receiver->receive(net, base_choices, base_recv_ots);
        std::cout << "reciver messages" << std::endl;
        for (std::size_t i = 0; i < params.base_ot_sizes; i++) {
            std::cout << std::hex << base_recv_ots[i][0] << "|" << base_recv_ots[i][1] << "--"
                      << petace::verse::bit_from_blocks(base_choices, i) << std::endl;
        }
    }
}
