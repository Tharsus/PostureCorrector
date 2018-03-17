#include "databaseconnection.h"

DatabaseConnection::DatabaseConnection()
{
    db = QSqlDatabase::addDatabase("QODBC");
}

bool DatabaseConnection::openDatabase()
{
    QString serverName = "V14T\\SQLEXPRESS"; //"localhost\\SQLEXPRESS";
    QString dbName = "PostureCorrectorDatabase";

    QString dsn = QString("Driver={SQL SERVER};SERVER=%1;Database=%2;Trusted_Connection=Yes").arg(serverName).arg(dbName);

    db.setDatabaseName(dsn);
    if (!db.open()) {
        return false;
    }

    lastID = get_lastID();
    return true;
}

void DatabaseConnection::closeDatabase()
{
    db.close();
}

int DatabaseConnection::get_lastID()
{
    QSqlQuery query(db);
    query.exec("SELECT id FROM PostureStatus WHERE id = (SELECT MAX(id) FROM PostureStatus);");
    /*query.next();
    int lastId = query.value(0).toInt();*/

    int lastId = 0;
    if (query.first()) {
        lastId = query.value(0).toInt();
    }


    return lastId;
}

bool DatabaseConnection::insertIntoDatabase(int status)
{
    // Get current time and change for the DATETIME SQL format
    QDateTime currentTime = QDateTime::currentDateTime();

    // Prepare SQL insert statement
    QSqlQuery query(db);
    query.prepare("INSERT INTO PostureStatus (id, status, DateTime) "
                  "VALUES (?, ?, ?)");
    query.addBindValue(lastID + 1);
    query.addBindValue(status);
    query.addBindValue(currentTime);

    // Execute query
    if (query.exec()) {
        // Update reference to last id in the database if insert was successfully
        lastID = lastID + 1;
        return true;
    }
    return false;
}

bool DatabaseConnection::selectAllFromDatabase(std::vector<unsigned int> &status, std::vector<QDateTime> &dateTime)
{
    QSqlQuery query(db);
    if (!query.exec("SELECT * FROM PostureStatus")) {
        return false;
    }

    while (query.next()) {
        status.push_back(query.value(1).toUInt());
        dateTime.push_back(query.value(2).toDateTime());
    }

    return true;
}

bool DatabaseConnection::fillStatisticVariables()
{
    QSqlQuery query(db);
    if (!query.exec("SELECT * FROM PostureStatus")) {
        return false;
    }

    while (query.next()) {
        //status.push_back(query.value(1).toUInt());
        //dateTime.push_back(query.value(2).toDateTime());
    }

    return true;
}

