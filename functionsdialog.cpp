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

#include "functionsdialog.hpp"
#include "ui_functionsdialog.h"

FunctionsDialog::FunctionsDialog(const VALIDATOR& Bind, QWidget* Parent)
: QDialog(Parent), ui(new Ui::FunctionsDialog), Validator(Bind)
{
	ui->setupUi(this);

	ui->Name->setValidator(new QRegExpValidator(QRegExp("^[A-z]+[A-z0-9]*$"), this));
	ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(false);
}

FunctionsDialog::~FunctionsDialog(void)
{
	delete ui;
}

void FunctionsDialog::SetValidator(const VALIDATOR& Bind)
{
	Validator = Bind;
}

void FunctionsDialog::ResetValidator(void)
{
	SetValidator([] (auto, auto, auto) { return QString(); });
}

void FunctionsDialog::DialogDataChanged()
{
	ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(
				!ui->Name->text().isEmpty() &&
				!ui->Code->document()->toPlainText().isEmpty());
}

void FunctionsDialog::open(const QString& Name, const QString& Code)
{
	ui->Name->setText(Name);
	ui->Name->setEnabled(false);
	ui->Code->setPlainText(Code);

	QDialog::open();
}

void FunctionsDialog::open(void)
{
	ui->Name->clear();
	ui->Name->setEnabled(true);
	ui->Code->clear();

	QDialog::open();
}

void FunctionsDialog::accept(void)
{
	const QString Name = ui->Name->text();
	const QString Code = ui->Code->document()->toPlainText();

	QString Message = Validator(Name, Code, !ui->Name->isEnabled());

	if (Message.isEmpty())
	{
		emit onEditFinished(Name, Code);

		QDialog::accept();
	}
	else QMessageBox::critical(this, tr("Error"), Message);
}
