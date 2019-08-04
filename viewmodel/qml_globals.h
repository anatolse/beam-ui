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
#pragma once

#include <QObject>
#include <QQmlApplicationEngine>
#include "currencies.h"

class QMLGlobals : public QObject
{
    Q_OBJECT
public:
    QMLGlobals(QQmlEngine&);

    Q_INVOKABLE static void showMessage(const QString& message);
    Q_INVOKABLE static void copyToClipboard(const QString& text);
    Q_INVOKABLE static bool isTransactionToken(const QString& text);
    Q_INVOKABLE static bool isSwapToken(const QString& text);
    Q_INVOKABLE static QString getLocaleName();
    Q_INVOKABLE static int  maxCommentLength();
    Q_INVOKABLE static bool needPasswordToSpend();
    Q_INVOKABLE static bool isPasswordValid(const QString& value);

    // Currency utils
    static bool isFeeOK(int fee, Currency currency);
    static int  getMinFeeOrRate(Currency currency);

    Q_INVOKABLE static int minFeeBeam();
    Q_INVOKABLE static int minFeeRateBtc();
    Q_INVOKABLE static int minFeeRateLtc();
    Q_INVOKABLE static int minFeeRateQtum();

    Q_INVOKABLE static int defFeeBeam();
    Q_INVOKABLE static int defFeeRateBtc();
    Q_INVOKABLE static int defFeeRateLtc();
    Q_INVOKABLE static int defFeeRateQtum();

    Q_INVOKABLE static QString beamFeeRateLabel();
    Q_INVOKABLE static QString btcFeeRateLabel();
    Q_INVOKABLE static QString ltcFeeRateLabel();
    Q_INVOKABLE static QString qtumFeeRateLabel();

private:
    QQmlEngine& _engine;
};
