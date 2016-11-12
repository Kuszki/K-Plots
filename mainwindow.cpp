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

	QSettings Settings("K-Plots");

	Settings.beginGroup("Window");

	setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::TabPosition::North);

	restoreGeometry(Settings.value("size").toByteArray());
	restoreState(Settings.value("layout").toByteArray());
	centralWidget()->deleteLater();

	if (isMaximized()) setGeometry(QApplication::desktop()->availableGeometry(this));

	Settings.endGroup();

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

	connect(ui->variablesWidget, &VariablesWidget::onAddVariable, this, &MainWindow::AddVariable);
	connect(ui->variablesWidget, &VariablesWidget::onEditVariable, this, &MainWindow::EditVariable);
	connect(ui->variablesWidget, &VariablesWidget::onRenameVariable, this, &MainWindow::RenameVariable);
	connect(ui->variablesWidget, &VariablesWidget::onRemoveVariable, this, &MainWindow::RemoveVariable);

	connect(ui->functionsWidget, &FunctionsWidget::onAddFunction, this, &MainWindow::AddFunction);
	connect(ui->functionsWidget, &FunctionsWidget::onEditFunction, this, &MainWindow::EditFunction);
	connect(ui->functionsWidget, &FunctionsWidget::onRemoveFunction, this, &MainWindow::RemoveFunction);
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

void MainWindow::AddVariable(const QString& Name, double Value)
{
	const KLString Label = Name.toStdString().c_str();

	if (!Variables.Exists(Label))
	{
		Variables.Add(Label, KLVariables::NUMBER);

		Variables[Label].SetReadonly(true);
		Variables[Label] = Value;
	}
}

void MainWindow::EditVariable(const QString& Name, double Value)
{
	const KLString Label = Name.toStdString().c_str();

	if (Variables.Exists(Label)) Variables[Label] = Value;
}

void MainWindow::RenameVariable(const QString& Old, const QString& New)
{
	const KLString OldLabel = Old.toStdString().c_str();
	const KLString NewLabel = New.toStdString().c_str();

	if (Variables.Exists(OldLabel)) Variables.Rename(OldLabel, NewLabel);
}

void MainWindow::RemoveVariable(const QString& Name)
{
	const KLString Label = Name.toStdString().c_str();

	if (Variables.Exists(Label)) Variables.Delete(Label);
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
}

void MainWindow::RemoveFunction(const QString& Name)
{
	const KLString Label = Name.toStdString().c_str();

	if (Functions.Exists(Label)) Functions.Delete(Label);
}
