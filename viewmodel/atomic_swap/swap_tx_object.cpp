// Copyright 2019 The Beam Team
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "swap_tx_object.h"
#include "wallet/swaps/common.h"
#include "wallet/swaps/swap_transaction.h"
#include "viewmodel/qml_globals.h"
#include "model/app_model.h"

using namespace beam;
using namespace beam::wallet;

namespace
{
    QString convertBeamHeightDiffToTime(int32_t dt)
    {
        if (dt <= 0)
        {
            return "";
        }
        const int32_t minute_s = 60;
        const int32_t quantum_s = 5 * minute_s;
        int32_t time_s = dt * beam::Rules().DA.Target_s;
        time_s = (time_s + (quantum_s >> 1)) / quantum_s;
        time_s *= quantum_s;
        return beamui::getEstimateTimeStr(time_s);
    }


    QString getWaitingPeerStr(const beam::wallet::TxParameters& txParameters)
    {
        auto minHeight = txParameters.GetParameter<beam::Height>(TxParameterID::MinHeight);
        auto responseTime = txParameters.GetParameter<beam::Height>(TxParameterID::PeerResponseTime);
        QString time = "";
        if (minHeight && responseTime)
        {
            time = convertBeamHeightDiffToTime(*minHeight + *responseTime - AppModel::getInstance().getWallet()->getCurrentHeight());
        }
        //% "If the peer won't get online in %1, the swap will be canceled automatically"
        return qtTrId("swap-tx-state-initial").arg(time);
    }

    QString getInProgressNormalStr(const beam::wallet::TxParameters& txParameters)
    {
        auto minHeightLock = txParameters.GetParameter<beam::Height>(TxParameterID::MinHeight, BEAM_LOCK_TX);
        auto lifeTime = txParameters.GetParameter<beam::Height>(TxParameterID::Lifetime, BEAM_LOCK_TX);
        QString time = "";
        if (minHeightLock && lifeTime)
        {
            time = convertBeamHeightDiffToTime(*minHeightLock + *lifeTime - AppModel::getInstance().getWallet()->getCurrentHeight());
        }
        //% "The swap is expected to complete in %1"
        return qtTrId("swap-tx-state-in-progress-normal").arg(time);
    }

    QString getInProgressRefundingStr(const beam::wallet::TxParameters& txParameters)
    {
        auto maxHeight = txParameters.GetParameter<beam::Height>(TxParameterID::MaxHeight, BEAM_REFUND_TX);
        auto isBeamSide = txParameters.GetParameter<bool>(TxParameterID::AtomicSwapIsBeamSide);
        auto swapCoin = txParameters.GetParameter<beam::wallet::AtomicSwapCoin>(TxParameterID::AtomicSwapCoin);
        QString coin = "";

        if (isBeamSide && *isBeamSide)
        {
            coin = "beam";
        }
        else if (swapCoin)
        {
            coin = beamui::toString(beamui::convertSwapCoinToCurrency(*swapCoin));
        }
        QString time = "";
        if (maxHeight)
        {
            time = convertBeamHeightDiffToTime(*maxHeight - AppModel::getInstance().getWallet()->getCurrentHeight());
        }
        //% "Your %2 will be refunded in %1"
        return qtTrId("swap-tx-state-in-progress-refunding").arg(time).arg(coin);
    }
}

SwapTxObject::SwapTxObject(QObject* parent)
        : TxObject(parent),
          m_isBeamSide(boost::none),
          m_swapCoin(boost::none)
{
}

SwapTxObject::SwapTxObject(const TxDescription& tx, QObject* parent/* = nullptr*/)
        : TxObject(tx, parent),
          m_isBeamSide(m_tx.GetParameter<bool>(TxParameterID::AtomicSwapIsBeamSide)),
          m_swapCoin(m_tx.GetParameter<AtomicSwapCoin>(TxParameterID::AtomicSwapCoin))
{
}

auto SwapTxObject::isBeamSideSwap() const -> bool
{
    if (m_isBeamSide)
    {
        return *m_isBeamSide;
    }
    else return false;
}

QString SwapTxObject::getStatus() const
{
    switch (m_tx.m_status)
    {
    case wallet::TxStatus::Pending:
        return "pending";

    case wallet::TxStatus::Registering:
    case wallet::TxStatus::InProgress:
        return "in progress";

    case wallet::TxStatus::Completed:
        return "completed";

    case wallet::TxStatus::Canceled:
        return "canceled";

    case wallet::TxStatus::Failed:
        {
            auto failureReason = m_tx.GetParameter<TxFailureReason>(TxParameterID::InternalFailureReason);
            if (failureReason && *failureReason == TxFailureReason::TransactionExpired)
            {
                return "expired";
            }
            return "failed";
        }

    default:
        assert(false && "Unknown TX status!");
        return "unknown";
    }
}

