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

#ifndef CHARTWIDGET_HPP
#define CHARTWIDGET_HPP

#include "boost/function.hpp"

#include <qcustomplot.h>

#include <KLLibs.hpp>

#include <QtConcurrent>

#include <QListWidget>
#include <QWidget>

namespace Ui
{
	class ChartWidget;
}

class ChartWidget : public QWidget
{

		Q_OBJECT

	private: struct RESULT
	{
		QString Function;

		QVector<double> Values;
		QVector<double> Integrals;
		QVector<double> Arguments;
	};

	private: struct CHART
	{
		QCPGraph* Values = nullptr;
		QCPGraph* Integrals = nullptr;
		QCPBars* Spectrum = nullptr;
	};

	private:

		static const boost::function<RESULT (KLScript*, QString, double, double, unsigned)> Request;

		const KLMap<KLString, KLString>* Functions = nullptr;
		const KLVariables* Variables = nullptr;

		QMap<QString, CHART> Plots;
		QList<QString> Charts;

		unsigned Samples;
		double Start;
		double Stop;

		Ui::ChartWidget* ui;

	private:

		virtual void dragEnterEvent(QDragEnterEvent* Event) override;
		virtual void dragMoveEvent(QDragMoveEvent* Event) override;
		virtual void dropEvent(QDropEvent* Event) override;

	public:

		explicit ChartWidget(QWidget* Parent = nullptr);
		virtual ~ChartWidget(void) override;

		void SetScriptParams(const KLVariables* V, const KLMap<KLString, KLString>* F);
		void SetPlotParams(double From, double To, unsigned Steps);

	public slots:

		void AddChart(const QString& Function);
		void RemoveChart(const QString& Function);

		void RangeChanged(double From, double To);
		void SamplesChanged(int Steps);

		void LegendCheckChanged(bool Status);

		void ReplotCharts(void);

	private slots:

		void RangeDraged(const QCPRange& New, const QCPRange& Old);
		void PlotResults(QFutureWatcher<RESULT>* Watcher, int Index);

		void PlotTypeChanged(int Type);

		void SaveButtonClicked(void);
		void FitButtonClicked(void);

	signals:

		void onAddChart(const QString&, const QString&);

};

#endif // CHARTWIDGET_HPP