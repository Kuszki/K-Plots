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

#ifndef PLOTSLISTWIDGET_HPP
#define PLOTSLISTWIDGET_HPP

#include <boost/function.hpp>

#include <QListWidget>
#include <QTreeWidget>
#include <QDropEvent>
#include <QWidget>

#include "titlewidget.hpp"

namespace Ui
{
	class PlotslistWidget;
}

class PlotslistWidget : public QWidget
{

		Q_OBJECT

	private:

		using VALIDATOR = boost::function<bool (const QString&)>;

		Ui::PlotslistWidget* ui;

		VALIDATOR Validator;

	private:

		virtual void dragEnterEvent(QDragEnterEvent* Event) override;
		virtual void dragMoveEvent(QDragMoveEvent* Event) override;
		virtual void dropEvent(QDropEvent* Event) override;

	public:

		explicit PlotslistWidget(const VALIDATOR& Bind = [] (auto) { return true; },
							QWidget* Parent = nullptr);
		virtual ~PlotslistWidget(void) override;

		void SetTitleWidget(TitleWidget* Widget);

		void SetValidator(const VALIDATOR& Bind);
		void ResetValidator(void);

	private slots:

		void TreeItemChanged(QTreeWidgetItem* Item);

		void AddButtonClicked(void);
		void RemoveButtonClicked(void);

	public slots:

		void AddPlot(const QString& Name);
		void AddChart(const QString& Plot, const QString& Name);

	signals:

		void onAddPlot(const QString&);
		void onRenamePlot(const QString&, const QString&);
		void onRemovePlot(const QString&);

		void onAddChart(const QString&, const QString&);
		void onRemoveChart(const QString&, const QString&);

};

#endif // PLOTSLISTWIDGET_HPP
