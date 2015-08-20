// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-
//
// Qt usage example for RInside, inspired by the standard 'density
// sliders' example for other GUI toolkits
//
// Copyright (C) 2011 - 2013  Dirk Eddelbuettel and Romain Francois

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
    void setSteamYearDataFile(int year);
    //void runRandomDataCmd(void);    //Needed?
    void readFile(QString file);    //Reads file into R
    void getStatsByYear();

private:
    void setupDisplay(void);    // standard GUI boilderplate of arranging things
    void plot(void);            // run a density plot in R and update the
    void filterFile(void);      // modify the richer SVG produced by R

    int getNumGames();
    double getAvgPrice();

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
