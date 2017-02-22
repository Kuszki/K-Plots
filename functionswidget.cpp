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

#include "functionswidget.hpp"
#include "ui_functionswidget.h"

FunctionsWidget::FunctionsWidget(const VALIDATOR& Bind, QWidget* Parent)
: QWidget(Parent), ui(new Ui::FunctionsWidget), Validator(Bind)
{
	ui->setupUi(this);

	ui->rightSpacer->changeSize(ui->addButton->sizeHint().width(), 0);
}

FunctionsWidget::~FunctionsWidget(void)
{
	delete ui;
}

void FunctionsWidget::SetTitleWidget(TitleWidget* Widget)
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

void FunctionsWidget::SetValidator(const VALIDATOR& Bind)
{
	Validator = Bind;
}

void FunctionsWidget::ResetValidator(void)
{
	SetValidator([] (auto, auto, auto) { return QString(); });
}

void FunctionsWidget::ResetWidget(void)
{
	while (ui->Functions->count()) delete ui->Functions->takeItem(0);
}

void FunctionsWidget::ListItemChanged(QListWidgetItem* Item)
{
	FunctionsDialog* Dialog = new FunctionsDialog(Validator, this);

	connect(Dialog, &FunctionsDialog::onEditFinished, [Item] (const QString&, const QString& Code) -> void
	{
		Item->setData(Qt::UserRole, Code);
	});

	connect(Dialog, &FunctionsDialog::onEditFinished, this, &FunctionsWidget::onEditFunction);
	connect(Dialog, &FunctionsDialog::accepted, Dialog, &FunctionsDialog::deleteLater);
	connect(Dialog, &FunctionsDialog::rejected, Dialog, &FunctionsDialog::deleteLater);

	Dialog->open(Item->text(), Item->data(Qt::UserRole).toString());
}

void FunctionsWidget::SearchEditChanged(const QString& Text)
{
	const int Rows = ui->Functions->count();

	if (Text.size()) for (int i = 0; i < Rows; ++i)
	{
		ui->Functions->setRowHidden(i, !ui->Functions->item(i)->text().contains(Text, Qt::CaseInsensitive));
	}
	else for (int i = 0; i < Rows; i++) ui->Functions->setRowHidden(i, false);
}

void FunctionsWidget::AddButtonClicked()
{
	FunctionsDialog* Dialog = new FunctionsDialog(Validator, this);

	connect(Dialog, &FunctionsDialog::onEditFinished, this, &FunctionsWidget::AddFunction);
	connect(Dialog, &FunctionsDialog::accepted, Dialog, &FunctionsDialog::deleteLater);
	connect(Dialog, &FunctionsDialog::rejected, Dialog, &FunctionsDialog::deleteLater);

	Dialog->open();
}

void FunctionsWidget::RemoveButtonClicked()
{
	const int Row = ui->Functions->currentRow();

	if (Row != -1)
	{
		const QString Name = ui->Functions->item(Row)->text();

		delete ui->Functions->takeItem(Row);

		emit onRemoveFunction(Name);
	}
}

void FunctionsWidget::AddFunction(const QString& Name, const QString& Code)
{
	const bool Sorting = ui->Functions->isSortingEnabled();
	const bool Block = ui->Functions->blockSignals(true);

	ui->Functions->setSortingEnabled(false);

	QListWidgetItem* Function = new QListWidgetItem(Name);

	Function->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
	Function->setData(Qt::UserRole, Code);
	Function->setText(Name);

	ui->Functions->addItem(Function);

	ui->Functions->blockSignals(Block);
	ui->Functions->setSortingEnabled(Sorting);

	ui->Functions->sortItems();

	emit onAddFunction(Name, Code);
}
