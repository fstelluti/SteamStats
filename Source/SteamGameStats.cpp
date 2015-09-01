//
// A C++ GUI application that analyses and displays gaming statistics from Steam using R and Rinside
// Initializes the GUI and uses SVG for the graph
//
// Author: Francois Stelluti
//
// Original file by Dirk Eddelbuettel and Romain Francois - Copyright (C) 2011
//

#include "SteamGameStats.h"
#include <exception>
#include <iostream>
#include <stdlib.h>

//Constructor sets to default year to 2015, the number of games considered and average price to zero.
SteamGameStats::SteamGameStats(RInside & R) : m_R(R), m_year(2015), m_numGames(0), m_avgPrice(0.0)
{
    //Set up temp files used for plots
    m_tempfile = QString::fromStdString(Rcpp::as<std::string>(m_R.parseEval("tfile <- tempfile()")));
    m_svgfile = QString::fromStdString(Rcpp::as<std::string>(m_R.parseEval("sfile <- tempfile()")));

    m_R.parseEvalQ("library(ggplot2);");    //Load the ggplot2 library

    //Instantiate labels and other UI components for each statistic
    numGamesLabel = new QLabel();
    avgPriceLabel = new QLabel();
    maxPriceLabel = new QLabel();
    avgMetaScoreLabel = new QLabel();
    totalPlaytimeLabel = new QLabel();

    yearCombo = new QComboBox;
    estimationBox = new QGroupBox();

    //Initialize and display the GUI
    setupDisplay();
}

SteamGameStats::~SteamGameStats() {}

void SteamGameStats::setupDisplay(void)  {
    //Window name
    QWidget *window = new QWidget;
    window->setWindowTitle("Steam Stats with using Rinside");

    //Sales by year component
    QGroupBox *yearBox = new QGroupBox("Steam sales by timeframe");

    //Year selection comboBox
    yearCombo->addItem("2015");
    yearCombo->addItem("2014");
    yearCombo->addItem("2013");
    yearCombo->addItem("2012");
    yearCombo->setFixedWidth(75);

    //Calculate statistics and plot based on the default year selected
//    QString defaultYear = QString::number(m_year);          //Convert year to string
//    readFile("Data/" + defaultYear + "_SteamStats.csv");    //Read file
//    getStatsByYear();                                       //Get all stats
//    plot();      //TODO Change/modify

    m_svg = new QSvgWidget();   //Initialize svg object

    generateStatsAndPlot(yearCombo->currentIndex());    //TODO Use this instead?

    //Connect comboBox to stats displayed
    QObject::connect(yearCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(generateStatsAndPlot(int)));

    //Set initial statistcs
    numGamesLabel->setText(QString::number(getNumGames()));
    avgPriceLabel->setText("$" + QString::number(getAvgPrice()));
    maxPriceLabel->setText("$" + QString::number(getMaxPrice()));
    avgMetaScoreLabel->setText(QString::number(getAvgMetascore()) + "%");
    totalPlaytimeLabel->setText(QString::number(getTotalPlaytime()) + " hours");

    //Add comboBox to 'Steam sales by year'
    QFormLayout *topLeft = new QFormLayout;
    topLeft->addRow(tr("Select year:"), yearCombo);

    //Set properties of yearBox
    yearBox->setMinimumSize(320,170);
    yearBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    yearBox->setLayout(topLeft);

    //Game statistics component
    estimationBox->setTitle("Game Stats for:    " + QString::number(m_year));

    //Add each statistic to 'Game Stats'
    QFormLayout *topRight = new QFormLayout;
    topRight->addRow(tr("Number of Games:"), numGamesLabel);
    topRight->addRow(tr("Average Price:"), avgPriceLabel);
    topRight->addRow(tr("Maximum Price:"), maxPriceLabel);
    topRight->addRow(tr("Average Metascore:"), avgMetaScoreLabel);
    topRight->addRow(tr("Total Playtime:"), totalPlaytimeLabel);

    //Set properties of estimationBox
    estimationBox->setMinimumSize(320,170);
    estimationBox->setMaximumSize(320,170);
    estimationBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    estimationBox->setLayout(topRight);

    //Adds both Top containers
    QHBoxLayout *upperlayout = new QHBoxLayout;
    upperlayout->addWidget(yearBox);
    upperlayout->addWidget(estimationBox);

    //Add the svg graph picture
    QHBoxLayout *lowerlayout = new QHBoxLayout;
    lowerlayout->addWidget(m_svg);

    QVBoxLayout *outer = new QVBoxLayout;
    outer->addLayout(upperlayout);
    outer->addLayout(lowerlayout);
    window->setLayout(outer);
    window->setMinimumSize(640,640);   //Set the size of the main window
    window->setMaximumSize(640,640);
    window->show();
}

