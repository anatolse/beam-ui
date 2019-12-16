import QtQuick.Layouts 1.11
import QtQuick 2.11
import Beam.Wallet 1.0
import "../utils.js" as Utils
import Beam.Wallet 1.0

ColumnLayout {
    id: control

    readonly property variant currencies: [
        {label: "BEAM", feeLabel: BeamGlobals.beamFeeRateLabel(), minFee: BeamGlobals.minFeeBeam(), defaultFee: BeamGlobals.defFeeBeam()},
        {label: "BTC",  feeLabel: BeamGlobals.btcFeeRateLabel(),  minFee: 0,                        defaultFee: BeamGlobals.defFeeRateBtc()},
        {label: "LTC",  feeLabel: BeamGlobals.ltcFeeRateLabel(),  minFee: 0,                        defaultFee: BeamGlobals.defFeeRateLtc()},
        {label: "QTUM", feeLabel: BeamGlobals.qtumFeeRateLabel(), minFee: 0,                        defaultFee: BeamGlobals.defFeeRateQtum()}
    ]

    function getCurrencyLabel() {
        return currencies[control.currency].label
    }

    function getFeeLabel() {
        return currencies[control.currency].feeLabel
    }

    function getFeeTitle() {
        if (control.currency == Currency.CurrBeam) {
            return control.currFeeTitle ?
                //% "BEAM Transaction fee"
                qsTrId("beam-transaction-fee") :
                //% "Transaction fee"
                qsTrId("general-fee")
        }
        //% "%1 Transaction fee rate"
        return qsTrId("general-fee-rate").arg(getCurrencyLabel())
    }

    function getTotalFeeTitle() {
        //% "%1 Transaction fee (est)"
        return qsTrId("general-fee-total").arg(getCurrencyLabel())
    }

    function getTotalFeeAmount() {
        return BeamGlobals.calcTotalFee(control.currency, control.fee);
    }

    readonly property bool     isValidFee:     hasFee ? feeInput.isValid : true
    readonly property bool     isValid:        error.length == 0 && isValidFee
    readonly property string   currencyLabel:  getCurrencyLabel()

    property string   title
    property string   color:        Style.accent_incoming
    property string   currColor:    Style.content_main
    property bool     hasFee:       false
    property bool     currFeeTitle: false
    property bool     multi:        false // changing this property in runtime would reset bindings
    property int      currency:     Currency.CurrBeam
    property string   amount:       "0"
    property int      fee:          currencies[currency].defaultFee
    property alias    error:        errmsg.text
    property bool     readOnlyA:    false
    property bool     readOnlyF:    false
    property bool     resetAmount:  true
    property var      amountInput:  ainput
    property bool     showTotalFee: false

    SFText {
        font.pixelSize:   14
        font.styleName:   "Bold"
        font.weight:      Font.Bold
        color:            Style.content_main
        text:             control.title
    }

    RowLayout {
        Layout.fillWidth: true

        SFTextInput {
            id:               ainput
            Layout.fillWidth: true
            font.pixelSize:   36
            font.styleName:   "Light"
            font.weight:      Font.Light
            color:            error.length ? Style.validator_error : control.color
            backgroundColor:  error.length ? Style.validator_error : Style.content_main
            validator:        RegExpValidator {regExp: /^(([1-9][0-9]{0,7})|(1[0-9]{8})|(2[0-4][0-9]{7})|(25[0-3][0-9]{6})|(0))(\.[0-9]{0,7}[1-9])?$/}
            selectByMouse:    true
            text:             formatDisplayedAmount()
            readOnly:         control.readOnlyA

            onTextChanged: {
                if (ainput.focus) {
                    // if nothing then "0", remove insignificant zeroes and "." in floats
                    control.amount = text ? text.replace(/\.0*$|(\.\d*[1-9])0+$/,'$1') : "0"
                }
            }

            onFocusChanged: {
                text = formatDisplayedAmount()
                if (focus) cursorPosition = positionAt(ainput.getMousePos().x, ainput.getMousePos().y)
            }

            function formatDisplayedAmount() {
                return control.amount == "0" ? "" : (ainput.focus ? control.amount : Utils.uiStringToLocale(control.amount))
            }

            Connections {
                target: control
                onAmountChanged: {
                    if (!ainput.focus) {
                        ainput.text = ainput.formatDisplayedAmount()
                    }
                }
            }
        }

        SFText {
            Layout.topMargin:   22
            font.pixelSize:     24
            font.letterSpacing: 0.6
            color:              control.currColor
            text:               getCurrencyLabel()
            visible:            !multi
        }

        CustomComboBox {
            id:                  currCombo
            Layout.topMargin:    22
            Layout.minimumWidth: 95
            spacing:             0
            fontPixelSize:       24
            fontLetterSpacing:   0.6
            currentIndex:        control.currency
            color:               control.currColor
            visible:             multi
            model:               Utils.currenciesList()

            onActivated: {
                if (multi) control.currency = index
                if (resetAmount) control.amount = 0
            }
        }
    }

    Item {
        Layout.fillWidth: true
        SFText {
            id:              errmsg
            color:           Style.validator_error
            font.pixelSize:  12
            font.styleName:  "Italic"
            width:           parent.width
            visible:         error.length
        }
    }

    GridLayout {
        columns:       2
        Layout.topMargin: 30
        ColumnLayout {
            Layout.maximumWidth:  198
            SFText {
                font.pixelSize:   14
                font.styleName:   "Bold"
                font.weight:      Font.Bold
                color:            Style.content_main
                text:             getFeeTitle()
                visible:          control.hasFee
            }
            FeeInput {
                id:               feeInput
                Layout.fillWidth: true
                visible:          control.hasFee
                fee:              control.fee
                minFee:           currencies[currency].minFee
                feeLabel:         getFeeLabel()
                color:            control.color
                readOnly:         control.readOnlyF
                Connections {
                    target: control
                    onFeeChanged: feeInput.fee = control.fee
                    onCurrencyChanged: feeInput.fee = currencies[currency].defaultFee
                }
            }
        }
       
        ColumnLayout {
            Layout.alignment:     Qt.AlignLeft | Qt.AlignTop
            visible:              showTotalFee && control.hasFee && control.currency != Currency.CurrBeam
            SFText {
                font.pixelSize:   14
                font.styleName:   "Bold"
                font.weight:      Font.Bold
                color:            Style.content_main
                text:             getTotalFeeTitle()
            }
            SFText {
                Layout.topMargin: 6
                font.pixelSize:   14
                color:            Style.content_main
                text:             getTotalFeeAmount()
            }
        }
    }

    SFText {
        enabled:               showTotalFee && control.hasFee && control.currency != Currency.CurrBeam
        visible:               enabled
        Layout.preferredWidth: 370
        font.pixelSize:        14
        wrapMode:              Text.WordWrap
        color:                 Style.content_secondary
        lineHeight:            1.1 
        //% "Remember to validate the expected fee rate for the blockchain (as it varies with time)."
        text:                  qsTrId("settings-fee-rate-note")
    }

    Binding {
        target:   control
        property: "fee"
        value:    feeInput.fee
    }
}