bool SwapTxObject::isExpired() const
{
    auto failureReason = m_tx.GetParameter<TxFailureReason>(TxParameterID::InternalFailureReason);

    return  m_tx.m_status == wallet::TxStatus::Failed &&
            failureReason &&
            *failureReason == TxFailureReason::TransactionExpired;
}

bool SwapTxObject::isInProgress() const
{
    return  m_tx.m_status == wallet::TxStatus::Pending ||
            m_tx.m_status == wallet::TxStatus::Registering ||
            m_tx.m_status == wallet::TxStatus::InProgress;
}

bool SwapTxObject::isPending() const
{
    return m_tx.m_status == wallet::TxStatus::Pending;
}

bool SwapTxObject::isCompleted() const
{
    return m_tx.m_status == wallet::TxStatus::Completed;
}

bool SwapTxObject::isCanceled() const
{
    return m_tx.m_status == wallet::TxStatus::Canceled;
}

bool SwapTxObject::isFailed() const
{
    auto failureReason = m_tx.GetParameter<TxFailureReason>(TxParameterID::InternalFailureReason);

    return  m_tx.m_status == wallet::TxStatus::Failed && !failureReason;
}

bool SwapTxObject::isCancelAvailable() const
{
    // TODO:SWAP link to transaction internal state like in AtomicSwapTransaction::Cancel()
    return  m_tx.m_status == wallet::TxStatus::Pending ||
            m_tx.m_status == wallet::TxStatus::Registering ||
            m_tx.m_status == wallet::TxStatus::InProgress;
}

bool SwapTxObject::isDeleteAvailable() const
{
    return  m_tx.m_status == wallet::TxStatus::Completed ||
            m_tx.m_status == wallet::TxStatus::Canceled ||
            m_tx.m_status == wallet::TxStatus::Failed;
}

auto SwapTxObject::getSwapCoinName() const -> QString
{
    if (m_swapCoin)
    {
        switch (*m_swapCoin)
        {
            case AtomicSwapCoin::Bitcoin:   return toString(beamui::Currencies::Bitcoin);
            case AtomicSwapCoin::Litecoin:  return toString(beamui::Currencies::Litecoin);
            case AtomicSwapCoin::Qtum:      return toString(beamui::Currencies::Qtum);
            case AtomicSwapCoin::Unknown:   return toString(beamui::Currencies::Unknown);
        }
    }
    return QString("unknown");
}

QString SwapTxObject::getSentAmountWithCurrency() const
{
    if (m_type == TxType::AtomicSwap)
    {
        return getSwapAmountWithCurrency(true);
    }
    return m_tx.m_sender ? getAmountWithCurrency() : "";
}

QString SwapTxObject::getSentAmount() const
{
    QString amount = beamui::AmountToUIString(getSentAmountValue());
    return amount == "0" ? "" : amount;
}

beam::Amount SwapTxObject::getSentAmountValue() const
{
    if (m_type == TxType::AtomicSwap)
    {
        return getSwapAmountValue(true);
    }

    return m_tx.m_sender ? m_tx.m_amount : 0;
}

QString SwapTxObject::getReceivedAmountWithCurrency() const
{
    if (m_type == TxType::AtomicSwap)
    {
        return getSwapAmountWithCurrency(false);
    }
    return !m_tx.m_sender ? getAmountWithCurrency() : "";
}

QString SwapTxObject::getReceivedAmount() const
{
    QString amount = beamui::AmountToUIString(getReceivedAmountValue());
    return amount == "0" ? "" : amount;
}

beam::Amount SwapTxObject::getReceivedAmountValue() const
{
    if (m_type == TxType::AtomicSwap)
    {
        return getSwapAmountValue(false);
    }

    return !m_tx.m_sender ? m_tx.m_amount : 0;
}

QString SwapTxObject::getSwapAmountWithCurrency(bool sent) const
{
    if (!m_isBeamSide)
    {
        return "";
    }

    bool s = sent ? !*m_isBeamSide : *m_isBeamSide;
    if (s)
    {
        auto swapAmount = m_tx.GetParameter<Amount>(TxParameterID::AtomicSwapAmount);
        if (swapAmount)
        {
            return AmountToUIString(*swapAmount, beamui::convertSwapCoinToCurrency(*m_swapCoin));
        }
        return "";
    }
    return getAmountWithCurrency();
}

beam::Amount SwapTxObject::getSwapAmountValue(bool sent) const
{
    if (!m_isBeamSide)
    {
        return 0;
    }

    bool s = sent ? !*m_isBeamSide : *m_isBeamSide;
    if (s)
    {
        auto swapAmount = m_tx.GetParameter<Amount>(TxParameterID::AtomicSwapAmount);
        if (swapAmount)
        {
            return *swapAmount;
        }
        return 0;
    }
    return m_tx.m_amount;
}

