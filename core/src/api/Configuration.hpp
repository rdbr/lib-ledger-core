// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from configuration.djinni

#pragma once

#include <string>

namespace ledger { namespace core { namespace api {

class Configuration {
public:
    virtual ~Configuration() {}

    /** Selects the keychain engine (P2PKH, P2SH...) */
    static std::string const KEYCHAIN_ENGINE;

    /** Sets the observable range for HD keychains (BIP32 based) */
    static std::string const KEYCHAIN_OBSERVABLE_RANGE;

    /** Selects the blockchain explorer engine (Ledger's API, Electrum server, RPC) */
    static std::string const BLOCKCHAIN_EXPLORER_ENGINE;

    /** Sets the API endpoint for API based engine (Ledger's API) */
    static std::string const BLOCKCHAIN_EXPLORER_API_ENDPOINT;

    /** Selects the blockchain observer engine (Ledger's API) */
    static std::string const BLOCKCHAIN_OBSERVER_ENGINE;

    /** Selects the synchronization engine */
    static std::string const SYNCHRONIZATION_ENGINE;
};

} } }  // namespace ledger::core::api
