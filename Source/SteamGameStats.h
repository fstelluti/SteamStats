//
// A C++ GUI application that analyses and displays gaming statistics from Steam using R and Rinside
//
// Copyright (C) 2011  Dirk Eddelbuettel and Romain Francois
//
// Modified by: Francois Stelluti

#ifndef SteamGameStats_H
#define SteamGameStats_H

#include <RInside.h>

#include <QtGui>
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>
#include <QFormLayout>
#include <QGroupBox>
#include <QButtonGroup>
#include <QMainWindow>
#include <QComboBox>
#include <QHBoxLayout>
#include <QSlider>
#include <QSpinBox>
#include <QLabel>
#include <QTemporaryFile>
#include <QSvgWidget>

class SteamGameStats : public QMainWindow
{
    Q_OBJECT

public:
    SteamGameStats(RInside & R);
    ~SteamGameStats();

private slots:
    void setSteamYearDataFile(int year); //Sets the year to selected the correct data file
    void readFile(QString file);         //Reads file into R
    void getStatsByYear();               //Get all statistics based on selected year

private:
    void setupDisplay(void);    // Set up the GUI components
    void plot(void);            // Run a plot of (Update later)
    void filterFile(void);      // modify the richer SVG produced by R

    int getNumGames() const;          // Statistic to get number of games
    double getAvgPrice() const;       // Statistic to get average price of all games
    std::string getPrice() const;          //Gets the properly formatted price form the csv file
    std::string getAvgOwners() const;          //Gets the properly formatted number of owners form the csv file

    QSvgWidget *m_svg;          // the SVG device
    RInside & m_R;              // reference to the R instance passed to constructor
    QString m_tempfile;         // name of file used by R for plots
    QString m_svgfile;          // another temp file, this time from Qt
    int m_year;
    QString m_file;             // Location of file with Steam data

    int m_numGames;
    double m_avgPrice;
};

#endif
