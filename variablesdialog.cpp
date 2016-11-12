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

#include "variablesdialog.hpp"
#include "ui_variablesdialog.h"

VariablesDialog::VariablesDialog(const VALIDATOR& Bind, QWidget* Parent)
: QDialog(Parent), ui(new Ui::VariablesDialog), Validator(Bind)
{
	ui->setupUi(this);

	ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(false);
	ui->Name->setValidator(new QRegExpValidator(QRegExp("^[A-z]+[A-z0-9]*$"), this));
	ui->Value->setValidator(new QDoubleValidator(this));
}

VariablesDialog::~VariablesDialog(void)
{
	delete ui;
}

void VariablesDialog::SetValidator(const VALIDATOR& Bind)
{
	Validator = Bind;
}

void VariablesDialog::ResetValidator(void)
{
	SetValidator([] (auto) { return true; });
}

void VariablesDialog::DialogDataChanged(void)
{
	ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(
				!ui->Name->text().isEmpty() &&
				!ui->Value->text().isEmpty() &&
				Validator(ui->Name->text()));
}

void VariablesDialog::accept()
{
	emit onEditFinished(ui->Name->text(), QLocale().toDouble(ui->Value->text()));

	QDialog::accept();
}

void VariablesDialog::open(const QString& Name, double Value)
{
	ui->Name->setText(Name);
	ui->Value->setText(QLocale().toString(Value));

	QDialog::open();
}

void VariablesDialog::open(void)
{
	ui->Name->clear();
	ui->Value->clear();

	QDialog::open();
}
