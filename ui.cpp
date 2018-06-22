#include <QApplication>
#include <QtQuick>
#include <QMessageBox>
#include <QInputDialog>

#include <qqmlcontext.h>

#include "viewmodel/main.h"
#include "viewmodel/dashboard.h"
#include "viewmodel/wallet.h"
#include "viewmodel/notifications.h"
#include "viewmodel/help.h"
#include "viewmodel/settings.h"

#include "wallet/wallet_db.h"

int main (int argc, char* argv[])
{
	QApplication app(argc, argv);

	static const char* WALLET_STORAGE = "wallet.db";
	QString pass;

	if (!beam::Keychain::isInitialized(WALLET_STORAGE))
	{
		if (QMessageBox::warning(0, "Warning", "Your wallet isn't created. Do you want to create it?", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
		{
			bool ok = false;
			pass = QInputDialog::getText(0, "Password", "Please, enter a password for your wallet:", QLineEdit::Password, nullptr, &ok);

			if (ok)
			{
				if (!pass.isEmpty())
				{
					ECC::NoLeak<ECC::uintBig> walletSeed;
					walletSeed.V = ECC::Zero;
					auto keychain = beam::Keychain::init(WALLET_STORAGE, pass.toStdString(), walletSeed);

					if (!keychain)
					{
						QMessageBox::critical(0, "Error", "Your wallet isn't created. Something went wrong.", QMessageBox::Ok);
						return 0;
					}
				}
				else
				{
					QMessageBox::critical(0, "Error", "Your wallet isn't created. Please, provide password for the wallet.", QMessageBox::Ok);
					return 0;
				}
			}
		}
		else return 0;
	}

	if (pass.isEmpty())
	{
		bool ok = false;
		pass = QInputDialog::getText(0, "Password", "Please, enter a password for your wallet:", QLineEdit::Password, nullptr, &ok);
	}

	{
		auto keychain = beam::Keychain::open(WALLET_STORAGE, pass.toStdString());

		if (keychain)
		{
			struct
			{
				MainViewModel main;
				DashboardViewModel dashboard;
				WalletViewModel wallet;
				NotificationsViewModel notifications;
				HelpViewModel help;
				SettingsViewModel settings;
			} viewModel;

			QQuickView view;
			view.setResizeMode(QQuickView::SizeRootObjectToView);

			QQmlContext *ctxt = view.rootContext();

			ctxt->setContextProperty("mainViewModel", &viewModel.main);

			ctxt->setContextProperty("walletViewModel", &viewModel.wallet);
			ctxt->setContextProperty("listModel", QVariant::fromValue(viewModel.wallet.tx()));

			view.setSource(QUrl("qrc:///main.qml"));
			view.show();

			return app.exec();
		}
		else
		{
			QMessageBox::critical(0, "Error", "Wallet data unreadable, restore wallet.db from latest backup or delete it and reinitialize the wallet.", QMessageBox::Ok);
			return 0;
		}
	}

	return 0;
}
