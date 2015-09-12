//
// A C++ GUI application that analyses and displays gaming statistics from Steam using R and Rinside
// Initializes the GUI and uses SVG for the graph
//
// Author: Francois Stelluti
//
// Original file by Dirk Eddelbuettel and Romain Francois - Copyright (C) 2011
//

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
#include <QPushButton>

class SteamGameStats : public QMainWindow
{
    Q_OBJECT

public:
    SteamGameStats(RInside & R);
    ~SteamGameStats();

private:

    //Enum to store plot variable names
    enum plotVariable {
        Price,
        Userscore,
        Owners,
        Playtime
    };

    void setupDisplay(void);                                // Set up the GUI components
    void filterFile(void);                                  // modify the richer SVG produced by R

    //Gets the correct data for the plot. Pass paramaters by reference to be able to use global variables
    void getPlotData(std::string &_VariableStatement, std::string &_VariableName, plotVariable &_axis);

    //Perform a correlation test on two variables (as R commands) from the graph.
    //Also supply the names of those variables directly, which should be the same as in the one used in R
    void correlationTest(std::string var1, std::string var2, std::string name1, std::string name2);

    int getNumGames() const;          // Number of games
    double getAvgPrice() const;       // Average price of all games
    double getMaxPrice() const;       // Max price of all games
    double getAvgMetascore() const;   // Average Metascore
    double getTotalPlaytime() const;  // Total playtime of all games (in hours)
    double getPValue() const;         // p-value from the correlation test
    double getCorrCoefficiant() const;// Correlation Coefficient from the correlation test

    std::string getPrice();              //Gets the properly formatted price form the csv file
    std::string getSelectElementsOfSet(std::string column, bool isFirst); //Gets the properly formatted first or second elements of
                                                            //a set. If pass true, selects first elements (ex: [(2,1),(3,5)]->[2,3] )
    std::string getNumberOfHoursPlayed();  //Get the total number of time played, in hours

    QSvgWidget *m_svg;          // the SVG device
    RInside & m_R;              // reference to the R instance passed to constructor
    QString m_tempfile;         // name of file used by R for plots
    QString m_svgfile;          // another temp file, this time from Qt
    int m_year;
    QString m_file;             // location of file with Steam data

    int m_numGames;
    double m_avgPrice;
    double m_maxPrice;
    double m_avgMetascore;
    double m_totalPlaytime;
    double m_p_value;
    double m_corrCoeff;

    //Labels for each statistic
    QLabel *numGamesLabel, *avgPriceLabel, *maxPriceLabel, *avgMetaScoreLabel, *totalPlaytimeLabel;
    QLabel *corrCoefficientLabel, *p_valueLabel, *correlationTestMessageLabel, *correlationTestResultLabel;

    //Other UI components
    QComboBox *yearCombo, *plotVarComboX, *plotVarComboY;
    QGroupBox *estimationBox;
    QPushButton *correlationButton, *plotButton; //Used to preform a correlation test and to plot

private slots:

    void setSteamYearDataFile(int year); //Sets the year to selected the correct data file
    void readFile(QString file);         //Reads file into R
    void getStatsByYear();               //Get all statistics based on selected year
    void generateStatsAndPlot(int comboIndex);   //Generate the plot and statistics for the window based on the year selected
    void displayCorrelationTest(void);   //Display the results of the correlation test

    void plot(plotVariable x_axis, plotVariable y_axis);    // Run a plot of two selected variables, defined in plotVariable
    void plotWithSelectedVariables();       //Used to plot based on the values of both plot variable comboBoxes


};

#endif
