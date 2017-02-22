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

#include "variableswidget.hpp"
#include "ui_variableswidget.h"

VariablesWidget::VariablesWidget(const VALIDATOR& Bind, QWidget* Parent)
: QWidget(Parent), ui(new Ui::VariablesWidget), Validator(Bind)
{
	ui->setupUi(this);

	ui->rightSpacer->changeSize(ui->addButton->sizeHint().width(), 0);
	ui->Variables->setItemDelegateForColumn(1, new ValueDelegate(this));
}

VariablesWidget::~VariablesWidget(void)
{
	delete ui;
}

void VariablesWidget::SetTitleWidget(TitleWidget* Widget)
{
	while (ui->toolsLayout->count())
	{
		QLayoutItem* I = ui->toolsLayout->takeAt(0);

		if (QWidget* W = I->widget())
			Widget->addRightWidget(W);
		else if (QSpacerItem* S = I->spacerItem())
			Widget->addRightSpacer(S);
	}
}

void VariablesWidget::SetValidator(const VALIDATOR& Bind)
{
	Validator = Bind;
}

void VariablesWidget::ResetValidator(void)
{
	SetValidator([] (auto) { return true; });
}

void VariablesWidget::ResetWidget(void)
{
	while (ui->Variables->rowCount()) ui->Variables->removeRow(0);
}

void VariablesWidget::ListItemChanged(QTableWidgetItem* Item)
{
	const int Col = Item->column();
	const int Row = Item->row();

	const QString Name = ui->Variables->item(Row, 0)->text();

	if (Col == 1)
	{
		emit onEditVariable(Name, Item->data(Qt::EditRole).toDouble());
	}
	else
	{
		if (Validator(Name))
		{
			emit onRenameVariable(Item->data(Qt::UserRole).toString(), Item->text());

			Item->setData(Qt::UserRole, Item->text());
		}
		else
		{
			Item->setText(Item->data(Qt::UserRole).toString());
		}
	}
}

void VariablesWidget::SearchEditChanged(const QString& Text)
{
	const int Rows = ui->Variables->rowCount();

	if (Text.size()) for (int i = 0; i < Rows; ++i)
	{
		ui->Variables->setRowHidden(i, !ui->Variables->item(i, 0)->text().contains(Text, Qt::CaseInsensitive));
	}
	else for (int i = 0; i < Rows; i++) ui->Variables->setRowHidden(i, false);
}

void VariablesWidget::AddButtonClicked(void)
{
	VariablesDialog* Dialog = new VariablesDialog(Validator, this);

	connect(Dialog, &VariablesDialog::onEditFinished, this, &VariablesWidget::AddVariable);
	connect(Dialog, &VariablesDialog::accepted, Dialog, &VariablesDialog::deleteLater);
	connect(Dialog, &VariablesDialog::rejected, Dialog, &VariablesDialog::deleteLater);

	Dialog->open();
}

void VariablesWidget::RemoveButtonClicked(void)
{
	const int Row = ui->Variables->currentRow();

	if (Row != -1)
	{
		const QString Name = ui->Variables->item(Row, 0)->text();

		ui->Variables->removeRow(Row);

		emit onRemoveVariable(Name);
	}
}

void VariablesWidget::AddVariable(const QString& Name, double Value)
{
	const int Rows = ui->Variables->rowCount();

	const bool Sorting = ui->Variables->isSortingEnabled();
	const bool Block = ui->Variables->blockSignals(true);

	ui->Variables->setSortingEnabled(false);
	ui->Variables->insertRow(Rows);

	QTableWidgetItem* NameBox = new QTableWidgetItem(Name);
	QTableWidgetItem* ValueBox = new QTableWidgetItem();

	NameBox->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
	NameBox->setData(Qt::UserRole, Name);

	ValueBox->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
	ValueBox->setData(Qt::EditRole, Value);

	ui->Variables->setItem(Rows, 0, NameBox);
	ui->Variables->setItem(Rows, 1, ValueBox);

	ui->Variables->blockSignals(Block);
	ui->Variables->setSortingEnabled(Sorting);

	ui->Variables->scrollToItem(NameBox);

	emit onAddVariable(Name, Value);
}
