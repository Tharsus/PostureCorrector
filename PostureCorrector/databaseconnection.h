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

class DatabaseConnection
{
public:
    DatabaseConnection();
    bool openDatabase();
    void closeDatabase();

    bool insertIntoDatabase(int);
    bool selectAllFromDatabase(std::vector<unsigned int> &, std::vector<QDateTime> &);

private:
    QSqlDatabase db;
    int lastID;

    int get_lastID();

};

#endif // DATABASECONNECTION_H
