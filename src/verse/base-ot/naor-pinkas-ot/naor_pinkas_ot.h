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

#pragma once

#include <array>
#include <memory>
#include <vector>

#include "network/network.h"
#include "solo/ec_openssl.h"
#include "solo/hash.h"

#include "verse/base-ot/base_ot_receiver.h"
#include "verse/base-ot/base_ot_sender.h"
#include "verse/util/defines.h"

namespace petace {
namespace verse {

/**
 * @brief 1-out-of-2 naor-pinkas ot [sender].
 *
 * @see Refer to README.md for more details.
 *
 * @par Example.
 * Refer to example.cpp.
 */
class NaorPinkasSender : public BaseOtSender {
public:
    explicit NaorPinkasSender(std::size_t base_ot_sizes) : BaseOtSender(base_ot_sizes) {
        ec_ = std::make_shared<EC>(kCurveID, solo::HashScheme::SHA_256);
        solo::PRNGFactory prng_factory(solo::PRNGScheme::AES_ECB_CTR);
        prng_ = prng_factory.create();
    }

    ~NaorPinkasSender() {
    }

    /**
     * @brief The sender gets the random messages of naor-pinkas ot protocol.
     *
     * @param[in] net The network interface (e.g., PETAce-Network interface).
     * @param[out] messages The random output messages of the sender.
     */
    void send(const std::shared_ptr<network::Network>& net, std::vector<std::array<block, 2>>& messages) override;

private:
    std::shared_ptr<EC> ec_ = nullptr;

    std::shared_ptr<solo::PRNG> prng_ = nullptr;
};

/**
 * @brief 1-out-of-2 naor-pinkas ot [receiver].
 *
 * @see Refer to README.md for more details.
 *
 * @par Example.
 * Refer to example.cpp.
 */
class NaorPinkasReceiver : public BaseOtReceiver {
public:
    explicit NaorPinkasReceiver(std::size_t base_ot_sizes) : BaseOtReceiver(base_ot_sizes) {
        ec_ = std::make_shared<EC>(kCurveID, solo::HashScheme::SHA_256);
        solo::PRNGFactory prng_factory(solo::PRNGScheme::AES_ECB_CTR);
        prng_ = prng_factory.create();
    }

    ~NaorPinkasReceiver() {
    }

    /**
     * @brief The receiver gets chosen messages indexed by choices in the naor-pinkas ot protocol.
     *
     * @param[in] net The network interface (e.g., PETAce-Network interface).
     * @param[in] choices The chosen bits of receiver.
     * @param[out] messages The chosen messages indexed by choices.
     */
    void receive(const std::shared_ptr<network::Network>& net, const std::vector<block>& choices,
            std::vector<block>& messages) override;

private:
    std::vector<block> base_choices{};

    std::shared_ptr<EC> ec_ = nullptr;

    std::shared_ptr<solo::PRNG> prng_ = nullptr;
};

inline std::unique_ptr<BaseOtReceiver> create_naor_pinkas_receiver(const VerseParams& params) {
    return std::make_unique<NaorPinkasReceiver>(params.base_ot_sizes);
}

inline std::unique_ptr<BaseOtSender> create_naor_pinkas_sender(const VerseParams& params) {
    return std::make_unique<NaorPinkasSender>(params.base_ot_sizes);
}

}  // namespace verse
}  // namespace petace
