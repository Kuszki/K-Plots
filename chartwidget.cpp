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

#include "chartwidget.hpp"
#include "ui_chartwidget.h"

const boost::function<ChartWidget::RESULT (KLScript*, QString, double, double, unsigned)> ChartWidget::Request =
[] (KLScript* Script, QString Function, double Start, double Stop, unsigned Samples) -> ChartWidget::RESULT
{
	const KLString Run = QString("goto %1 t;").arg(Function).toStdString().c_str();

	auto& t = Script->Variables["t"];
	double dt = double(Stop - Start) / Samples;

	ChartWidget::RESULT Result; t = Start;

	Result.Function = Function;
	Script->Variables["dt"] = dt;

	while (t.ToNumber() < Stop)
	{
		if (!Script->Evaluate(Run)) return Result;
		else
		{
			Result.Arguments.append(t.ToNumber());
			Result.Values.append(Script->GetReturn());
		}

		t = t.ToNumber() + dt;
	}

	Result.Integrals.append(0);

	for (const auto& V : Result.Values)
	{
		Result.Integrals.append(Result.Integrals.last() + V * dt);
	}

	Result.Integrals.removeLast();

	delete Script;
	return Result;
};

ChartWidget::ChartWidget(QWidget* Parent)
: QWidget(Parent), ui(new Ui::ChartWidget)
{
	ui->setupUi(this);

	ui->Plot->setInteraction(QCP::iRangeDrag, true);
	ui->Plot->setInteraction(QCP::iRangeZoom, true);

	setAcceptDrops(true);

	connect(ui->Plot->xAxis, SIGNAL(rangeChanged(QCPRange,QCPRange)), SLOT(RangeDraged(QCPRange,QCPRange)));
}

ChartWidget::~ChartWidget(void)
{
	delete ui;
}

void ChartWidget::SetScriptParams(const KLVariables* V, const KLMap<KLString, KLString>* F)
{
	Variables = V;
	Functions = F;
}

void ChartWidget::SetPlotParams(double From, double To, unsigned Steps)
{
	Start = qMin(From, To);
	Stop = qMax(From, To);
	Samples = Steps;

	ui->Plot->xAxis->setRange(Start, Stop);
}

void ChartWidget::AddChart(const QString& Function)
{
	if (!Charts.contains(Function))
	{
		Charts.append(Function);

		QFutureWatcher<RESULT>* Watcher = new QFutureWatcher<RESULT>(this);
		KLScript* Script = new KLScript();

		Script->Variables = *Variables;
		Script->Functions = *Functions;

		connect(Watcher, &QFutureWatcher<RESULT>::resultReadyAt, boost::bind(&ChartWidget::PlotResults, this, Watcher, _1));
		connect(Watcher, &QFutureWatcher<RESULT>::finished, Watcher, &QFutureWatcher<RESULT>::deleteLater);

		Watcher->setFuture(QtConcurrent::run(Request, Script, Function, Start, Stop, Samples));

		emit onAddChart(parent()->objectName(), Function);
	}
}

void ChartWidget::RemoveChart(const QString& Function)
{
	if (Charts.contains(Function)) Charts.removeAll(Function);

	if (Plots.contains(Function))
	{
		auto Data = Plots.take(Function);

		ui->Plot->removePlottable(Data.Integrals);
		ui->Plot->removePlottable(Data.Spectrum);
		ui->Plot->removePlottable(Data.Values);
	}

	ui->Plot->replot();
}

void ChartWidget::RangeChanged(double From, double To)
{
	QCPRange Range = ui->Plot->xAxis->range();

	Start = From;
	Stop = To;

	ui->Plot->xAxis->blockSignals(true);

	if (Start + Range.size() > Stop) ui->Plot->xAxis->setRange(Start, Stop);
	else ui->Plot->xAxis->setRange(Start, Range.size(), Qt::AlignLeft);

	ui->Plot->xAxis->blockSignals(false);
	ui->Plot->replot();

	ReplotCharts();
}

void ChartWidget::SamplesChanged(int Steps)
{
	Samples = Steps;

	ReplotCharts();
}

