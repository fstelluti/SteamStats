//
// A C++ GUI application that analyses and displays gaming statistics from Steam using R and Rinside
// Initializes the GUI and uses SVG for the graph
//
// Author: Francois Stelluti
//
// Original file by Dirk Eddelbuettel and Romain Francois - Copyright (C) 2011
//

#include <QApplication>
#include "SteamGameStats.h"

int main(int argc, char *argv[])
{
    RInside R(argc, argv);  		// create an embedded R instance

    QApplication app(argc, argv);
    SteamGameStats steamGameStats(R);		// pass R instance by reference

    return app.exec();
}
