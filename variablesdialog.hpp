/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Plot simple charts using QCustomPlot and KLScript                      *
 *  Copyright (C) 2016  Łukasz "Kuszki" Dróżdż  l.drozdz@openmailbox.org   *
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

#ifndef VARIABLESDIALOG_HPP
#define VARIABLESDIALOG_HPP

#include <boost/function.hpp>

#include <QPushButton>
#include <QLocale>
#include <QDialog>

namespace Ui
{
	class VariablesDialog;
}

class VariablesDialog : public QDialog
{

		Q_OBJECT

	private:

		using VALIDATOR = boost::function<bool (const QString&)>;

		Ui::VariablesDialog* ui;

		VALIDATOR Validator;

	public:

		explicit VariablesDialog(const VALIDATOR& Bind = [] (auto) { return true; },
							QWidget* Parent = nullptr);
		virtual ~VariablesDialog(void) override;

		void SetValidator(const VALIDATOR& Bind);
		void ResetValidator(void);

	private slots:

		void DialogDataChanged(void);

	public slots:

		virtual void accept(void) override;

		virtual void open(const QString& Name, double Value);
		virtual void open(void) override;

	signals:

		void onEditFinished(const QString&, double);

};

#endif // VARIABLESDIALOG_HPP
