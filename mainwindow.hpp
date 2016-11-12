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

#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#define VARDUMP(V) \
	for (const auto* Space = &V; Space; Space = Space->Parent) \
	for (const auto& Var : *Space) qDebug() << Var.Index << "=" << Var.Value.ToNumber();

#include <boost/bind.hpp>

#include <KLLibs.hpp>

#include <QMainWindow>
#include <QSettings>
#include <QDebug>

#include "titlewidget.hpp"

namespace Ui
{
	class MainWindow;
}

class MainWindow : public QMainWindow
{

		Q_OBJECT

	private:

		KLMap<KLString, KLString> Functions;
		KLVariables Variables;

		Ui::MainWindow* ui;

	public:

		explicit MainWindow(QWidget* Parent = nullptr);
		virtual ~MainWindow(void) override;

	private slots:

		void AddVariable(const QString& Name, double Value);
		void EditVariable(const QString& Name, double Value);
		void RenameVariable(const QString& Old, const QString& New);
		void RemoveVariable(const QString& Name);

		void AddFunction(const QString& Name, const QString& Code);
		void EditFunction(const QString& Name, const QString& Code);
		void RemoveFunction(const QString& Name);

};

#endif // MAINWINDOW_HPP
