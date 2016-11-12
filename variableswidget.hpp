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

#ifndef VARIABLESWIDGET_HPP
#define VARIABLESWIDGET_HPP

#include <boost/function.hpp>

#include <QInputDialog>
#include <QTableWidget>
#include <QWidget>

#include "variablesdialog.hpp"
#include "titlewidget.hpp"

namespace Ui
{
	class VariablesWidget;
}

class VariablesWidget : public QWidget
{

		Q_OBJECT

	private:

		using VALIDATOR = boost::function<bool (const QString&)>;

		Ui::VariablesWidget* ui;

		VALIDATOR Validator;

	public:

		explicit VariablesWidget(const VALIDATOR& Bind = [] (auto) { return true; },
							QWidget* Parent = nullptr);
		virtual ~VariablesWidget(void) override;

		void SetTitleWidget(TitleWidget* Widget);

		void SetValidator(const VALIDATOR& Bind);
		void ResetValidator(void);

	private slots:

		void ListItemChanged(QTableWidgetItem* Item);

		void SearchEditChanged(const QString& Text);

		void AddButtonClicked(void);
		void RemoveButtonClicked(void);

	public slots:

		void AddVariable(const QString& Name, double Value);

	signals:

		void onAddVariable(const QString&, double);
		void onEditVariable(const QString&, double);
		void onRenameVariable(const QString&, const QString&);
		void onRemoveVariable(const QString&);

};

#endif // VARIABLESWIDGET_HPP
