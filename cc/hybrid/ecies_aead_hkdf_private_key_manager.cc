// Copyright 2017 Google Inc.
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
//
///////////////////////////////////////////////////////////////////////////////

#include "cc/hybrid/ecies_aead_hkdf_private_key_manager.h"

#include <map>

#include "cc/hybrid_decrypt.h"
#include "cc/key_manager.h"
#include "cc/hybrid/ecies_aead_hkdf_hybrid_decrypt.h"
#include "cc/util/errors.h"
#include "cc/util/status.h"
#include "cc/util/statusor.h"
#include "cc/util/validation.h"
#include "google/protobuf/message.h"
#include "proto/ecies_aead_hkdf.pb.h"
#include "proto/tink.pb.h"

using google::crypto::tink::EciesAeadHkdfPrivateKey;
using google::crypto::tink::EciesAeadHkdfKeyFormat;
using google::crypto::tink::EciesAeadHkdfParams;
using google::crypto::tink::KeyData;
using google::crypto::tink::KeyTemplate;
using google::protobuf::Message;
using crypto::tink::util::Status;
using crypto::tink::util::StatusOr;

namespace crypto {
namespace tink {

constexpr char EciesAeadHkdfPrivateKeyManager::kKeyTypePrefix[];
constexpr char EciesAeadHkdfPrivateKeyManager::kKeyType[];

const std::string& EciesAeadHkdfPrivateKeyManager::get_key_type() const {
  return key_type_;
}

uint32_t EciesAeadHkdfPrivateKeyManager::get_version() const {
  return 0;
}

StatusOr<std::unique_ptr<HybridDecrypt>>
EciesAeadHkdfPrivateKeyManager::GetPrimitive(const KeyData& key_data) const {
  if (DoesSupport(key_data.type_url())) {
    EciesAeadHkdfPrivateKey ecies_private_key;
    if (!ecies_private_key.ParseFromString(key_data.value())) {
      return ToStatusF(crypto::tink::util::error::INVALID_ARGUMENT,
                       "Could not parse key_data.value as key type '%s'.",
                       key_data.type_url().c_str());
    }
    return GetPrimitiveImpl(ecies_private_key);
  } else {
    return ToStatusF(crypto::tink::util::error::INVALID_ARGUMENT,
                     "Key type '%s' is not supported by this manager.",
                     key_data.type_url().c_str());
  }
}

StatusOr<std::unique_ptr<HybridDecrypt>>
EciesAeadHkdfPrivateKeyManager::GetPrimitive(const Message& key) const {
  std::string key_type =
      std::string(kKeyTypePrefix) + key.GetDescriptor()->full_name();
  if (DoesSupport(key_type)) {
    const EciesAeadHkdfPrivateKey& ecies_private_key =
        reinterpret_cast<const EciesAeadHkdfPrivateKey&>(key);
    return GetPrimitiveImpl(ecies_private_key);
  } else {
    return ToStatusF(crypto::tink::util::error::INVALID_ARGUMENT,
                     "Key type '%s' is not supported by this manager.",
                     key_type.c_str());
  }
}

StatusOr<std::unique_ptr<HybridDecrypt>>
EciesAeadHkdfPrivateKeyManager::GetPrimitiveImpl(
    const EciesAeadHkdfPrivateKey& ecies_private_key) const {
  Status status = Validate(ecies_private_key);
  if (!status.ok()) return status;
  auto ecies_result = EciesAeadHkdfHybridDecrypt::New(ecies_private_key);
  if (!ecies_result.ok()) return ecies_result.status();
  return std::move(ecies_result.ValueOrDie());
}

StatusOr<std::unique_ptr<Message>> EciesAeadHkdfPrivateKeyManager::NewKey(
    const KeyTemplate& key_template) const {
  return crypto::tink::util::Status(crypto::tink::util::error::UNIMPLEMENTED, "not implemented yet");
}

Status EciesAeadHkdfPrivateKeyManager::Validate(
    const EciesAeadHkdfParams& params) const {
  return Status::OK;
}

Status EciesAeadHkdfPrivateKeyManager::Validate(
    const EciesAeadHkdfPrivateKey& key) const {
  Status status = ValidateVersion(key.version(), get_version());
  if (!status.ok()) return status;
  return Validate(key.public_key().params());
}

Status EciesAeadHkdfPrivateKeyManager::Validate(
    const EciesAeadHkdfKeyFormat& key_format) const {
  return Status::OK;
}

}  // namespace tink
}  // namespace crypto
