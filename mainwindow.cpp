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

#include "mainwindow.hpp"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget* Parent)
: QMainWindow(Parent), ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	About = new AboutDialog(this);
	Start = new QDoubleSpinBox(this);
	Stop = new QDoubleSpinBox(this);
	Samples = new QSpinBox(this);

	Start->setMinimum(-10000.0);
	Stop->setMaximum(10000.0);
	Samples->setMinimum(10);
	Samples->setMaximum(100000);

	Start->setValue(0);
	Stop->setValue(10);
	Samples->setValue(100);

	Start->setMinimumWidth(100);
	Stop->setMinimumWidth(100);
	Samples->setMinimumWidth(100);

	Start->setPrefix(tr("From "));
	Stop->setPrefix(tr("to "));
	Samples->setPrefix(tr("Plot "));

	ui->actionsTool->addSeparator();
	ui->actionsTool->addWidget(Start);
	ui->actionsTool->addWidget(Stop);
	ui->actionsTool->addSeparator();
	ui->actionsTool->addWidget(Samples);

	QSettings Settings("K-Plots");

	Settings.beginGroup("Window");

	setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::TabPosition::North);
	restoreGeometry(Settings.value("size").toByteArray());
	restoreState(Settings.value("layout").toByteArray());
	centralWidget()->deleteLater();

	Settings.endGroup();

	PlotSamplesChanged(Samples->value());
	PlotRangeChanged();
	AddPlot(tr("Plot"));

	Variables.Add("t", KLVariables::NUMBER, KLVariables::KLSCALLBACK(), false);
	Variables.Add("dt", KLVariables::NUMBER, KLVariables::KLSCALLBACK(), false);

	if (isMaximized()) setGeometry(QApplication::desktop()->availableGeometry(this));

	ui->variablesWidget->SetValidator([this] (const QString& Name) -> bool
	{
		return !Variables.Exists(Name.toStdString().c_str());
	});

	ui->functionsWidget->SetValidator([this] (const QString& Name, const QString& Code, bool New) -> QString
	{
		const QString Script = QString("define %1;\n%2\nend;").arg(Name).arg(Code);
		const KLString Label = Name.toStdString().c_str();

		if (!New && Functions.Exists(Label)) return tr("This name is used by another function");
		else
		{
			KLScriptbinding Engine(&Variables);

			Engine.Functions = Functions;
			Engine.Functions.Delete(Label);

			if (Engine.Validate(Script)) return QString();
			else return tr("Error in line %1: %2")
					.arg(Engine.GetLine())
					.arg(Engine.GetMessage());
		}
	});

	ui->plotslistWidget->SetValidator([this] (const QString& Name) -> bool
	{
		for (const auto Plot: Plots) if (Plot->objectName() == Name) return false; return true;
	});

	connect(ui->variablesWidget, &VariablesWidget::onAddVariable, this, &MainWindow::AddVariable);
	connect(ui->variablesWidget, &VariablesWidget::onEditVariable, this, &MainWindow::EditVariable);
	connect(ui->variablesWidget, &VariablesWidget::onRenameVariable, this, &MainWindow::RenameVariable);
	connect(ui->variablesWidget, &VariablesWidget::onRemoveVariable, this, &MainWindow::RemoveVariable);

	connect(ui->functionsWidget, &FunctionsWidget::onAddFunction, this, &MainWindow::AddFunction);
	connect(ui->functionsWidget, &FunctionsWidget::onEditFunction, this, &MainWindow::EditFunction);
	connect(ui->functionsWidget, &FunctionsWidget::onRemoveFunction, this, &MainWindow::RemoveFunction);

	connect(ui->plotslistWidget, &PlotslistWidget::onAddPlot, this, &MainWindow::AddPlot);
	connect(ui->plotslistWidget, &PlotslistWidget::onAddChart, this, &MainWindow::AddChart);
	connect(ui->plotslistWidget, &PlotslistWidget::onRenamePlot, this, &MainWindow::RenamePlot);
	connect(ui->plotslistWidget, &PlotslistWidget::onRemovePlot, this, &MainWindow::RemovePlot);
	connect(ui->plotslistWidget, &PlotslistWidget::onRemoveChart, this, &MainWindow::RemoveChart);

	connect(ui->actionAbout, &QAction::triggered, About, &AboutDialog::show);

	connect(Start, SIGNAL(valueChanged(double)), SLOT(PlotRangeChanged()));
	connect(Stop, SIGNAL(valueChanged(double)), SLOT(PlotRangeChanged()));
	connect(Samples, SIGNAL(valueChanged(int)), SLOT(PlotSamplesChanged(int)));
}

MainWindow::~MainWindow(void)
{
	QSettings Settings("K-Plots");

	Settings.beginGroup("Window");
	Settings.setValue("size", saveGeometry());
	Settings.setValue("layout", saveState());
	Settings.endGroup();

	delete ui;
}

void MainWindow::PlotSamplesChanged(int Count)
{
	Samples->setSuffix(tr(" sample(s)", 0, Count));

	emit onSamplesChanged(Count);
}

void MainWindow::PlotRangeChanged(void)
{
	Start->setMaximum(Stop->value() - 1.0);
	Stop->setMinimum(Start->value() + 1.0);

	emit onRangeChanged(Start->value(), Stop->value());
}

