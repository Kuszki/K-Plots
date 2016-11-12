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

#ifndef FUNCTIONSWIDGET_HPP
#define FUNCTIONSWIDGET_HPP

#include <boost/function.hpp>

#include <QInputDialog>
#include <QListWidget>
#include <QWidget>

#include "functionsdialog.hpp"
#include "titlewidget.hpp"

namespace Ui
{
	class FunctionsWidget;
}

class FunctionsWidget : public QWidget
{

		Q_OBJECT

	private:

		using VALIDATOR = boost::function<QString (const QString&, const QString&, bool)>;

		Ui::FunctionsWidget* ui;

		VALIDATOR Validator;

	public:

		explicit FunctionsWidget(const VALIDATOR& Bind = [] (auto, auto, auto) { return QString(); },
							QWidget* Parent = nullptr);
		virtual ~FunctionsWidget(void) override;

		void SetTitleWidget(TitleWidget* Widget);

		void SetValidator(const VALIDATOR& Bind);
		void ResetValidator(void);

	private slots:

		void ListItemChanged(QListWidgetItem* Item);

		void SearchEditChanged(const QString& Text);

		void AddButtonClicked(void);
		void RemoveButtonClicked(void);

	public slots:

		void AddFunction(const QString& Name, const QString& Code);

	signals:

		void onAddFunction(const QString&, const QString&);
		void onEditFunction(const QString&, const QString&);
		void onRemoveFunction(const QString&);

};

#endif // FUNCTIONSWIDGET_HPP