//TODO Make the plot variable?
void SteamGameStats::plot()
{
    //TODO Currently, only a plot of the number of owners vs the price of a game

    //QString file = "/home/swag/Desktop/Github/R_SteamStats/test.svg";
    //std::string fileS = "/home/swag/Desktop/Github/R_SteamStats/test.svg";

    std::string svgFile = "svg(filename=tfile,width=6,height=5,pointsize=10); ";
    std::string price = getPrice();
    std::string owners = getSelectElementsOfSet("Owners", true);
    std::string data = "Owners <- as.numeric(Owners); dataPriceOwners <- data.frame(price, Owners); ";

    //Axes are constrained to eliminate outliers from view only
    //Best fit using a Local Polynomial Regression Model is used
    std::string plot = "print(ggplot(dataPriceOwners, aes(x=price, y=Owners, colour=Owners)) + geom_point(shape=16) "
                       " + labs(title='Price vs Average number of Owners', x='Price', y='AvgNumOwners') "
                       " + geom_smooth(method=loess,se=FALSE) "
                       " + scale_colour_gradientn(colours=rainbow(7),guide=FALSE)"
                       " + xlim(0,100) + ylim(0,200000) ); " ;  //TODO plot - Doesn't work with other years

    //Testing plot
    int plotvar = rand() % 100 + 1;
    std::string lala2 = std::to_string(plotvar);
    std::string plot2 = "plot(density(1:" + lala2 + "));" ;  //This Works, but eventually crashes
    //std::string plot2 = "print(ggplot(dataPriceOwners, aes(x=price, y=Owners)) + geom_point(shape=16));";

    std::string dev = "dev.off();";

    //Alternate saving method
    std::string svgSave = "ggsave(file='/home/swag/Desktop/Github/R_SteamStats/test.svg', plot=image, width=6, height=5);";

    //Build command and execute in R
    std::string cmd = svgFile + price + owners + data + plot + dev;
    //std::string cmd = price + owners + data + plot + svgSave;

    m_R.parseEvalQ(cmd);        //Parse and execute the string from R  --- FAILS here segmentation fault
    std::cout << "Plot grrrr" << std::endl;
    filterFile();               //Simplify the svg file for display by Qt
    m_svg->load(m_svgfile);

}

void SteamGameStats::setSteamYearDataFile(int year) {
    if( year != m_year)
        m_year = year;
}

void SteamGameStats::readFile(QString file)
{
    m_file = file;

    //Command to read the csv file
    std::string cmd = "SD <- read.csv(\"~/Desktop/Github/R_SteamStats/" + file.toStdString() + "\", header=TRUE)";
    m_R["SD"] = cmd;            //Store variable in R for future use
    m_R.parseEvalQNT(cmd);      //Parse and execute the command string
}

void SteamGameStats::getStatsByYear()
{
    std::string numGames = "nrow(SD)";    //Number of games (rows)
    std::string averagePrice = getPrice() + "avgPrice <- mean(price); avgPrice <- round(avgPrice, digits=2)";
    std::string maxPrice = getPrice() + "maxPrice <- max(price); maxPrice <- round(maxPrice, digits=2)";
    std::string avgMetascore = getSelectElementsOfSet("Userscore..Metascore.", false) +
                "avgMetascore <- gsub('\\\\(|\\\\)|\\\\%', '', Userscore..Metascore.);"   //Get rid of brackets and percent sign
                "avgMetascore <- round(mean(as.numeric(avgMetascore),na.rm=TRUE),2);";    //Calculate the mean, ignoring N/A's and rounding to 2 decimal places
    std::string totalPlayTime = getSelectElementsOfSet("Playtime..Median.", true) + getNumberOfHoursPlayed();

    Rcpp::NumericVector v;  //Store result as a vector -- REMOVE?

    v[0] = m_R.parseEval(numGames);
    m_numGames = v[0];  //Store the number of games

    v[1] = m_R.parseEval(averagePrice);
    m_avgPrice = v[1]; //Store average price

    v[2] = m_R.parseEval(maxPrice);
    m_maxPrice = v[2]; //Store max price

    v[3] = m_R.parseEval(avgMetascore);
    m_avgMetascore = v[3];

    v[4] = m_R.parseEval(totalPlayTime);
    m_totalPlaytime = v[4];
}

int SteamGameStats::getNumGames() const
{
    return m_numGames;
}

double SteamGameStats::getAvgPrice() const
{
    return m_avgPrice;
}