void MainWindow::AddVariable(const QString& Name, double Value)
{
	const KLString Label = Name.toStdString().c_str();

	if (!Variables.Exists(Label))
	{
		Variables.Add(Label, KLVariables::NUMBER, KLVariables::KLSCALLBACK(), false);

		if (sender() != ui->variablesWidget)
		{
			ui->variablesWidget->blockSignals(true);
			ui->variablesWidget->AddVariable(Name, Value);
			ui->variablesWidget->blockSignals(false);
		}

		Variables[Label] = Value;
	}
}

void MainWindow::EditVariable(const QString& Name, double Value)
{
	const KLString Label = Name.toStdString().c_str();

	if (Variables.Exists(Label)) Variables[Label] = Value;

	emit onReplotRequest();
}

void MainWindow::RenameVariable(const QString& Old, const QString& New)
{
	const KLString OldLabel = Old.toStdString().c_str();
	const KLString NewLabel = New.toStdString().c_str();

	if (Variables.Exists(OldLabel)) Variables.Rename(OldLabel, NewLabel);

	emit onReplotRequest();
}

void MainWindow::RemoveVariable(const QString& Name)
{
	const KLString Label = Name.toStdString().c_str();

	if (Variables.Exists(Label)) Variables.Delete(Label);

	emit onReplotRequest();
}

void MainWindow::AddFunction(const QString& Name, const QString& Code)
{
	const KLString Label = Name.toStdString().c_str();
	const KLString Function = Code.toStdString().c_str();

	if (!Functions.Exists(Label))
	{
		const QString Script = QString("define %1;\n%2\nend;").arg(Name).arg(Code);

		KLScriptbinding Engine(&Variables);

		Engine.Functions = Functions;

		if (Engine.Validate(Script)) Functions.Insert(Function, Label);

		if (sender() != ui->functionsWidget)
		{
			ui->functionsWidget->blockSignals(true);
			ui->functionsWidget->AddFunction(Name, Code);
			ui->functionsWidget->blockSignals(false);
		}
	}
}

void MainWindow::EditFunction(const QString& Name, const QString& Code)
{
	const KLString Label = Name.toStdString().c_str();
	const KLString Function = Code.toStdString().c_str();

	if (Functions.Exists(Label))
	{
		const QString Script = QString("define %1;\n%2\nend;").arg(Name).arg(Code);

		KLScriptbinding Engine(&Variables);

		Engine.Functions = Functions;
		Engine.Functions.Delete(Label);

		if (Engine.Validate(Script)) Functions[Label] = Function;
	}

	emit onReplotRequest();
}

void MainWindow::RemoveFunction(const QString& Name)
{
	const KLString Label = Name.toStdString().c_str();

	if (Functions.Exists(Label)) Functions.Delete(Label);

	ui->plotslistWidget->RemoveFunction(Name);

	emit onReplotRequest();
}

void MainWindow::AddPlot(const QString& Name)
{
	for (const auto Plot: Plots) if (Plot->objectName() == Name) return;

	QDockWidget* Plot = new QDockWidget(this);
	ChartWidget* Chart = new ChartWidget(Plot);

	Chart->SetPlotParams(Start->value(), Stop->value(), Samples->value());
	Chart->SetScriptParams(&Variables, &Functions);

	Plot->setWidget(Chart);
	Plot->setObjectName(Name);
	Plot->setWindowTitle(Name);

	addDockWidget(Qt::LeftDockWidgetArea, Plot);

	if (Plots.size()) tabifyDockWidget(Plots.last(), Plot);

	if (sender() != ui->plotslistWidget)
	{
		ui->plotslistWidget->blockSignals(true);
		ui->plotslistWidget->AddPlot(Name);
		ui->plotslistWidget->blockSignals(false);
	}

	Plots.append(Plot);

	connect(this, &MainWindow::onReplotRequest, Chart, &ChartWidget::ReplotCharts);
	connect(this, &MainWindow::onRangeChanged, Chart, &ChartWidget::RangeChanged);
	connect(this, &MainWindow::onSamplesChanged, Chart, &ChartWidget::SamplesChanged);

	connect(Chart, &ChartWidget::onAddChart, this, &MainWindow::AddChart);
}

void MainWindow::RenamePlot(const QString& Old, const QString& New)
{
	for (const auto Plot: Plots) if (Plot->objectName() == Old)
	{
		Plot->setObjectName(New);
		Plot->setWindowTitle(New);
	}
}

void MainWindow::RemovePlot(const QString& Name)
{
	for (const auto Plot: Plots) if (Plot->objectName() == Name)
	{
		Plots.removeAll(Plot);
		Plot->deleteLater();
	}

	ui->plotslistWidget->RemoveFunction(Name);
}

void MainWindow::AddChart(const QString& Plot, const QString& Function)
{
	if (sender() == ui->plotslistWidget)
	{
		for (const auto Chart: Plots) if (Chart->objectName() == Plot)
		{
			if (ChartWidget* Widget = qobject_cast<ChartWidget*>(Chart->widget()))
			{
			    Widget->AddChart(Function);
			}
		}
	}
	else
	{
		ui->plotslistWidget->AddChart(Plot, Function);
	}
}

void MainWindow::RemoveChart(const QString& Plot, const QString& Function)
{
	for (const auto Chart: Plots) if (Chart->objectName() == Plot)
	{
		if (ChartWidget* Widget = qobject_cast<ChartWidget*>(Chart->widget()))
		{
			Widget->RemoveChart(Function);
		}
	}
}
