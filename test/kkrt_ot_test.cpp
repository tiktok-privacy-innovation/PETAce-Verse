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
#include "verse/n-choose-one/kkrt/kkrt_nco_ot_ext.h"
#include "verse/util/common.h"
#include "verse/util/defines.h"
#include "verse/verse_factory.h"

class KkrtOtTest : public ::testing::Test {
public:
    void kkrt_ot(bool is_sender, petace::verse::VerseParams& params) {
        std::size_t ext_ot_size = 500;

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
        for (std::size_t i = 0; i < 4; i++) {
            base_choices_.emplace_back(petace::verse::read_block_from_dev_urandom());
        }
        for (std::size_t i = 0; i < ext_ot_size; i++) {
            ext_choices_.emplace_back(_mm_set_epi64x(ext_ot_size - i, i));
        }

        auto npot_receiver = petace::verse::VerseFactory<petace::verse::BaseOtReceiver>::get_instance().build(
                petace::verse::OTScheme::NaorPinkasReceiver, params);
        auto kkrt_sender = petace::verse::VerseFactory<petace::verse::NcoOtExtSender>::get_instance().build(
                petace::verse::OTScheme::KkrtSender, params);

        auto npot_sender = petace::verse::VerseFactory<petace::verse::BaseOtSender>::get_instance().build(
                petace::verse::OTScheme::NaorPinkasSender, params);
        auto kkrt_receiver = petace::verse::VerseFactory<petace::verse::NcoOtExtReceiver>::get_instance().build(
                petace::verse::OTScheme::KkrtReceiver, params);

        if (is_sender) {
            npot_receiver->receive(net, base_choices_, base_recv_ots);
            kkrt_sender->set_base_ots(base_choices_, base_recv_ots);
            kkrt_sender->send(net, ext_ot_size);
            msg0_.resize(ext_ot_size);
            for (std::size_t i = 0; i < ext_ot_size; i++) {
                kkrt_sender->encode(i, ext_choices_[i], msg0_[i]);
            }
            petace::verse::send_block(net, msg0_.data(), msg0_.size());
        } else {
            npot_sender->send(net, base_send_ots);
            kkrt_receiver->set_base_ots(base_send_ots);
            kkrt_receiver->receive(net, ext_choices_, msg1_);

            msg0_.resize(ext_ot_size);
            petace::verse::recv_block(net, msg0_.data(), msg0_.size());
        }
    }

public:
    std::vector<petace::verse::block> base_choices_;
    std::vector<petace::verse::block> ext_choices_;
    std::vector<petace::verse::block> msg0_;
    std::vector<petace::verse::block> msg1_;
};

TEST_F(KkrtOtTest, kkrt_ot) {
    pid_t pid;
    int status;

    pid = fork();
    if (pid < 0) {
        status = -1;
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        petace::verse::VerseParams params;
        params.base_ot_sizes = 512;
        kkrt_ot(true, params);
        exit(EXIT_SUCCESS);
    } else {
        petace::verse::VerseParams params;
        params.base_ot_sizes = 512;
        kkrt_ot(false, params);
        while (waitpid(pid, &status, 0) < 0) {
            if (errno != EINTR) {
                status = -1;
                break;
            }
        }
        for (std::size_t i = 0; i < msg0_.size(); i++) {
            ASSERT_EQ(msg1_[i][0], msg0_[i][0]);
            ASSERT_EQ(msg1_[i][1], msg0_[i][1]);
        }
        return;
    }
}

TEST_F(KkrtOtTest, kkrt_ot_except) {
    pid_t pid;
    int status;

    pid = fork();
    if (pid < 0) {
        status = -1;
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        petace::verse::VerseParams params;
        params.base_ot_sizes = 500;
        EXPECT_THROW(kkrt_ot(true, params), std::invalid_argument);
        exit(EXIT_SUCCESS);
    } else {
        petace::verse::VerseParams params;
        params.base_ot_sizes = 500;
        EXPECT_THROW(kkrt_ot(false, params), std::invalid_argument);
        while (waitpid(pid, &status, 0) < 0) {
            if (errno != EINTR) {
                status = -1;
                break;
            }
        }
        return;
    }
}