double SteamGameStats::getMaxPrice() const
{
    return m_maxPrice;
}

double SteamGameStats::getAvgMetascore() const
{
    return m_avgMetascore;
}

double SteamGameStats::getTotalPlaytime() const
{
    return m_totalPlaytime;
}

std::string SteamGameStats::getPrice()
{
    //Use the variable SD, which is stored in R after reading the selected CSV file
    //Also returns Price as a variable in R (used to get mean, average, etc)

    //First, eleminate the '$' symbol, set 'Free" entries to zero, and then convert the price to numbers
    std::string priceFormatted = "SD2 <- SD; SD2$Price <- sub('$','',as.character(SD2$Price), fixed=TRUE);"
                       "SD2$Price <- sub('Free','0',as.character(SD2$Price), fixed=TRUE);"
                       "price <- as.numeric(SD2$Price); ";

    return priceFormatted;
}

std::string SteamGameStats::getSelectElementsOfSet(std::string column, bool isFirst)
{
    //First create a string that will select either the first or second element of each tuple
    std::string elementSelect;

    if(isFirst)
        elementSelect = "TRUE,FALSE";
    else
        elementSelect = "FALSE,TRUE";

    //Use the variable SD, which is stored in R after reading the selected CSV file
    //Now split the string and then ignore the first or second half of the tuple
    //Save the result as the column name
    std::string ownersFormatted = "SD3 <- SD; SD3 <- unlist(strsplit(as.character(SD3$" + column + "), ' '));"
                                  "SD3 <- SD3[c(" + elementSelect + ")];"
                                  + column + " <- gsub(',','',SD3,fixed=TRUE);" ;

    return ownersFormatted;
}

std::string SteamGameStats::getNumberOfHoursPlayed()
{
    //First split the time, in the format HH:MM, into two sets (hours and mins)
    //Then convert mins to hours and sum the total number of hours

    std::string numHoursPlayed =
    "splitPlayTimes <-unlist(strsplit(as.character(Playtime..Median.), ':'));"
    "splitPlayTimesHour <- splitPlayTimes[c(TRUE,FALSE)];"          //Get hours only
    "splitPlayTimesMin <- splitPlayTimes[c(FALSE,TRUE)];"
    "totalPlayTimesMin <- strptime(splitPlayTimesMin,'%M');"        //Get minutes only

    //Sum all hours, rounded to 2 decimal places
    "totalPlayTimes <- round((sum(totalPlayTimesMin$min)/60) + sum(as.numeric(splitPlayTimesHour)),2);" ;

    return numHoursPlayed;
}

void SteamGameStats::generateStatsAndPlot(int comboIndex)
{

    //Make sure that the selected year exists
    try
    {

        QString yearStr = yearCombo->itemText(comboIndex);
        int yearInt = yearCombo->itemText(comboIndex).toInt();

        //Sets the year, which is used when updating the display
        setSteamYearDataFile(yearInt);

        readFile("Data/" + yearStr + "_SteamStats.csv");   //Read file
        getStatsByYear();                                  //Get all stats

        estimationBox->setTitle("Game Stats for:    " + QString::number(m_year));  //Display the year in the title

        numGamesLabel->setText(QString::number(getNumGames()));
        avgPriceLabel->setText("$" + QString::number(getAvgPrice()));
        maxPriceLabel->setText("$" + QString::number(getMaxPrice()));
        avgMetaScoreLabel->setText(QString::number(getAvgMetascore()) + "%");
        totalPlaytimeLabel->setText(QString::number(getTotalPlaytime()) + " hours");

        plot();            //Plot the data    TODO: Add parameter for year?

    }
    catch (std::exception &e)
    {
        std::cout << "Selected year invalid. Exception: " << e.what() << std::endl;
    }
}

//Author: Eddelbuettel and Romain Francois - Copyright (C) 2011
void SteamGameStats::filterFile() {
    // cairoDevice creates richer SVG than Qt can display
    // but per Michaele Lawrence, a simple trick is to s/symbol/g/ which we do here
    QFile infile(m_tempfile);
    infile.open(QFile::ReadOnly);
    QFile outfile(m_svgfile);
    outfile.open(QFile::WriteOnly | QFile::Truncate);

    QTextStream in(&infile);
    QTextStream out(&outfile);
    QRegExp rx1("<symbol");
    QRegExp rx2("</symbol");
    while (!in.atEnd()) {
        QString line = in.readLine();
        line.replace(rx1, "<g"); // so '<symbol' becomes '<g ...'
        line.replace(rx2, "</g");// and '</symbol becomes '</g'
        out << line << "\n";
    }
    infile.close();
    outfile.close();
}
