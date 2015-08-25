//
// A C++ GUI application that analyses and displays gaming statistics from Steam using R and Rinside
// Initializes the GUI and uses SVG for the graph
//
// Copyright (C) 2011  Dirk Eddelbuettel and Romain Francois
//
// Modified by: Francois Stelluti

#include "SteamGameStats.h"

//Constructor sets to default year to 2015, the number of games considered and average price to zero.
SteamGameStats::SteamGameStats(RInside & R) : m_R(R), m_year(2015), m_numGames(0), m_avgPrice(0.0)
{
    //Set up temp files used for plots
    m_tempfile = QString::fromStdString(Rcpp::as<std::string>(m_R.parseEval("tfile <- tempfile()")));
    m_svgfile = QString::fromStdString(Rcpp::as<std::string>(m_R.parseEval("sfile <- tempfile()")));

    m_R.parseEvalQ("library(ggplot2)");    //Load the ggplot2 library

    //Initialize and display the GUI
    setupDisplay();
}

SteamGameStats::~SteamGameStats() {}

void SteamGameStats::setupDisplay(void)  {
    //Window name
    QWidget *window = new QWidget;
    window->setWindowTitle("Steam Stats with using Rinside");

    readFile("Data/2015_SteamStats.csv");   //Read file -- Change when using multiple files
    getStatsByYear();                       //Get all stats

    //Labels for each statistic
    QLabel *numGamesLabel = new QLabel(QString::number(getNumGames()));
    QLabel *avgPriceLabel = new QLabel("$" + QString::number(getAvgPrice()));
    QLabel *maxPriceLabel = new QLabel("$" + QString::number(getMaxPrice()));

    //Sales by year component
    QGroupBox *yearBox = new QGroupBox("Steam sales by timeframe");

    //Year selection comboBox
    QComboBox *yearCombo = new QComboBox;
    yearCombo->addItem("2015",0);
    yearCombo->addItem("2014",1);
    yearCombo->addItem("2013",1);
    yearCombo->addItem("2012",1);

    yearCombo->setFixedWidth(75);

    //Add comboBox to 'Steam sales by year'
    QFormLayout *topLeft = new QFormLayout;
    topLeft->addRow(tr("Select year:"), yearCombo);
    //topLeft->addWidget(combo);
    //topLeft->setAlignment(combo,Qt::AlignLeft);

    //Set properties of yearBox
    yearBox->setMinimumSize(320,170);
    yearBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    yearBox->setLayout(topLeft);

    QButtonGroup *kernelGroup = new QButtonGroup;
    //kernelGroup->addButton(radio1, 0);
    //kernelGroup->addButton(radio2, 1);
    //kernelGroup->addButton(radio3, 2);
    //kernelGroup->addButton(radio4, 3);
    //kernelGroup->addButton(radio5, 4);
    //QObject::connect(kernelGroup, SIGNAL(buttonClicked(int)), this, SLOT(getKernel(int)));

    //Need a method when selected other year
    //Assume it is 2015 for now

    m_svg = new QSvgWidget();
    plot();

    //Game statistics component
    QGroupBox *estimationBox = new QGroupBox("Game Stats");

    //Add each statistic to 'Game Stats'
    QFormLayout *topRight = new QFormLayout;
    topRight->addRow(tr("Number of Games:"), numGamesLabel);
    topRight->addRow(tr("Average Price:"), avgPriceLabel);
    topRight->addRow(tr("Maximum Price:"), maxPriceLabel);

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

//Make the plot variable
void SteamGameStats::plot(void) {

    //Currently, only a plot of the number of owners vs the price of a game

    std::string svgFile = "svg(width=6,height=5,pointsize=10,filename=tfile); ";
    std::string price = getPrice();
    std::string owners = getAvgOwners();
    std::string data = "dataPriceOwners <- data.frame(price, avgNumOwners); ";
    std::string plot = "print(ggplot(dataPriceOwners, aes(x=price, y=avgNumOwners, colour=avgNumOwners)) + geom_point(shape=16) "
                       " + labs(title='Price vs Average number of Owners', x='Price', y='AvgNumOwners') "
                       " + geom_smooth(method=lm,se=FALSE) + scale_colour_gradientn(colours=c('#FF3300', '#3366FF', '#00CCFF')) ); " ;
    std::string dev = "dev.off()";

    //Build command and execute in R
    std::string cmd = svgFile + price + owners + data + plot + dev;

    m_R.parseEvalQ(cmd);        //Parse and execute the string from R
    filterFile();           	//Simplify the svg file for display by Qt
    m_svg->load(m_svgfile);
}

//CHANGE??
//Use: df <- read.csv("2015_SteamStats.csv", header=TRUE) P <- df$Price ,etc to get stats

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
    //Set the year
    setSteamYearDataFile(m_year);

    //Switch? IF? Assume it's only 2015 for now. Use vector to store different results?

    std::string numGames = "nrow(SD)";    //Number of games (rows)
    std::string averagePrice = getPrice() + "avgPrice <- mean(price); avgPrice <- round(avgPrice, digits=2)";
    std::string maxPrice = getPrice() + "maxPrice <- max(price); maxPrice <- round(maxPrice, digits=2)";

    Rcpp::NumericVector v;  //Store result as a vector -- REMOVE?

    v[0] = m_R.parseEval(numGames);
    m_numGames = v[0];  //First index of vector is the number of games

    v[1] = m_R.parseEval(averagePrice);
    m_avgPrice = v[1]; //Second index of vector is the average price

    v[2] = m_R.parseEval(maxPrice);
    m_maxPrice = v[2];
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

std::string SteamGameStats::getAvgOwners()
{
    //Use the variable SD, which is stored in R after reading the selected CSV file
    //Only considers the average number of owners (ignores + or - number of owners)

    //First split the string and then ignore the + or - (ex: 12,000 +-334 -> 12,000)
    std::string ownersFormatted = "SD3 <- SD; SD3 <- unlist(strsplit(as.character(SD3$Owners), ' '));"
                                  "SD3 <- SD3[c(TRUE,FALSE)];"
                                  "SD3 <- gsub(',','',SD3,fixed=TRUE);"
                                  "avgNumOwners <- as.numeric(SD3); ";

    return ownersFormatted;
}

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
