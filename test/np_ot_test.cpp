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

class NPOtTest : public ::testing::Test {
public:
    void np_ot(bool is_sender) {
        petace::verse::VerseParams params;
        params.base_ot_sizes = 128;

        srandom((unsigned int)time(nullptr));

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

        base_choices_.emplace_back(petace::verse::read_block_from_dev_urandom());

        auto npot_receiver = petace::verse::VerseFactory<petace::verse::BaseOtReceiver>::get_instance().build(
                petace::verse::OTScheme::NaorPinkasReceiver, params);
        auto npot_sender = petace::verse::VerseFactory<petace::verse::BaseOtSender>::get_instance().build(
                petace::verse::OTScheme::NaorPinkasSender, params);

        msg_.clear();
        msgs_.clear();
        if (is_sender) {
            npot_receiver->receive(net, base_choices_, msg_);
            petace::verse::send_block(net, &msg_[0], msg_.size());
            petace::verse::send_block(net, &base_choices_[0], base_choices_.size());
        } else {
            npot_sender->send(net, msgs_);
            msg_.resize(params.base_ot_sizes);
            petace::verse::recv_block(net, &msg_[0], msg_.size());
            petace::verse::recv_block(net, &base_choices_[0], base_choices_.size());
        }
    }

public:
    std::vector<petace::verse::block> base_choices_;
    std::vector<std::array<petace::verse::block, 2>> msgs_;
    std::vector<petace::verse::block> msg_;
};

TEST_F(NPOtTest, np_ot) {
    pid_t pid;
    int status;

    pid = fork();
    if (pid < 0) {
        status = -1;
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        np_ot(true);
        exit(EXIT_SUCCESS);
    } else {
        np_ot(false);
        while (waitpid(pid, &status, 0) < 0) {
            if (errno != EINTR) {
                status = -1;
                break;
            }
        }
        for (std::size_t i = 0; i < msg_.size(); i++) {
            ASSERT_EQ(msg_[i][0], msgs_[i][petace::verse::bit_from_blocks(base_choices_, i)][0]);
            ASSERT_EQ(msg_[i][1], msgs_[i][petace::verse::bit_from_blocks(base_choices_, i)][1]);
        }
        return;
    }
}
