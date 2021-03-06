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

	QFutureWatcher<bool> valuesWatcher;
	QThread watchdogThread;
	QTimer Watchdog;

	valuesWatcher.moveToThread(&watchdogThread);
	Watchdog.moveToThread(&watchdogThread);
	watchdogThread.start();

	ChartWidget::RESULT Result;
	Result.Function = Function;
	Result.Arguments.reserve(Samples);
	Result.Values.reserve(Samples);

	auto& t = Script->Variables["t"];
	double dt = double(Stop - Start) / (Samples - 1);

	QObject::connect(&valuesWatcher, &QFutureWatcher<bool>::started, boost::bind(&QTimer::start, &Watchdog, 500));
	QObject::connect(&valuesWatcher, &QFutureWatcher<bool>::finished, &Watchdog, &QTimer::stop);
	QObject::connect(&Watchdog, &QTimer::timeout, boost::bind(&KLScript::Terminate, Script));

	valuesWatcher.setFuture(QtConcurrent::run([Script, &Result, &Run, &t] (double Start, int Samples, double dt) -> bool
	{
		Script->Variables["dt"] = dt; t = Start;

		for (int i = 0; i < Samples; ++i) if (!Script->Evaluate(Run)) return false;
		else
		{
			Result.Arguments.append(t.ToNumber());
			Result.Values.append(Script->GetReturn());
			ChartWidget::fitValue(Result.Values.last());

			t = t.ToNumber() + dt;
		}

		return true;
	}, Start, Samples, dt));

	Result.Integrals.reserve(Samples);
	Result.Derivatives.reserve(Samples);
	Result.Transform.reserve(Samples);
	Result.Realis.reserve(Samples);
	Result.Imaginaris.reserve(Samples);
	Result.Freqargs.reserve(Samples);

	if (valuesWatcher.result())
	{
		QFutureSynchronizer<void> Synchronizer;

		Synchronizer.addFuture(QtConcurrent::run([&Result] (double dt) -> void
		{
			const int Size = Result.Values.size() - 1;
			Result.Integrals.append(0);

			for (int i = 0; i < Size; ++i)
			{
				Result.Integrals.append(Result.Integrals.last() + Result.Values[i] * dt);
				ChartWidget::fitValue(Result.Values.last());
			}
		}, dt));

		Synchronizer.addFuture(QtConcurrent::run([&Result] (double dt) -> void
		{
			const int Size = Result.Values.size();
			Result.Derivatives.append(qQNaN());

			for (int i = 1; i < Size; ++i)
			{
				Result.Derivatives.append((Result.Values[i] - Result.Values[i - 1]) / dt);
				ChartWidget::fitValue(Result.Values.last());
			}
		}, dt));

		{

			const int Size = Result.Values.size();
			fftw_complex* Complex = fftw_alloc_complex(Size);
			fftw_plan Plan = fftw_plan_dft_r2c_1d(Size, &Result.Values.first(), Complex, FFTW_ESTIMATE);

			fftw_execute(Plan);

			for (int i = 0; i < Size / 2 + 1; ++i)
			{
				Result.Transform.append(2.0 * qSqrt(Complex[i][0] * Complex[i][0] + Complex[i][1] * Complex[i][1]) / Samples);
				Result.Realis.append(2.0 * Complex[i][0] / Samples);
				Result.Imaginaris.append(2.0 * Complex[i][1] / Samples);

				Result.Freqargs.append(i / (dt * Samples));
			}

			fftw_destroy_plan(Plan);
			fftw_free(Complex);

			Result.Transform.first() /= 2;
			Result.Realis.first() /= 2;
			Result.Imaginaris.first() /= 2;

		}

		Synchronizer.waitForFinished();
	}

	watchdogThread.exit();
	watchdogThread.wait();

	delete Script;
	return Result;
};

