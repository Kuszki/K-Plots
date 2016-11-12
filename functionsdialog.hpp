/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Plot simple charts using QCustomPlot and KLScript                      *
 *  Copyright (C) 2015  Łukasz "Kuszki" Dróżdż  l.drozdz@openmailbox.org   *
 *                                                                         *
 *  This program is free software: you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the  Free Software Foundation, either  version 3 of the  License, or   *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This  program  is  distributed  in the hope  that it will be useful,   *
 *  but WITHOUT ANY  WARRANTY;  without  even  the  implied  warranty of   *
 *  MERCHANTABILITY  or  FITNESS  FOR  A  PARTICULAR  PURPOSE.  See  the   *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have  received a copy  of the  GNU General Public License   *
 *  along with this program. If not, see http://www.gnu.org/licenses/.     *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef FUNCTIONSDIALOG_HPP
#define FUNCTIONSDIALOG_HPP

#include <boost/function.hpp>

#include <QPushButton>
#include <QMessageBox>
#include <QDialog>

namespace Ui
{
	class FunctionsDialog;
}

class FunctionsDialog : public QDialog
{

		Q_OBJECT

	private:

		using VALIDATOR = boost::function<QString (const QString&, const QString&, bool)>;

		Ui::FunctionsDialog* ui;

		VALIDATOR Validator;

	public:

		explicit FunctionsDialog(const VALIDATOR& Bind = [] (auto, auto, auto) { return QString(); },
							QWidget* Parent = nullptr);
		virtual ~FunctionsDialog(void) override;

		void SetValidator(const VALIDATOR& Bind);
		void ResetValidator(void);

	private slots:

		void DialogDataChanged(void);

	public slots:

		virtual void open(const QString& Name, const QString& Code);
		virtual void open(void) override;

		virtual void accept(void) override;

	signals:

		void onEditFinished(const QString&, const QString&);
};

#endif // FUNCTIONSDIALOG_HPP
