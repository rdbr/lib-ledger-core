/*
 *
 * RippleLikeTransactionApi
 *
 * Created by El Khalil Bellakrid on 06/01/2019.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ledger
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */


#include "RippleLikeTransactionApi.h"
#include <wallet/common/Amount.h>
#include <wallet/common/AbstractAccount.hpp>
#include <wallet/common/AbstractWallet.hpp>
#include <ripple/RippleLikeAddress.h>
#include <bytes/BytesWriter.h>
#include <bytes/BytesReader.h>
#include <bytes/RLP/RLPListEncoder.h>
#include <bytes/RLP/RLPStringEncoder.h>
#include <utils/hex.h>
#include <api_impl/BigIntImpl.hpp>

namespace ledger {
    namespace core {

        RippleLikeTransactionApi::RippleLikeTransactionApi(const api::Currency &currency) {
            _currency = currency;
        }

        RippleLikeTransactionApi::RippleLikeTransactionApi(const std::shared_ptr<OperationApi> &operation) {
            auto &tx = operation->getBackend().rippleTransaction.getValue();
            _time = tx.receivedAt;

            if (tx.block.nonEmpty()) {
                _block = std::make_shared<RippleLikeBlockApi>(tx.block.getValue());
            } else {
                _block = nullptr;
            }

            _hash = tx.hash;

            _currency = operation->getAccount()->getWallet()->getCurrency();

            _fees = std::make_shared<Amount>(_currency, 0, tx.fees);
            _value = std::make_shared<Amount>(_currency, 0, tx.value);

            _receiver = RippleLikeAddress::fromBase58(tx.receiver, _currency);
            _sender = RippleLikeAddress::fromBase58(tx.sender, _currency);

        }

        std::string RippleLikeTransactionApi::getHash() {
            return _hash;
        }

        std::shared_ptr<api::Amount> RippleLikeTransactionApi::getFees() {
            return _fees;
        }

        std::shared_ptr<api::RippleLikeAddress> RippleLikeTransactionApi::getReceiver() {
            return _receiver;
        }

        std::shared_ptr<api::RippleLikeAddress> RippleLikeTransactionApi::getSender() {
            return _sender;
        }

        std::shared_ptr<api::Amount> RippleLikeTransactionApi::getValue() {
            return _value;
        }

        std::chrono::system_clock::time_point RippleLikeTransactionApi::getDate() {
            return _time;
        }

        std::shared_ptr<api::BigInt> RippleLikeTransactionApi::getLedgerSequence() {
            return _ledgerSequence;
        }

        std::shared_ptr<api::BigInt> RippleLikeTransactionApi::getSequence() {
            return _sequence;
        }

        std::vector<uint8_t> RippleLikeTransactionApi::getSigningPubKey() {
            return _signingPubKey;
        }

        void RippleLikeTransactionApi::setSignature(const std::vector<uint8_t> &rSignature,
                                                    const std::vector<uint8_t> &sSignature) {
            _rSignature = rSignature;
            _sSignature = sSignature;
        }

        void RippleLikeTransactionApi::setDERSignature(const std::vector<uint8_t> &signature) {
            BytesReader reader(signature);
            //DER prefix
            reader.readNextByte();
            //Total length
            reader.readNextVarInt();
            //Nb of elements for R
            reader.readNextByte();
            //R length
            auto rSize = reader.readNextVarInt();
            if (rSize > 0 && reader.peek() == 0x00) {
                reader.readNextByte();
                _rSignature = reader.read(rSize - 1);
            } else {
                _rSignature = reader.read(rSize);
            }
            //Nb of elements for S
            reader.readNextByte();
            //S length
            auto sSize = reader.readNextVarInt();
            if (sSize > 0 && reader.peek() == 0x00) {
                reader.readNextByte();
                _sSignature = reader.read(sSize - 1);
            } else {
                _sSignature = reader.read(sSize);
            }
        }