ChartWidget::ChartWidget(QWidget* Parent)
: QWidget(Parent), ui(new Ui::ChartWidget), Synchronizer(QMutex::Recursive), Colors(
{
	Qt::red, Qt::blue, Qt::green,
	Qt::darkRed, Qt::darkBlue, Qt::darkGreen,
	Qt::darkYellow, Qt::darkMagenta, Qt::darkCyan,
	Qt::yellow, Qt::magenta, Qt::cyan
})
{
	ui->setupUi(this);

	ui->spectrumButton->setMenu(new QMenu(tr("Spectrum options"), this));

	ui->spectrumButton->menu()->addAction(ui->actionAbs);
	ui->spectrumButton->menu()->addAction(ui->actionRe);
	ui->spectrumButton->menu()->addAction(ui->actionIm);

	ui->typeCombo->setMinimumWidth(ui->typeCombo->sizeHint().width() + 25);

	ui->Plot->setInteraction(QCP::iRangeDrag, true);
	ui->Plot->setInteraction(QCP::iRangeZoom, true);
	ui->Plot->axisRect()->setupFullAxesBox();

	ui->actionAbs->setChecked(true);

	setAcceptDrops(true);

	connect(ui->actionAbs, &QAction::toggled, this, &ChartWidget::SpectrumTypeChanged);
	connect(ui->actionRe, &QAction::toggled, this, &ChartWidget::SpectrumTypeChanged);
	connect(ui->actionIm, &QAction::toggled, this, &ChartWidget::SpectrumTypeChanged);

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

void ChartWidget::ResetWidget(void)
{
	for (const auto& Plot : Plots) deletePlotable(Plot);

	Charts.clear(); Plots.clear();

	ui->Plot->replot();
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

	if (Plots.contains(Function)) deletePlotable(Plots.take(Function));

	ui->Plot->replot();
}

void ChartWidget::RangeChanged(double From, double To)
{
	QCPRange Range = ui->Plot->xAxis->range();

	Start = From;
	Stop = To;

	ui->Plot->xAxis->blockSignals(true);

	if (ui->typeCombo->currentIndex() == 3)
	{
		const double Limit = (Samples / 2 + 1) / (Samples * (double(Stop - Start) / (Samples - 1)));

		if (Range.size() > Limit) ui->Plot->xAxis->setRange(0, Limit);
	}
	else
	{
		if (Start + Range.size() > Stop) ui->Plot->xAxis->setRange(Start, Stop);
		else ui->Plot->xAxis->setRange(Start, Range.size(), Qt::AlignLeft);
	}

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
	Synchronizer.lock();

	if (!Finished)
	{
		Replot = true; return;
	}
	else Finished = false;

	Synchronizer.unlock();

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
	connect(Watcher, &QFutureWatcher<RESULT>::finished, this, &ChartWidget::FinishReploting);

	Watcher->setFuture(QtConcurrent::mapped(Tasks, Function));
}

void ChartWidget::RangeDraged(const QCPRange& New, const QCPRange& Old)
{
	const double Highest = ui->typeCombo->currentIndex() == 3 ? (Samples / 2 + 1) / (Samples * (double(Stop - Start) / (Samples - 1))) : Stop;
	const double Lowest = ui->typeCombo->currentIndex() == 3 ? 0 : Start;

	const double Lower = New.lower < Lowest ? Lowest : New.lower;
	const double Upper = New.upper > Highest ? Highest : New.upper;

	ui->Plot->xAxis->blockSignals(true);

	if (Lower == Lowest || Upper == Highest) ui->Plot->xAxis->setRange(Old);
	else ui->Plot->xAxis->setRange(Lower, Upper);

	ui->Plot->xAxis->blockSignals(false);
}

void ChartWidget::PlotResults(QFutureWatcher<RESULT>* Watcher, int Index)
{
	RESULT Result = Watcher->resultAt(Index);
	QPen Pen = ui->Plot->yAxis->basePen();
	CHART Data;

	if (Plots.contains(Result.Function)) deletePlotable(Plots.take(Result.Function));

	Pen.setColor(Colors.isEmpty() ? Qt::black : Colors.takeFirst());

	QPen Repen = Pen; QPen Impen = Pen;

	Repen.setStyle(Qt::DashLine);
	Impen.setStyle(Qt::DotLine);

	Data.Values = ui->Plot->addGraph(ui->Plot->xAxis, ui->Plot->yAxis);
	Data.Values->setData(Result.Arguments, Result.Values);
	Data.Values->setName(Result.Function);
	Data.Values->setPen(Pen);

	Data.Integrals = ui->Plot->addGraph(ui->Plot->xAxis, ui->Plot->yAxis);
	Data.Integrals->setData(Result.Arguments, Result.Integrals);
	Data.Integrals->setName(Result.Function);
	Data.Integrals->setPen(Pen);

	Data.Derivatives = ui->Plot->addGraph(ui->Plot->xAxis, ui->Plot->yAxis);
	Data.Derivatives->setData(Result.Arguments, Result.Derivatives);
	Data.Derivatives->setName(Result.Function);
	Data.Derivatives->setPen(Pen);

	Data.Spectrum = ui->Plot->addGraph(ui->Plot->xAxis, ui->Plot->yAxis);
	Data.Spectrum->setData(Result.Freqargs, Result.Transform);
	Data.Spectrum->setName(Result.Function);
	Data.Spectrum->setPen(Pen);

	Data.Repart = ui->Plot->addGraph(ui->Plot->xAxis, ui->Plot->yAxis);
	Data.Repart->setData(Result.Freqargs, Result.Realis);
	Data.Repart->setName(Result.Function);
	Data.Repart->setPen(Repen);

	Data.Impart = ui->Plot->addGraph(ui->Plot->xAxis, ui->Plot->yAxis);
	Data.Impart->setData(Result.Freqargs, Result.Imaginaris);
	Data.Impart->setName(Result.Function);
	Data.Impart->setPen(Impen);

	Data.Values->setVisible(ui->typeCombo->currentIndex() == 0);
	Data.Integrals->setVisible(ui->typeCombo->currentIndex() == 1);
	Data.Derivatives->setVisible(ui->typeCombo->currentIndex() == 2);

	Data.Spectrum->setVisible(ui->typeCombo->currentIndex() == 3  && ui->actionAbs->isChecked());
	Data.Repart->setVisible(ui->typeCombo->currentIndex() == 3 && ui->actionRe->isChecked());
	Data.Impart->setVisible(ui->typeCombo->currentIndex() == 3 && ui->actionIm->isChecked());

	Data.Integrals->removeFromLegend();
	Data.Derivatives->removeFromLegend();
	Data.Spectrum->removeFromLegend();
	Data.Repart->removeFromLegend();
	Data.Impart->removeFromLegend();

	Plots.insert(Result.Function, Data);

	ui->Plot->replot();
}

void ChartWidget::FinishReploting(void)
{
	QMutexLocker Locker(&Synchronizer);

	Finished = true;
	if (Replot) ReplotCharts();
	Replot = false;
}

void ChartWidget::ZoomCheckChanged(void)
{
	Qt::Orientations Orientation;

	if (ui->xrangeCheck->isChecked()) Orientation |=  Qt::Horizontal;
	if (ui->yrangeCheck->isChecked()) Orientation |=  Qt::Vertical;

	ui->Plot->axisRect()->setRangeZoom(Orientation);
}

void ChartWidget::PlotTypeChanged(int Type)
{
	bool View = Type == 3;
	bool Rescale = View != Lastview;

	for (auto Plot : Plots)
	{
		Plot.Values->setVisible(Type == 0);
		Plot.Integrals->setVisible(Type == 1);
		Plot.Derivatives->setVisible(Type == 2);

		Plot.Spectrum->setVisible(Type == 3 && ui->actionAbs->isChecked());
		Plot.Repart->setVisible(Type == 3 && ui->actionRe->isChecked());
		Plot.Impart->setVisible(Type == 3 && ui->actionIm->isChecked());
	}

	Lastview = View;

	if (!Rescale) ui->Plot->replot();
	else ZoomButtonClicked();
}

void ChartWidget::SpectrumTypeChanged(void)
{
	for (auto Plot : Plots)
	{
		Plot.Spectrum->setVisible(ui->typeCombo->currentIndex() == 3 && ui->actionAbs->isChecked());
		Plot.Repart->setVisible(ui->typeCombo->currentIndex() == 3 && ui->actionRe->isChecked());
		Plot.Impart->setVisible(ui->typeCombo->currentIndex() == 3 && ui->actionIm->isChecked());
	}

	ui->Plot->replot();
}

void ChartWidget::SaveButtonClicked(void)
{
	const QString Path = QFileDialog::getSaveFileName(this, tr("Select file to save data"), QString(), tr("CSV files (*.csv)"));

	if (!Path.isEmpty())
	{
		QFile File(Path); if (File.open(QFile::WriteOnly | QFile::Text))
		{
			QTextStream Stream(&File);
			const char Separator = ';';

			Stream << tr("Time");
			for (const auto& Name : Charts) Stream << Separator << Name;
			Stream << endl;

			QCPGraph* CHART::*Current =
					ui->typeCombo->currentIndex() == 1 ? &CHART::Integrals :
					ui->typeCombo->currentIndex() == 2 ? &CHART::Derivatives :
					ui->typeCombo->currentIndex() == 3 ? &CHART::Spectrum :
												  &CHART::Values;

			for (const auto& Key : (Plots.first().*Current)->data()->keys())
			{
				Stream << Key;
				for (const auto& Plot : Plots) Stream << Separator << (Plot.*Current)->data()->value(Key).value;
				Stream << endl;
			}
		}
		else
		{
			QMessageBox::warning(this, tr("Error"), tr("Can't open selected file"));
		}
	}
}

void ChartWidget::ZoomButtonClicked()
{
	ui->Plot->xAxis->blockSignals(true);

	ui->Plot->xAxis->rescale(true);
	ui->Plot->yAxis->rescale(true);
	ui->Plot->replot();

	ui->Plot->xAxis->blockSignals(false);
}

void ChartWidget::FitButtonClicked(void)
{
	ui->Plot->xAxis->blockSignals(true);

	ui->Plot->yAxis->rescale(true);
	ui->Plot->replot();

	ui->Plot->xAxis->blockSignals(false);
}

void ChartWidget::CopyButtonClicked(void)
{
	QSettings Settings("K-Plots");

	Settings.beginGroup("Plot");

	const int Width = Settings.value("width", 680).toInt();
	const int Height = Settings.value("height", 400).toInt();
	const int Scale = Settings.value("scale", 1.0).toDouble();

	Settings.endGroup();

	QApplication::clipboard()->setPixmap(ui->Plot->toPixmap(Width, Height, Scale));
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

void ChartWidget::deletePlotable(const CHART& Plotable)
{
	if (Plotable.Values->pen().color() != Qt::black) Colors.insert(0, Plotable.Values->pen().color());

	ui->Plot->removePlottable(Plotable.Derivatives);
	ui->Plot->removePlottable(Plotable.Integrals);
	ui->Plot->removePlottable(Plotable.Spectrum);
	ui->Plot->removePlottable(Plotable.Values);
	ui->Plot->removePlottable(Plotable.Repart);
	ui->Plot->removePlottable(Plotable.Impart);
}

void ChartWidget::fitValue(double& Value)
{
	if (Value > QCPRange::maxRange) Value = QCPRange::maxRange;
	else if (Value < -QCPRange::maxRange) Value = -QCPRange::maxRange;
}
