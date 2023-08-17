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

#include <stdlib.h>
#include <unistd.h>

#include <array>
#include <memory>
#include <string>

#include "gtest/gtest.h"

#include "network/net_factory.h"
#include "solo/prng.h"

#include "verse/base-ot/naor-pinkas-ot/naor_pinkas_ot.h"
#include "verse/two-choose-one/iknp/iknp_ot_ext.h"
#include "verse/util/common.h"
#include "verse/util/defines.h"
#include "verse/verse_factory.h"

class IKNPOtTest : public ::testing::Test {
public:
    void iknp_ot(bool is_sender) {
        petace::verse::VerseParams params;
        params.base_ot_sizes = 128;
        params.ext_ot_sizes = 1024;

        petace::network::NetParams net_params;
        if (is_sender) {
            net_params.remote_addr = "127.0.0.1";
            net_params.remote_port = 8890;
            net_params.local_addr = "127.0.0.1";
            net_params.local_port = 8891;
        } else {
            net_params.remote_addr = "127.0.0.1";
            net_params.remote_port = 8891;
            net_params.local_addr = "127.0.0.1";
            net_params.local_port = 8890;
        }

        auto net = petace::network::NetFactory::get_instance().build(petace::network::NetScheme::SOCKET, net_params);

        std::vector<petace::verse::block> base_recv_ots;
        std::vector<std::array<petace::verse::block, 2>> base_send_ots;
        base_choices_.emplace_back(petace::verse::read_block_from_dev_urandom());
        for (std::size_t i = 0; i < 8; i++) {
            ext_choices_.emplace_back(petace::verse::read_block_from_dev_urandom());
        }

        auto npot_receiver = petace::verse::VerseFactory<petace::verse::BaseOtReceiver>::get_instance().build(
                petace::verse::OTScheme::NaorPinkasReceiver, params);
        auto iknp_sender = petace::verse::VerseFactory<petace::verse::OtExtSender>::get_instance().build(
                petace::verse::OTScheme::IknpSender, params);

        auto npot_sender = petace::verse::VerseFactory<petace::verse::BaseOtSender>::get_instance().build(
                petace::verse::OTScheme::NaorPinkasSender, params);
        auto iknp_receiver = petace::verse::VerseFactory<petace::verse::OtExtReceiver>::get_instance().build(
                petace::verse::OTScheme::IknpReceiver, params);

        msg_.clear();
        msgs_.clear();
        if (is_sender) {
            npot_receiver->receive(net, base_choices_, base_recv_ots);
            iknp_sender->set_base_ots(base_choices_, base_recv_ots);
            iknp_sender->send(net, msgs_);
            petace::verse::send_block(net, &msgs_[0][0], msgs_.size() * 2);
        } else {
            npot_sender->send(net, base_send_ots);
            iknp_receiver->set_base_ots(base_send_ots);
            iknp_receiver->receive(net, ext_choices_, msg_);
            msgs_.resize(params.ext_ot_sizes);
            petace::verse::recv_block(net, &msgs_[0][0], msgs_.size() * 2);
        }
    }

public:
    std::vector<petace::verse::block> base_choices_;
    std::vector<petace::verse::block> ext_choices_;
    std::vector<std::array<petace::verse::block, 2>> msgs_;
    std::vector<petace::verse::block> msg_;
};

TEST_F(IKNPOtTest, iknp_ot) {
    pid_t pid;
    int status;

    pid = fork();
    if (pid < 0) {
        status = -1;
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        iknp_ot(true);
        exit(EXIT_SUCCESS);
    } else {
        iknp_ot(false);
        while (waitpid(pid, &status, 0) < 0) {
            if (errno != EINTR) {
                status = -1;
                break;
            }
        }
        for (std::size_t i = 0; i < msg_.size(); i++) {
            ASSERT_EQ(msg_[i][0], msgs_[i][petace::verse::bit_from_blocks(ext_choices_, i)][0]);
            ASSERT_EQ(msg_[i][1], msgs_[i][petace::verse::bit_from_blocks(ext_choices_, i)][1]);
        }
        return;
    }
}