        //Field ID References:
        // https://github.com/ripple/rippled/blob/master/src/ripple/protocol/SField.h#L57-L74
        // and https://github.com/ripple/rippled/blob/72e6005f562a8f0818bc94803d222ac9345e1e40/src/ripple/protocol/impl/SField.cpp#L72-L266
        std::vector<uint8_t> RippleLikeTransactionApi::serialize() {
            BytesWriter writer;

            //1 byte TransactionType Field ID:   Type Code = 1, Field Code = 2
            writer.writeByte(0x12);
            //2 bytes TransactionType ("Payment")
            writer.writeByteArray({0x00, 0x00});

            //1 byte Flags Field ID:   Type Code = 2, Field Code = 2
            writer.writeByte(0x22);
            //4 bytes Flags (tfFullyCanonicalSig ?)
            writer.writeByteArray({0x80, 0x00, 0x00, 0x00});

            //1 byte Sequence Field ID:   Type Code = 2, Field Code = 4
            writer.writeByte(0x24);
            //4 bytes Sequence
            auto sequence = _sequence->toString(16);
            writer.writeBeValue<uint32_t>(static_cast<const uint32_t>(_sequence->intValue()));

            //2 bytes LastLedgerSequence Field ID:   Type Code = 2, Field Code = 27
            writer.writeByteArray({0x20, 0x1B});
            //LastLedgerSequence
            writer.writeBeValue<uint32_t>(static_cast<const uint32_t>(_ledgerSequence->intValue()));


            auto maxAmountBound = std::vector<uint8_t>({0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
            auto bigIntMax = BigInt::fromHex(hex::toString(maxAmountBound));

            //1 byte Amount Field ID:   Type Code = 6, Field Code = 1
            writer.writeByte(0x61);
            //8 bytes Amount (with bitwise OR with 0x4000000000000000)
            auto finalValue = BigInt::fromHex(_value->toBigInt()->toString(16)) + bigIntMax;
            writer.writeBeValue<uint64_t>(static_cast<const uint64_t>(finalValue.toUint64()));

            //1 byte Fee Field ID:   Type Code = 6, Field Code = 8
            writer.writeByte(0x68);
            //8 bytes Fees (with bitwise OR with 0x4000000000000000)
            auto finalFees = BigInt::fromHex(_fees->toBigInt()->toString(16)) + bigIntMax;
            writer.writeBeValue<uint64_t>(static_cast<const uint64_t>(finalFees.toUint64()));

            //TODO: !!!find out if this is included in raw unsigned tx or not
            //1 byte Signing pubKey Field ID:   Type Code = 7, Field Code = 3 (STI_VL = 7 type)
            writer.writeByte(0x73);
            //Var bytes Signing pubKey (prefix length)
            writer.writeVarInt(_signingPubKey.size());
            writer.writeByteArray(_signingPubKey);

            if (!_rSignature.empty() && !_sSignature.empty()) {
                //1 byte Signature Field ID:   Type Code = 7, Field Code = 4 (STI_VL = 7 type, and TxnSignature = 4)
                writer.writeByte(0x74);
                //Var bytes Signature (prefix length)
                //Get length of VarInt representing length of R
                BytesWriter rLength;
                rLength.writeVarInt(_rSignature.size());
                //Get length of VarInt representing length of R
                BytesWriter sLength;
                sLength.writeVarInt(_sSignature.size());
                //Get length of VarInt representing length of R and S (plus their stack sizes)
                auto sAndRLengthInt = 1 + rLength.toByteArray().size() + _rSignature.size() + 1 + sLength.toByteArray().size() + _sSignature.size();
                BytesWriter sAndRLength;
                sAndRLength.writeVarInt(sAndRLengthInt);
                //DER Signature = Total Size | DER prefix | Size(S+R) | R StackSize | R Length | R | S StackSize | S Length | S
                auto totalSigLength = 1 + sAndRLength.toByteArray().size() + sAndRLengthInt;
                writer.writeVarInt(totalSigLength);
                //DER prefix
                BytesWriter sigWriter;
                writer.writeByte(0x30);
                //Size of DER signature minus DER prefix | Size(S+R)
                writer.writeVarInt(sAndRLengthInt);
                //R field
                writer.writeByte(0x02); //Nb of stack elements
                writer.writeVarInt(_rSignature.size());
                writer.writeByteArray(_rSignature);
                //S field
                writer.writeByte(0x02); //Nb of stack elements
                writer.writeVarInt(_sSignature.size());
                writer.writeByteArray(_sSignature);
            }

            //1 byte Account Field ID: Type Code = 8, Field Code = 1 (STI_ACCOUNT = 8 type, and Account = 1)
            writer.writeByte(0x81);
            //20 bytes Acount (hash160 of pubKey without 0x00 prefix)
            auto senderHash = _sender->getHash160();
            writer.writeVarInt(senderHash.size());
            writer.writeByteArray(senderHash);
            //1 byte Destination Field ID: Type Code = 8, Field Code = 3 (STI_ACCOUNT = 8 type, and Destination = 3)
            writer.writeByte(0x83);
            //20 bytes Destination (hash160 of pubKey without 0x00 prefix)
            auto receiverHash = _receiver->getHash160();
            writer.writeVarInt(receiverHash.size());
            writer.writeByteArray(receiverHash);
            return writer.toByteArray();
        }

        RippleLikeTransactionApi &RippleLikeTransactionApi::setFees(const std::shared_ptr<BigInt> &fees) {
            if (!fees) {
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT,
                                     "RippleLikeTransactionApi::setFees: Invalid Fees");
            }
            _fees = std::make_shared<Amount>(_currency, 0, *fees);
            return *this;
        }

        RippleLikeTransactionApi &RippleLikeTransactionApi::setValue(const std::shared_ptr<BigInt> &value) {
            if (!value) {
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT,
                                     "RippleLikeTransactionApi::setValue: Invalid Value");
            }
            _value = std::make_shared<Amount>(_currency, 0, *value);
            return *this;
        }

        RippleLikeTransactionApi &RippleLikeTransactionApi::setSequence(const BigInt& sequence) {
            _sequence = std::make_shared<api::BigIntImpl>(sequence);
            return *this;
        }

        RippleLikeTransactionApi & RippleLikeTransactionApi::setLedgerSequence(const BigInt& ledgerSequence) {
            _ledgerSequence = std::make_shared<api::BigIntImpl>(ledgerSequence);
            return *this;
        }

        RippleLikeTransactionApi &RippleLikeTransactionApi::setSender(const std::shared_ptr<api::RippleLikeAddress> &sender) {
            _sender = sender;
            return *this;
        }

        RippleLikeTransactionApi &RippleLikeTransactionApi::setReceiver(const std::shared_ptr<api::RippleLikeAddress> &receiver) {
            _receiver = receiver;
            return *this;
        }

        RippleLikeTransactionApi &RippleLikeTransactionApi::setSigningPubKey(const std::vector<uint8_t> &pubKey) {
            _signingPubKey = pubKey;
            return *this;
        }

    }
}