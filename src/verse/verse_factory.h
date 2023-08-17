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

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <utility>

#include "verse/base-ot/base_ot_receiver.h"
#include "verse/base-ot/base_ot_sender.h"
#include "verse/base-ot/naor-pinkas-ot/naor_pinkas_ot.h"
#include "verse/two-choose-one/iknp/iknp_ot_ext.h"
#include "verse/two-choose-one/ot_ext_receiver.h"
#include "verse/two-choose-one/ot_ext_sender.h"

namespace petace {
namespace verse {

enum class OTScheme : std::uint32_t { NaorPinkasSender = 0, NaorPinkasReceiver = 1, IknpSender = 2, IknpReceiver = 3 };

template <class T>
using VerseCreator = std::function<std::unique_ptr<T>(const VerseParams& params)>;

/**
 * @brief Oblivious transfer (extension) factory.
 *
 * @see Refer to README.md for more details.
 *
 * @par Example.
 * Refer to example.cpp.
 */
template <class T>
class VerseFactory {
public:
    /**
     * @brief Get a factory singleton.
     *
     * @return Return factory instance.
     */
    static VerseFactory& get_instance() {
        static VerseFactory<T> factory;
        return factory;
    }

    /**
     * @brief Get ot protocol object based on ot scheme.
     *
     * @param scheme: Name of the ot protocol
     * @param params: Parameters of the ot protocol
     */
    std::unique_ptr<T> build(const OTScheme& scheme, const VerseParams& params) {
        auto where = creator_map_.find(scheme);
        if (where == creator_map_.end()) {
            throw std::invalid_argument("verse creator is not registered.");
        }
        return where->second(params);
    }

    /**
     * @brief Register new ot protocol
     *
     * @param scheme: Name of the ot protocol
     * @param creator: Object of the ot protocol
     */
    void register_verse(const OTScheme& scheme, VerseCreator<T> creator) {
        creator_map_.insert(std::make_pair(scheme, creator));
    }

protected:
    VerseFactory() {
    }
    ~VerseFactory() {
    }
    VerseFactory(const VerseFactory&) = delete;
    VerseFactory& operator=(const VerseFactory&) = delete;
    VerseFactory(VerseFactory&&) = delete;
    VerseFactory& operator=(VerseFactory&&) = delete;

private:
    std::map<OTScheme, VerseCreator<T>> creator_map_;
};

/**
 * @brief Register oblivious transfer (extension) instance.
 *
 * @see Refer to README.md for more details.
 *
 * @par Example.
 * Refer to example.cpp.
 */
template <class T>
class VerseRegistrar {
public:
    explicit VerseRegistrar(const OTScheme& scheme, VerseCreator<T> creator) {
        VerseFactory<T>::get_instance().register_verse(scheme, creator);
    }
};

#define REGISTER_VERSE_BASE_OT_SENDER(scheme, creator) \
    static VerseRegistrar<BaseOtSender> registrar__naorpinkas_sender__object(scheme, creator);
#define REGISTER_VERSE_BASE_OT_RECEIVER(scheme, creator) \
    static VerseRegistrar<BaseOtReceiver> registrar__naorpinkas_receiver__object(scheme, creator);
#define REGISTER_VERSE_EXTOT_SENDER(scheme, creator) \
    static VerseRegistrar<OtExtSender> registrar__iknp_sender__object(scheme, creator);
#define REGISTER_VERSE_EXTOT_RECEIVER(scheme, creator) \
    static VerseRegistrar<OtExtReceiver> registrar__iknp_receiver__object(scheme, creator);

REGISTER_VERSE_BASE_OT_SENDER(OTScheme::NaorPinkasSender, create_naor_pinkas_sender)
REGISTER_VERSE_BASE_OT_RECEIVER(OTScheme::NaorPinkasReceiver, create_naor_pinkas_receiver)
REGISTER_VERSE_EXTOT_SENDER(OTScheme::IknpSender, create_iknp_ext_sender)
REGISTER_VERSE_EXTOT_RECEIVER(OTScheme::IknpReceiver, create_iknp_ext_receiver)

}  // namespace verse
}  // namespace petace
