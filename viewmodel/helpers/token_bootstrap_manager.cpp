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

#include "token_bootstrap_manager.h"

#include "model/app_model.h"

TokenBootstrapManager::TokenBootstrapManager()
    : _wallet_model(*AppModel::getInstance().getWallet())
{
    connect(
        &_wallet_model,
        SIGNAL(txStatus(beam::wallet::ChangeAction,
                        const std::vector<beam::wallet::TxDescription>&)),
        SLOT(onTxStatus(beam::wallet::ChangeAction,
                        const std::vector<beam::wallet::TxDescription>&)));
    _wallet_model.getAsync()->getWalletStatus();
}

TokenBootstrapManager::~TokenBootstrapManager() {}

void TokenBootstrapManager::onTxStatus(
    beam::wallet::ChangeAction action,
    const std::vector<beam::wallet::TxDescription>& items)
{
    if (action != beam::wallet::ChangeAction::Reset)
    {
        _wallet_model.getAsync()->getWalletStatus();
        return;   
    }

    _myTxIds.clear();
    _myTxIds.reserve(items.size());
    for (const auto& item : items)
    {
        const auto& txId = item.GetTxID();
        if (txId)
        {
            _myTxIds.emplace_back(txId.value());
        }
    }

    checkIsTxPreviousAccepted();
}

void TokenBootstrapManager::checkTokenForDuplicate(const QString& token)
{
    auto parameters = beam::wallet::ParseParameters(token.toStdString());
    if (!parameters)
    {
        LOG_ERROR() << "Can't parse token params";
        return;
    }

    auto txId = parameters.value().GetTxID();
    if (!txId)
    {
        LOG_ERROR() << "Empty tx id in txParams";
        return;
    }
    auto txIdValue = txId.value();
    _tokensInProgress[txIdValue] = token;

    _myTxIds.empty()
        ? _wallet_model.getAsync()->getWalletStatus()
        : checkIsTxPreviousAccepted();
}

void TokenBootstrapManager::checkIsTxPreviousAccepted()
{
    if (!_tokensInProgress.empty())
    {
        for (const auto& txId : _myTxIds)
        {
            const auto& it = _tokensInProgress.find(txId);
            if (it != _tokensInProgress.end())
            {
                emit tokenPreviousAccepted(it->second);
                _tokensInProgress.erase(it);
            }
        }

        for (const auto& token : _tokensInProgress)
        {
            emit tokenFirstTimeAccepted(token.second);
        }
        _tokensInProgress.clear();
    }
}
