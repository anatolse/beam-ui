function formatDateTime(datetime, localeName) {
    var maxTime = new Date(4294967295000);
    if (datetime >= maxTime) {
        //: time never string
        //% "Never"
        return qsTrId("time-never");
    }
    var timeZoneShort = datetime.getTimezoneOffset() / 60 * (-1);
    return datetime.toLocaleDateString(localeName)
         + " | "
         + datetime.toLocaleTimeString(localeName)
         + (timeZoneShort >= 0 ? " (GMT +" : " (GMT ")
         + timeZoneShort
         + ")";
}

function number2Locale (number) {
    return number.toLocaleString(Qt.locale(), 'f', -128)
}

function number2LocaleFixed (number) {
    if (number < 0.00000001) number = 0.00000001;
    return number.toLocaleString(Qt.locale(), 'f', 8).replace(/\.?0+$/,"")
}

// @arg amount - any number or float string in "C" locale
function amount2locale (amount) {
    var locale = Qt.locale()
    var parts  = amount.toString().split(".")
    var left   = parts[0].replace(/(\d)(?=(?:\d{3})+\b)/g, "$1" + locale.groupSeparator)
    return parts[1] ? [left, parts[1]].join(locale.decimalPoint) : left
}

function getLogoTopGapSize(parentHeight) {
    return parentHeight * (parentHeight < 768 ? 0.13 : 0.18)
}

function handleMousePointer(mouse, element) {
    element.cursorShape = element.parent.linkAt(mouse.x, mouse.y).length
        ? element.cursorShape = Qt.PointingHandCursor
        : element.cursorShape = Qt.ArrowCursor;
}

function openExternal(externalLink, settings, dialog) {
    if (settings.isAllowedBeamMWLinks) {
        Qt.openUrlExternally(externalLink);
    } else {
        dialog.externalUrl = externalLink;
        dialog.onOkClicked = function () {
            settings.isAllowedBeamMWLinks = true;
        };
        dialog.open();
    }
}

function handleExternalLink(mouse, element, settings, dialog) {
    if (element.cursorShape == Qt.PointingHandCursor) {
        var externalLink = element.parent.linkAt(mouse.x, mouse.y);
        if (settings.isAllowedBeamMWLinks) {
            Qt.openUrlExternally(externalLink);
        } else {
            dialog.externalUrl = externalLink;
            dialog.onOkClicked = function () {
                settings.isAllowedBeamMWLinks = true;
            };
            dialog.open();
        }
        return true;
    }
}

function calcDisplayRate(aiReceive, aiSend, numOnly) {
    // ai[X] = amount input control
    var cr = aiReceive.currency
    var cs = aiSend.currency
    if (cr == cs) return {rate: 1, displayRate: "1", error: false}

    var ams = aiSend.amount
    var amr = aiReceive.amount
    if (ams == 0 || amr == 0) return {rate: 0, displayRate: "", error: false}

    var minRate     = 0.00000001
    var rate        = amr / ams
    var format      = function (value) {return value.toLocaleString(numOnly ? Qt.locale("C") : Qt.locale(), 'f', value < minRate ? 17 : 8).replace(/\.?0+$/,"")}
    var displayRate = format(rate)

    return {
        rate: rate,
        displayRate: displayRate,
        error: rate < minRate,
        errorText: rate < minRate
            //% "Rate cannot be less than %1"
            ? qsTrId("invalid-rate-min").arg(minRate)
            : undefined,
        minRate: minRate,
        minDisplayRate: format(minRate)
    }
}

function getAmountWithoutCurrency(amountWithCurrency) {
    return amountWithCurrency.split(" ")[0];
}

function currenciesList() {
    return ["BEAM", "BTC", "LTC", "QTUM"]
}

const symbolBeam  = '\uEAFB'
const symbolBtc   = '\u20BF'
const symbolLtc   = '\u0141'
const symbolQtum  = '\uEAFD'