QString SwapTxObject::getFee() const
{
    if (m_isBeamSide)
    {
        auto fee = m_tx.GetParameter<beam::Amount>(TxParameterID::Fee, *m_isBeamSide ? SubTxIndex::BEAM_LOCK_TX : SubTxIndex::BEAM_REDEEM_TX);
        if (fee)
        {
            return beamui::AmountToUIString(*fee, beamui::Currencies::Beam);
        }
    }
    return QString();
}

QString SwapTxObject::getFeeRate() const
{
    if (m_isBeamSide)   // check if initialized
    {
        auto feeRate = m_tx.GetParameter<beam::Amount>(TxParameterID::Fee, *m_isBeamSide ? SubTxIndex::REDEEM_TX : SubTxIndex::LOCK_TX);

        if (feeRate && m_swapCoin)
        {
            QString value = QString::number(*feeRate);

            QString rateMeasure;
            switch (*m_swapCoin)
            {
            case AtomicSwapCoin::Bitcoin:
                rateMeasure = QMLGlobals::btcFeeRateLabel();
                break;

            case AtomicSwapCoin::Litecoin:
                rateMeasure = QMLGlobals::ltcFeeRateLabel();
                break;

            case AtomicSwapCoin::Qtum:
                rateMeasure = QMLGlobals::qtumFeeRateLabel();
                break;

            default:
                break;
            }
            return value + " " + rateMeasure;
        }
    }
    return QString();
}

QString SwapTxObject::getFailureReason() const
{
    if (getTxDescription().m_status == wallet::TxStatus::Failed && getTxDescription().m_txType == beam::wallet::TxType::AtomicSwap)
    {
        auto failureReason = getTxDescription().GetParameter<TxFailureReason>(TxParameterID::InternalFailureReason);
        if (!failureReason)
        {
            auto txState = getTxDescription().GetParameter<wallet::AtomicSwapTransaction::State>(TxParameterID::State);
            if (txState && *txState == wallet::AtomicSwapTransaction::State::Refunded)
            {
                //% "Refunded"
                return qtTrId("swap-tx-failture-refunded");
            }
            else
            {
                auto extFailureReason = getTxDescription().GetParameter<TxFailureReason>(TxParameterID::FailureReason);
                if (extFailureReason)
                {
                    return getReasonString(*extFailureReason);
                }
            }
        }
        else
        {
            return getReasonString(*failureReason);
        }
    }
    return QString();
}

QString SwapTxObject::getSwapState() const
{
    if (getTxDescription().m_txType == beam::wallet::TxType::AtomicSwap)
    {
        switch (getTxDescription().m_status)
        {
        case beam::wallet::TxStatus::Pending:
        case beam::wallet::TxStatus::InProgress:
            {
                auto state = getTxDescription().GetParameter<wallet::AtomicSwapTransaction::State>(TxParameterID::State);
                if (state)
                {
                    switch (*state)
                    {
                    case wallet::AtomicSwapTransaction::State::Initial:
                        return getWaitingPeerStr(getTxDescription());
                    case wallet::AtomicSwapTransaction::State::BuildingBeamLockTX:
                    case wallet::AtomicSwapTransaction::State::BuildingBeamRefundTX:
                    case wallet::AtomicSwapTransaction::State::BuildingBeamRedeemTX:
                    case wallet::AtomicSwapTransaction::State::HandlingContractTX:
                    case wallet::AtomicSwapTransaction::State::SendingRedeemTX:
                    case wallet::AtomicSwapTransaction::State::SendingBeamLockTX:
                    case wallet::AtomicSwapTransaction::State::SendingBeamRedeemTX:
                        return getInProgressNormalStr(getTxDescription());
                    case wallet::AtomicSwapTransaction::State::SendingRefundTX:
                    case wallet::AtomicSwapTransaction::State::SendingBeamRefundTX:
                        return getInProgressRefundingStr(getTxDescription());
                    default:
                        break;
                    }
                }
                else
                {
                    return getWaitingPeerStr(getTxDescription());
                }
            }
            break;
        default:
            break;
        }
    }
    return "";
}

beam::wallet::AtomicSwapCoin SwapTxObject::getSwapCoinType() const
{
    return *m_swapCoin;
}

namespace
{
    template<typename T>
    void copyParameter(TxParameterID id, const TxParameters& source, TxParameters& dest)
    {
        if (auto p = source.GetParameter<T>(id); p)
        {
            dest.SetParameter(id, *p);
        }
    }

    void copyParameter(TxParameterID id, const TxParameters& source, TxParameters& dest, bool inverse = false)
    {
        if (auto p = source.GetParameter<bool>(id); p)
        {
            dest.SetParameter(id, inverse ? !*p : *p);
        }
    }

