#ifndef DATABASECONNECTION_H
#define DATABASECONNECTION_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlError>
#include <QDateTime>

#include <iostream>
#include <ctime>

#include <vector>

#include <QDebug>

#include "list_of_states.h"

class DatabaseConnection
{
public:
    DatabaseConnection();
    bool openDatabase();
    void closeDatabase();

    bool insertIntoDatabase(int);
    bool selectAllFromDatabase(std::vector<unsigned int> &, std::vector<QDateTime> &);

    bool fillChartVariables();

private:
    QSqlDatabase db;
    int lastID;

    int get_lastID();

};

#endif // DATABASECONNECTION_H