void ChartWidget::LegendCheckChanged(bool Status)
{
	ui->Plot->legend->setVisible(Status);
	ui->Plot->replot();
}

void ChartWidget::ReplotCharts(void)
{
	QList<QPair<QString, KLScript*>> Tasks;

	for (const auto& Chart : Charts)
	{
		KLScript* Script = new KLScript();

		Script->Variables = *Variables;
		Script->Functions = *Functions;

		Tasks.append({Chart, Script});
	}

	boost::function<RESULT (QPair<QString, KLScript*>)> Function = [this] (const QPair<QString, KLScript*>& Job) -> RESULT
	{
		return Request(Job.second, Job.first, Start, Stop, Samples);
	};

	QFutureWatcher<RESULT>* Watcher = new QFutureWatcher<RESULT>(this);

	connect(Watcher, &QFutureWatcher<RESULT>::resultReadyAt, boost::bind(&ChartWidget::PlotResults, this, Watcher, _1));
	connect(Watcher, &QFutureWatcher<RESULT>::finished, Watcher, &QFutureWatcher<RESULT>::deleteLater);

	Watcher->setFuture(QtConcurrent::mapped(Tasks, Function));
}

void ChartWidget::RangeDraged(const QCPRange& New, const QCPRange& Old)
{
	const double Lower = New.lower < Start ? Start : New.lower;
	const double Upper = New.upper > Stop ? Stop : New.upper;

	ui->Plot->xAxis->blockSignals(true);

	if (Lower == Start || Upper == Stop) ui->Plot->xAxis->setRange(Old);
	else ui->Plot->xAxis->setRange(Lower, Upper);

	ui->Plot->xAxis->blockSignals(false);
}

void ChartWidget::PlotResults(QFutureWatcher<RESULT>* Watcher, int Index)
{
	RESULT Result = Watcher->resultAt(Index);
	CHART Data;

	if (Plots.contains(Result.Function))
	{
		auto Data = Plots.take(Result.Function);

		ui->Plot->removePlottable(Data.Integrals);
		ui->Plot->removePlottable(Data.Spectrum);
		ui->Plot->removePlottable(Data.Values);
	}

	Data.Values = ui->Plot->addGraph(ui->Plot->xAxis, ui->Plot->yAxis);

	Data.Values->setData(Result.Arguments, Result.Values);
	Data.Values->setName(Result.Function);

	Data.Integrals = ui->Plot->addGraph(ui->Plot->xAxis, ui->Plot->yAxis);

	Data.Integrals->setData(Result.Arguments, Result.Integrals);
	Data.Integrals->setName(Result.Function);

	Data.Values->setVisible(ui->typeCombo->currentIndex() == 0);
	Data.Integrals->setVisible(ui->typeCombo->currentIndex() == 1);

	if (!Data.Values->visible()) Data.Values->removeFromLegend();
	if (!Data.Integrals->visible()) Data.Integrals->removeFromLegend();

	Plots.insert(Result.Function, Data);

	ui->Plot->replot();
}

void ChartWidget::PlotTypeChanged(int Type)
{
	for (auto Plot : Plots)
	{
		Plot.Values->setVisible(Type == 0);
		Plot.Integrals->setVisible(Type == 1);

		if (!Plot.Values->visible()) Plot.Values->removeFromLegend();
		else Plot.Values->addToLegend();

		if (!Plot.Integrals->visible()) Plot.Integrals->removeFromLegend();
		else Plot.Integrals->addToLegend();
	}

	ui->Plot->replot();
}

void ChartWidget::SaveButtonClicked(void)
{

}

void ChartWidget::FitButtonClicked(void)
{
	ui->Plot->yAxis->rescale(true);
	ui->Plot->replot();
}

void ChartWidget::dragEnterEvent(QDragEnterEvent* Event)
{
	Event->acceptProposedAction();
}

void ChartWidget::dragMoveEvent(QDragMoveEvent* Event)
{
	Event->acceptProposedAction();
}

void ChartWidget::dropEvent(QDropEvent* Event)
{
	if (QListWidget* Source = qobject_cast<QListWidget*>(Event->source()))
	{
		AddChart(Source->currentItem()->text());
	}
}