    template<size_t V>
    QString getSwapCoinTxId(const TxParameters& source)
    {
        if (auto res = source.GetParameter<std::string>(TxParameterID::AtomicSwapExternalTxID, V))
        {
            return QString(res->c_str());
        }
        else return QString();
    }
    
    template<size_t V>
    QString getSwapCoinTxConfirmations(const TxParameters& source)
    {
        if (auto res = source.GetParameter<uint32_t>(TxParameterID::Confirmations, V))
        {
            auto n = std::to_string(*res);
            return QString::fromStdString(n);
        }
        else return QString();
    }

    template<size_t V>
    QString getBeamTxKernelId(const TxParameters& source)
    {
        if (auto res = source.GetParameter<Merkle::Hash>(TxParameterID::KernelID, V))
        {
            return QString::fromStdString(to_hex(res->m_pData, res->nBytes));
        }
        else return QString();
    }
}

QString SwapTxObject::getToken() const
{
    if (m_type != TxType::AtomicSwap)
    {
        return "";
    }

    TxParameters tokenParams(m_tx.m_txId);

    auto isInitiator = m_tx.GetParameter<bool>(TxParameterID::IsInitiator);
    if (*isInitiator == false) 
    {
        if (auto p = m_tx.GetParameter<WalletID>(TxParameterID::MyID); p)
        {
            tokenParams.SetParameter(TxParameterID::PeerID, *p);
        }
    }
    else
    {
        copyParameter<WalletID>(TxParameterID::PeerID, m_tx, tokenParams);
    }

    tokenParams.SetParameter(TxParameterID::IsInitiator, true);

    copyParameter(TxParameterID::IsSender, m_tx, tokenParams, !*isInitiator);
    copyParameter(TxParameterID::AtomicSwapIsBeamSide, m_tx, tokenParams, !*isInitiator);

    tokenParams.SetParameter(beam::wallet::TxParameterID::TransactionType, m_type);
    copyParameter<Height>(TxParameterID::MinHeight, m_tx, tokenParams);
    copyParameter<Height>(TxParameterID::PeerResponseTime, m_tx, tokenParams);
    copyParameter<Timestamp>(TxParameterID::CreateTime, m_tx, tokenParams);
    copyParameter<Height>(TxParameterID::Lifetime, m_tx, tokenParams);

    copyParameter<Amount>(TxParameterID::Amount, m_tx, tokenParams);
    copyParameter<Amount>(TxParameterID::AtomicSwapAmount, m_tx, tokenParams);
    copyParameter<AtomicSwapCoin>(TxParameterID::AtomicSwapCoin, m_tx, tokenParams);

    return QString::fromStdString(std::to_string(tokenParams));
}

bool SwapTxObject::isLockTxProofReceived() const
{
    Height proofHeight;
    if (m_tx.GetParameter(TxParameterID::KernelProofHeight, proofHeight, SubTxIndex::BEAM_LOCK_TX))
    {
        return true;
    }
    else return false;
}

bool SwapTxObject::isRefundTxProofReceived() const
{
    Height proofHeight;
    if (m_tx.GetParameter(TxParameterID::KernelProofHeight, proofHeight, SubTxIndex::BEAM_REFUND_TX))
    {
        return true;
    }
    else return false;
}

QString SwapTxObject::getSwapCoinLockTxId() const
{
    return getSwapCoinTxId<SubTxIndex::LOCK_TX>(m_tx);
}

QString SwapTxObject::getSwapCoinRedeemTxId() const
{
    return getSwapCoinTxId<SubTxIndex::REDEEM_TX>(m_tx);
}

QString SwapTxObject::getSwapCoinRefundTxId() const
{
    return getSwapCoinTxId<SubTxIndex::REFUND_TX>(m_tx);
}

QString SwapTxObject::getSwapCoinLockTxConfirmations() const
{
    return getSwapCoinTxConfirmations<SubTxIndex::LOCK_TX>(m_tx);
}

QString SwapTxObject::getSwapCoinRedeemTxConfirmations() const
{
    return getSwapCoinTxConfirmations<SubTxIndex::REDEEM_TX>(m_tx);
}

QString SwapTxObject::getSwapCoinRefundTxConfirmations() const
{
    return getSwapCoinTxConfirmations<SubTxIndex::REFUND_TX>(m_tx);
}

QString SwapTxObject::getBeamLockTxKernelId() const
{
    return getBeamTxKernelId<SubTxIndex::BEAM_LOCK_TX>(m_tx);
}

QString SwapTxObject::getBeamRedeemTxKernelId() const
{
    return getBeamTxKernelId<SubTxIndex::REDEEM_TX>(m_tx);
}

QString SwapTxObject::getBeamRefundTxKernelId() const
{
    return getBeamTxKernelId<SubTxIndex::REFUND_TX>(m_tx);
}
