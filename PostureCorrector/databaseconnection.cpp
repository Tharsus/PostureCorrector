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

bool DatabaseConnection::fillChartVariables(std::vector<QDate> &days, std::vector< std::vector<int> > &alertsInEachState, std::vector< std::vector<int> > &durationInEachState)
{
    // Statistics variables
    /*std::vector<QDate> days;
    std::vector< std::vector<int> > alertsInEachState;
    std::vector< std::vector<int> > durationInEachState;*/

    // Auxiliar variables for computing statistics
    int state[2];
    QDateTime dateTime[2];
    // Initialize vector with [0, 0, 0, 0, 0]
    std::vector<int> timeInEachState(5, 0);
    std::vector<int> alerts(4, 0);

    // Auxiliar variables for looping through database
    int index = 1;
    bool complete = false;



    while (not(complete)) {
        // Prepare SQL insert statement
        QSqlQuery query(db);
        query.prepare("SELECT * "
                      "FROM PostureStatus "
                      " WHERE id >= ? and id <= ?; ");

        query.addBindValue(index);

        if (index+19 < lastID) {
            query.addBindValue(index+19);

        }
        else {
            query.addBindValue(lastID);
            complete = true;
        }


        if (!query.exec()) {
            return false;
        }

        if (index==1){
            if (query.next()) {
                // 0=id , 1=state , 2=dateTime
                state[1]=query.value(1).toInt();
                dateTime[1]=query.value(2).toDateTime();
                days.push_back(dateTime[1].date());
            }
        }
        while (query.next()) {
            /*qDebug() << "ID= " << query.value(0).toUInt();;
            qDebug() << "State= " << query.value(1).toUInt();
            qDebug() << "DateTime= " << query.value(2).toDateTime();*/

            /*qDebug() << "Date= " << query.value(2).toDate();

            state[0] = query.value(1).toInt();
            dateTime[0] = query.value(2).toDateTime();
            qDebug() << dateTime[0].date().day();

            days.push_back(dateTime[0].date());
            qDebug() << endl;*/

            // Store previous state and dateTime
            state[0]=state[1];
            dateTime[0]=dateTime[1];

            // Update with new values
            state[1]=query.value(1).toInt();
            dateTime[1]=query.value(2).toDateTime();

            if (dateTime[0].date() != dateTime[1].date()) {
                // Prepare new day
                days.push_back(dateTime[1].date());
                durationInEachState.push_back(timeInEachState);
                alertsInEachState.push_back(alerts);

                for (unsigned i=0; i<5; i++) {
                    timeInEachState.at(i)=0;
                }
                for (unsigned i=0; i<4; i++) {
                    alerts.at(i)=0;
                }
            }


            // Update statistics of each state
            if (state[1] != START) {
                if (state[0]==CORRECT_POSTURE || state[0]==START) {
                    timeInEachState.at(0) += (dateTime[1].toMSecsSinceEpoch()-dateTime[0].toMSecsSinceEpoch());
                }
                else if (state[0]==LOW_HEIGHT) {
                    timeInEachState.at(1) += (dateTime[1].toMSecsSinceEpoch()-dateTime[0].toMSecsSinceEpoch());
                    alerts.at(0) += 1;
                }
                else if (state[0]==TOO_CLOSE) {
                    timeInEachState.at(2) += (dateTime[1].toMSecsSinceEpoch()-dateTime[0].toMSecsSinceEpoch());
                    alerts.at(1) += 1;
                }
                else if (state[0]==ROLL_RIGHT) {
                    timeInEachState.at(3) += (dateTime[1].toMSecsSinceEpoch()-dateTime[0].toMSecsSinceEpoch());
                    alerts.at(2) += 1;
                }
                else if (state[0]==ROLL_LEFT) {
                    timeInEachState.at(4) += (dateTime[1].toMSecsSinceEpoch()-dateTime[0].toMSecsSinceEpoch());
                    alerts.at(3) += 1;
                }
            }

        }

        if (not(complete)) {
            index += 20;
        }
    }
    // Push the last list to vector
    durationInEachState.push_back(timeInEachState);
    alertsInEachState.push_back(alerts);

    /*for (unsigned i=0; i<durationInEachState.size(); i++) {
        qDebug() << endl << "indice: " << i;
        qDebug() << "Day: " << days[i];
        qDebug() << "0: " << durationInEachState[i][0];
        qDebug() << "1: " << durationInEachState[i][1] << " number: " << alertsInEachState[i][0];
        qDebug() << "2: " << durationInEachState[i][2] << " number: " << alertsInEachState[i][1];
        qDebug() << "3: " << durationInEachState[i][3] << " number: " << alertsInEachState[i][2];
        qDebug() << "4: " << durationInEachState[i][4] << " number: " << alertsInEachState[i][3];
    }*/

    return true;
}

