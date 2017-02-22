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

#include "plotslistwidget.hpp"
#include "ui_plotslistwidget.h"

PlotslistWidget::PlotslistWidget(const VALIDATOR& Bind, QWidget* Parent)
: QWidget(Parent), ui(new Ui::PlotslistWidget), Validator(Bind)
{
	ui->setupUi(this);

	ui->rightSpacer->changeSize(ui->addButton->sizeHint().width(), 0);

	setAcceptDrops(true);
}

PlotslistWidget::~PlotslistWidget(void)
{
	delete ui;
}

void PlotslistWidget::dragEnterEvent(QDragEnterEvent* Event)
{
	Event->acceptProposedAction();
}

void PlotslistWidget::dragMoveEvent(QDragMoveEvent* Event)
{
	Event->acceptProposedAction();
}

void PlotslistWidget::dropEvent(QDropEvent* Event)
{
	if (QListWidget* Source = qobject_cast<QListWidget*>(Event->source()))
	{
		QTreeWidgetItem* Parent = ui->Plots->itemAt(Event->pos());
		const QString Function = Source->currentItem()->text();

		if (!Parent || Parent->parent()) Event->ignore();
		else
		{
			for (int i = 0; i < Parent->childCount(); ++i) if (Parent->child(i)->text(0) == Function)
			{
				Event->ignore(); return;
			}

			QTreeWidgetItem* Item = new QTreeWidgetItem();

			Item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			Item->setText(0, Function);

			Parent->addChild(Item);
			Event->accept();

			emit onAddChart(Parent->text(0), Function);
		}
	}
}

void PlotslistWidget::SetTitleWidget(TitleWidget* Widget)
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

void PlotslistWidget::SetValidator(const VALIDATOR& Bind)
{
	Validator = Bind;
}

void PlotslistWidget::ResetValidator(void)
{
	SetValidator([] (auto) { return true; });
}

void PlotslistWidget::ResetWidget(void)
{
	for (int i = 0; i < ui->Plots->topLevelItemCount(); ++i)
	{
		QTreeWidgetItem* Parent = ui->Plots->topLevelItem(i);

		while (Parent->childCount()) delete Parent->takeChild(0);
	}
}

void PlotslistWidget::TreeItemChanged(QTreeWidgetItem* Item)
{
	if (!Item->text(0).isEmpty() && Validator(Item->text(0)))
	{
		emit onRenamePlot(Item->data(0, Qt::UserRole).toString(), Item->text(0));

		Item->setData(0, Qt::UserRole, Item->text(0));
	}
	else
	{
		Item->setText(0, Item->data(0, Qt::UserRole).toString());
	}
}

void PlotslistWidget::SearchEditChanged(const QString& Text)
{
	if (Text.size()) for (int i = 0; i < ui->Plots->topLevelItemCount(); ++i)
	{
		QTreeWidgetItem* Item = ui->Plots->topLevelItem(i);

		bool Match = Item->text(0).contains(Text, Qt::CaseInsensitive);

		if (!Match) for (int j = 0; j < Item->childCount(); ++j)
		{
			bool Child = Item->child(j)->text(0).contains(Text, Qt::CaseInsensitive);

			Item->child(j)->setHidden(!Child);

			if (Child) Match = true;
		}

		Item->setHidden(!Match);
	}
	else for (int i = 0; i < ui->Plots->topLevelItemCount(); ++i) ui->Plots->topLevelItem(i)->setHidden(false);
}

void PlotslistWidget::AddButtonClicked(void)
{
	int Index = ui->Plots->topLevelItemCount();

	while (!Validator(tr("Plot %1").arg(Index))) ++Index;

	AddPlot(tr("Plot %1").arg(Index));
}

void PlotslistWidget::RemoveButtonClicked(void)
{
	QTreeWidgetItem* Item = ui->Plots->currentItem();

	if (QTreeWidgetItem* Parent = Item->parent())
	{
		emit onRemoveChart(Parent->text(0), Item->text(0)); delete Item;
	}
	else if (ui->Plots->topLevelItemCount() > 1)
	{
		emit onRemovePlot(Item->text(0)); delete Item;
	}
}

void PlotslistWidget::AddPlot(const QString& Name)
{
	QTreeWidgetItem* Item = new QTreeWidgetItem();

	Item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
	Item->setData(0, Qt::UserRole, Name);
	Item->setText(0, Name);

	ui->Plots->addTopLevelItem(Item);
	ui->Plots->sortByColumn(0, Qt::AscendingOrder);

	emit onAddPlot(Name);
}

void PlotslistWidget::RemovePlot(const QString& Name)
{
	for (auto Item : ui->Plots->findItems(Name, Qt::MatchExactly)) delete Item;
}

void PlotslistWidget::RemoveFunction(const QString& Function)
{
	for (int i = 0; i < ui->Plots->topLevelItemCount(); ++i)
	{
		QTreeWidgetItem* Parent = ui->Plots->topLevelItem(i);
		QList<QTreeWidgetItem*> Items;

		for (int j = 0; j < Parent->childCount(); ++j)
		{
			QTreeWidgetItem* Item = Parent->child(j);

			if (Item->text(0) == Function) Items.append(Item);
		}

		for (auto Item : Items) delete Item;
	}
}

void PlotslistWidget::AddChart(const QString& Plot, const QString& Name)
{
	if (QTreeWidgetItem* Parent = ui->Plots->findItems(Plot, Qt::MatchExactly).first())
	{
		QTreeWidgetItem* Item = new QTreeWidgetItem();

		Item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		Item->setText(0, Name);

		Parent->addChild(Item);

		emit onAddChart(Plot, Name);
	}
}
