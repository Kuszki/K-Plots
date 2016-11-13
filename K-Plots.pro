#* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
#*                                                                         *
#*  Plot simple charts using QCustomPlot and KLScript                      *
#*  Copyright (C) 2016  Łukasz "Kuszki" Dróżdż  l.drozdz@openmailbox.org   *
#*                                                                         *
#*  This program is free software: you can redistribute it and/or modify   *
#*  it under the terms of the GNU General Public License as published by   *
#*  the  Free Software Foundation, either  version 3 of the  License, or   *
#*  (at your option) any later version.                                    *
#*                                                                         *
#*  This  program  is  distributed  in the hope  that it will be useful,   *
#*  but WITHOUT ANY  WARRANTY;  without  even  the  implied  warranty of   *
#*  MERCHANTABILITY  or  FITNESS  FOR  A  PARTICULAR  PURPOSE.  See  the   *
#*  GNU General Public License for more details.                           *
#*                                                                         *
#*  You should have  received a copy  of the  GNU General Public License   *
#*  along with this program. If not, see http://www.gnu.org/licenses/.     *
#*                                                                         *
#* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

QT			+=	core gui widgets printsupport concurrent

TARGET		=	K-Plots
TEMPLATE		=	app
CONFIG		+=	c++14

DEFINES		+=	USING_BOOST USING_QT


SOURCES		+=	main.cpp \
				mainwindow.cpp \
				variableswidget.cpp \
				variablesdialog.cpp \
				aboutdialog.cpp \
				titlewidget.cpp \
				functionswidget.cpp \
				functionsdialog.cpp \
				plotslistwidget.cpp \
				chartwidget.cpp

HEADERS		+=	mainwindow.hpp \
				variableswidget.hpp \
				variablesdialog.hpp \
				aboutdialog.hpp \
				titlewidget.hpp \
				functionswidget.hpp \
				functionsdialog.hpp \
				plotslistwidget.hpp \
				chartwidget.hpp

FORMS		+=	mainwindow.ui \
				variableswidget.ui \
				variablesdialog.ui \
				aboutdialog.ui \
				titlewidget.ui \
				functionswidget.ui \
				functionsdialog.ui \
				plotslistwidget.ui \
				chartwidget.ui

RESOURCES		+=	resources.qrc

LIBS			+=	-L$$PWD/../build-KLLibs -lkllibs
LIBS			+=	-lqcustomplot

INCLUDEPATH	+=	$$PWD/../KLLibs

TRANSLATIONS	+=	k-plots_pl.ts

QMAKE_CXXFLAGS	+=	-s -std=c++14 -march=native
