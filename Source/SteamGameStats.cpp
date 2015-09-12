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
    corrCoefficientLabel = new QLabel("Coeff: ");    //The Correlation coefficient
    p_valueLabel = new QLabel("p-value: ");          //p-value for the correlation test
    correlationTestMessageLabel = new QLabel();
    correlationTestResultLabel = new QLabel();

    yearCombo = new QComboBox();
    plotVarComboX = new QComboBox();
    plotVarComboY = new QComboBox();
    estimationBox = new QGroupBox();

    correlationButton = new QPushButton("Correlation Test");
    plotButton = new QPushButton("Plot Data");

    //Initialize and display the GUI
    setupDisplay();
}

SteamGameStats::~SteamGameStats() {}

void SteamGameStats::setupDisplay(void)
{
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

    //Plot variable comboBoxes for the X and Y axis
    plotVarComboX->addItem("Price");
    plotVarComboX->addItem("Userscore");
    plotVarComboX->addItem("Owners");
    plotVarComboX->addItem("Playtime");
    plotVarComboX->setCurrentIndex(0);
    plotVarComboX->setFixedWidth(100);

    plotVarComboY->addItem("Price");
    plotVarComboY->addItem("Userscore");
    plotVarComboY->addItem("Owners");
    plotVarComboY->addItem("Playtime");
    plotVarComboY->setCurrentIndex(1);      //Make sure the starting index is different
    plotVarComboY->setFixedWidth(100);

    //Set properties of the plot button
    plotButton->setToolTip("Plot the data using corresponding variables");
    plotButton->setMaximumWidth(120);

    //Set properties of the correlation button
    correlationButton->setToolTip("Correlation Test using Pearson's' coefficient");
    correlationButton->setMaximumWidth(130);

    m_svg = new QSvgWidget();   //Initialize svg object

    generateStatsAndPlot(yearCombo->currentIndex());    //Generate initial plot

    //Connect comboBox to stats displayed
    QObject::connect(yearCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(generateStatsAndPlot(int)));

    //Connect Correlaton test button
    QObject::connect(correlationButton, SIGNAL(released()), this, SLOT(displayCorrelationTest()));

    //Connect Plot button
    QObject::connect(plotButton, SIGNAL(released()), this, SLOT(plotWithSelectedVariables()));

    //Use these layouts to display multiple labels in one line
    QHBoxLayout *correlationStatsLayout = new QHBoxLayout();
    correlationStatsLayout->addWidget(p_valueLabel);
    correlationStatsLayout->addWidget(corrCoefficientLabel);
    QHBoxLayout *correlationButtonLayout = new QHBoxLayout();
    correlationButtonLayout->addWidget(correlationButton);
    correlationButtonLayout->addWidget(correlationTestResultLabel);
    QHBoxLayout *plotVariableLayout = new QHBoxLayout();
    plotVariableLayout->addWidget(plotButton);
    plotVariableLayout->addWidget(plotVarComboX);
    plotVariableLayout->addWidget(plotVarComboY);

    //Add year selection and correlation test TODO: Add more?
    QFormLayout *topLeft = new QFormLayout();
    topLeft->addRow(tr("Select year:"), yearCombo);
    topLeft->addRow(correlationButtonLayout);
    topLeft->addRow(correlationStatsLayout);
    topLeft->addRow(plotVariableLayout);

    //Set properties of yearBox
    yearBox->setMinimumSize(320,170);
    yearBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    yearBox->setLayout(topLeft);

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

void SteamGameStats::plot(plotVariable x_axis, plotVariable y_axis)
{
    //Declare both variables as strings to be evaluated by R
    std::string xVariableStatement, yVariableStatement;

    //Also declare each variable name, to use within R
    std::string x_VariableName, y_VariableName;

    //Set up the SVG file
    std::string svgFile = "svg(filename=tfile,width=6,height=5,pointsize=10); ";

    //Get the data for the x_axis and y_axis
    getPlotData(xVariableStatement, x_VariableName, x_axis);
    getPlotData(yVariableStatement, y_VariableName, y_axis);

    //Construct the data to pass into the plot
    std::string data = "dataPlot <- data.frame(" + x_VariableName + ", " + y_VariableName + "); ";

    //Axes are constrained to eliminate outliers from view only
    //Best fit using a Local Polynomial Regression Model is used
    std::string plot = "print(ggplot(dataPlot, aes(x=" + x_VariableName + ","  + y_VariableName + ", colour=" + y_VariableName +")) + geom_point(shape=16) "
                       " + labs(title='" + x_VariableName + " vs " + y_VariableName + "', x='" + x_VariableName + "', y='" + y_VariableName + "') "
                       " + geom_smooth(method=loess,se=FALSE) "
                       " + scale_colour_gradientn(colours=rainbow(7),guide=FALSE) );" ;

    std::string dev = "dev.off();";     //End of SVG file

    //Run the correlation test, use names of variables that are used in R
    correlationTest(xVariableStatement, yVariableStatement, x_VariableName, y_VariableName);

    //Build command and execute in R
    std::string cmd = svgFile + xVariableStatement + yVariableStatement + data + plot + dev;

    m_R.parseEvalQ(cmd);        //Parse and execute the string from R
    filterFile();               //Simplify the svg file for display by Qt
    m_svg->load(m_svgfile);

}

void SteamGameStats::getPlotData(std::string &_VariableStatement, std::string &_VariableName, plotVariable &_axis)
{
    //Get both the statement to pass into R and the variable name (used in plotting the data) from the plotVariable enum
    switch (_axis)
    {
        case Price:
        {
            _VariableStatement = getPrice();
            _VariableName = "price";
            break;
        }
        case Userscore:
        {
            _VariableStatement = getSelectElementsOfSet("Userscore..Metascore.", true) + //Gets User score
                    "userscore <- gsub('\\\\(|\\\\)|\\\\%', '', Userscore..Metascore.);"   //Get rid of brackets and percent sign
                    "userscore <- as.numeric(userscore);";
            _VariableName = "userscore";
            break;
        }
        case Owners:
        {
            _VariableStatement = getSelectElementsOfSet("Owners", true) + "Owners <-as.numeric(Owners);";
            _VariableName = "Owners";
            break;
        }
        case Playtime:
        {
            //Consider number of hours only
            _VariableStatement = "splitPlayTimes <-unlist(strsplit(as.character(Playtime..Median.), ':'));"
                        "playtime <- splitPlayTimes[c(TRUE,FALSE)]; playtime <-as.numeric(playtime);";       //Get hours only
            _VariableName = "playtime";
            break;
        }
        default:
        {
            std::cout << "Error, invalid variable" << std::endl;
        }
    }
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

    //Store the results
    m_numGames = m_R.parseEval(numGames);
    m_avgPrice = m_R.parseEval(averagePrice);
    m_maxPrice = m_R.parseEval(maxPrice);
    m_avgMetascore = m_R.parseEval(avgMetascore);
    m_totalPlaytime = m_R.parseEval(totalPlayTime);

}

void SteamGameStats::plotWithSelectedVariables()
{
    //Clear the corrletion test labels
    correlationTestResultLabel->setText("");
    corrCoefficientLabel->setText("Coeff: ");
    p_valueLabel->setText("p-value: ");

    //Get enum values from variable comboBoxes
    plotVariable varX = static_cast<plotVariable>(plotVarComboX->currentIndex());
    plotVariable varY = static_cast<plotVariable>(plotVarComboY->currentIndex());

    plot(varX, varY);     //Plot the data
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

double SteamGameStats::getPValue() const
{
    return m_p_value;
}

double SteamGameStats::getCorrCoefficiant() const
{
    return m_corrCoeff;
}

void SteamGameStats::correlationTest(std::string variable1, std::string variable2, std::string name1, std::string name2) {

    //Compute the Pearson product-moment correlation coefficient to see if there is a positive or negative,
    //statistically significant correlation. Assuming an alpha of 0.05 to compare with the p-value

    //First, evaluate the variables in R
    m_R.parseEvalQ(variable1);
    m_R.parseEvalQ(variable2);

    //Use the variable names in subsequent commands. First preform the correlation test
    std::string correlationTest = "corrTest <-cor.test(" + name1 + "," + name2 + ", use='complete.obs'); ";

    //Extract the p-value and coefficient and store the results
    std::string p_value = "pVal <-corrTest$p.value; pVal <-round(pVal,digits=4);";
    std::string corrCoeff = "corrEst <-corrTest$estimate; corrEst <-round(corrEst,digits=4)";

    m_p_value = m_R.parseEval(correlationTest + p_value);
    m_corrCoeff = m_R.parseEval(correlationTest + corrCoeff);

}

void SteamGameStats::displayCorrelationTest(void)
{
    //Store the p_value and correlation coefficient
    double pValue = getPValue();
    double corrCoeff = getCorrCoefficiant();

    //Display a message depending on the p_value and correlation coefficient
    //First, if the p-value is too high, results are not statistically significant (use alpha of 0.05)
    if(pValue > 0.05)
        correlationTestResultLabel->setText("Not statistically sig.");
    //Consider the Pearson coefficient
    else {
        if(corrCoeff > 0.7)
            correlationTestResultLabel->setText("Strong positive correlation");
        else if(corrCoeff < -0.7)
            correlationTestResultLabel->setText("Strong negative correlation");
        else
            correlationTestResultLabel->setText("No sig. correlation");
    }

    //Display the p_value and correlation coefficient
    p_valueLabel->setText("p-value: " + QString::number(pValue));
    corrCoefficientLabel->setText("Coeff: " + QString::number(corrCoeff));
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
                                  + column + " <- gsub(',','',SD3,fixed=TRUE);";

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

    //Make sure that the selected year exists, or that another exception isn't thrown
    try
    {

        QString yearStr = yearCombo->itemText(comboIndex);
        int yearInt = yearCombo->itemText(comboIndex).toInt();

        //Clear the corrletion test labels
        correlationTestResultLabel->setText("");
        corrCoefficientLabel->setText("Coeff: ");
        p_valueLabel->setText("p-value: ");

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

        plotWithSelectedVariables();    //TODO off by one error?

    }
    catch (std::exception &e)
    {
        std::cout << "Exception: " << e.what() << std::endl;
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
