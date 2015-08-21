//
// A C++ GUI application that analyses and displays gaming statistics from Steam using R and Rinside
// Initializes the GUI and uses SVG for the graph
//
// Copyright (C) 2011  Dirk Eddelbuettel and Romain Francois
//
// Modified by: Francois Stelluti

#include "SteamGameStats.h"

SteamGameStats::SteamGameStats(RInside & R) : m_R(R)
{
    m_year = 2015;      //Default year
    m_numGames = 0;     //Initially zero games
    m_avgPrice = 0;     //and zero average price

    m_tempfile = QString::fromStdString(Rcpp::as<std::string>(m_R.parseEval("tfile <- tempfile()")));
    m_svgfile = QString::fromStdString(Rcpp::as<std::string>(m_R.parseEval("sfile <- tempfile()")));

    setupDisplay();
}

SteamGameStats::~SteamGameStats() {}

void SteamGameStats::setupDisplay(void)  {
    QWidget *window = new QWidget;
    window->setWindowTitle("Steam Stats with using Rinside");

//    QSpinBox *spinBox = new QSpinBox;
//    QSlider *slider = new QSlider(Qt::Horizontal);
//    spinBox->setRange(5, 200);
//    slider->setRange(5, 200);
//    QObject::connect(spinBox, SIGNAL(valueChanged(int)), slider, SLOT(setValue(int)));
//    QObject::connect(slider, SIGNAL(valueChanged(int)), spinBox, SLOT(setValue(int)));
//    spinBox->setValue(m_bw);
//    QObject::connect(spinBox, SIGNAL(valueChanged(int)), this, SLOT(getBandwidth(int)));

    readFile("Data/2015_SteamStats.csv");            //Read file
    getStatsByYear();                           //Get all stats

    //Labels for each statistic
    QLabel *numGamesLabel = new QLabel(QString::number(getNumGames()));
    QLabel *avgPriceLabel = new QLabel("$" + QString::number(getAvgPrice()));

    //QLineEdit *cmdEntry = new QLineEdit(m_cmd);
    //QObject::connect(cmdEntry,  SIGNAL(textEdited(QString)), this, SLOT(getRandomDataCmd(QString)));
    //QObject::connect(cmdEntry,  SIGNAL(editingFinished()), this, SLOT(runRandomDataCmd()));

    QGroupBox *yearBox = new QGroupBox("Steam sales by year");

    QComboBox *combo = new QComboBox;
    combo->addItem("2015",0);
    combo->addItem("2014",1);
    combo->addItem("2013",1);
    combo->addItem("2012",1);

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addWidget(combo);

    yearBox->setMinimumSize(260,140);
    yearBox->setMaximumSize(260,140);
    yearBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    yearBox->setLayout(vbox);

    QButtonGroup *kernelGroup = new QButtonGroup;
    //kernelGroup->addButton(radio1, 0);
    //kernelGroup->addButton(radio2, 1);
    //kernelGroup->addButton(radio3, 2);
    //kernelGroup->addButton(radio4, 3);
    //kernelGroup->addButton(radio5, 4);
    //QObject::connect(kernelGroup, SIGNAL(buttonClicked(int)), this, SLOT(getKernel(int)));

    //Need a method when selected other year -- testing commit to other branch
    //Assume it is 2015 for now

    m_svg = new QSvgWidget();
    //runRandomDataCmd();         // also calls plot()
    plot();

    QGroupBox *estimationBox = new QGroupBox("Game Stats");

    //QVBoxLayout *topright = new QVBoxLayout;
    QFormLayout *topright = new QFormLayout;
//    topright->addWidget(cmdLabel);
//    topright->addWidget(cmdLabel2);
//    topright->addWidget(cmdLabel3);
    topright->addRow(tr("&Number of Games:"), numGamesLabel);
    topright->addRow(tr("&Average Price:"), avgPriceLabel);

    estimationBox->setMinimumSize(360,140);
    estimationBox->setMaximumSize(360,140);
    estimationBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    estimationBox->setLayout(topright);

    QHBoxLayout *upperlayout = new QHBoxLayout; //Adds both Top containers
    upperlayout->addWidget(yearBox);
    upperlayout->addWidget(estimationBox);

    QHBoxLayout *lowerlayout = new QHBoxLayout;
    lowerlayout->addWidget(m_svg);

    QVBoxLayout *outer = new QVBoxLayout;
    outer->addLayout(upperlayout);
    outer->addLayout(lowerlayout);
    window->setLayout(outer);
    window->show();
}

void SteamGameStats::plot(void) {
    std::string cmd0 = "svg(width=6,height=6,pointsize=10,filename=tfile); ";
    std::string cmd1 = "plot(density(1,100)); ";
    std::string cmd2 = "dev.off()";
    //std::string cmd1 = "plot(density(y, bw=bw/100, graphType=graphType), xlim=range(y)+c(-2,2), main=\"Kernel: ";
    //std::string cmd2 = "\"); points(y, rep(0, length(y)), pch=16, col=rgb(0,0,0,1/4));  dev.off()";
    //std::string cmd = cmd0 + cmd1 + graphstrings[m_graphType] + cmd2; // stick the selected kernel in the middle
    std::string cmd = cmd0 + cmd1 + cmd2;

    m_R.parseEvalQ(cmd);
    filterFile();           	// we need to simplify the svg file for display by Qt
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
    std::string cmd = "SD <- read.csv(\"~/Desktop/Github/R_SteamStats/" + file.toStdString() + "\", header=TRUE)";    //Command to read the csv file
    m_R["SD"] = cmd;            //Store variable in R
    m_R.parseEvalQNT(cmd);
}

void SteamGameStats::getStatsByYear()
{
    //Set the year
    setSteamYearDataFile(m_year);

    //Switch? IF? Assume it's only 2015

    std::string cmd1 = "nrow(SD)";                   //Number of games (rows)
    std::string cmd2 = "SD2 <- SD; SD2$Price <- sub('$','',as.character(SD2$Price), fixed=TRUE);"
                       "SD2$Price <- sub('Free','0',as.character(SD2$Price), fixed=TRUE);"
                       "Price <- as.numeric(SD2$Price); avgPrice <- mean(Price); avgPrice <- round(avgPrice, digits=2)";

    Rcpp::NumericVector v = m_R.parseEval(cmd1);  //Store result
    m_numGames = v[0];

    Rcpp::NumericVector v2 = m_R.parseEval(cmd2);
    m_avgPrice = v2[0];
}

int SteamGameStats::getNumGames()
{
    return m_numGames;
}

double SteamGameStats::getAvgPrice()
{
    return m_avgPrice;
}

//void SteamGameStats::runRandomDataCmd(void) {
//    std::string cmd = "y2 <- " + m_cmd.toStdString() + "; y <- y2";
//    m_R.parseEvalQNT(cmd);
//    plot();                     // after each random draw, update plot with estimate
//}

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
