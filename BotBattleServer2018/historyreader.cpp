#include "historyreader.h"
#include <iostream>


#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QStringList>

using namespace std;

HistoryReader::HistoryReader()
{

}

void HistoryReader::read(const QJsonArray &array)
{
    for (int i = 0; i < array.size(); i++) {
        read(array.at(i));
    }
}

void HistoryReader::read(const QJsonObject &obj)
{
//    cout << "Obj: " << obj.keys().join('|').toStdString() << endl;
    int id = obj.value("id").toInt();
    if (id == 0) {
        // must be food
        readFoodRecs(obj.value("food"));
    }
    else {
        // must be bot
        readBotRec(id, obj.value("rec"));
    }
}


void HistoryReader::read(const QJsonValue &value)
{
    if (value.isObject()) {
        read(value.toObject());
    }
    else if (value.isArray()) {
        read(value.toArray());
    }
    else {
        cout << "Unknown JSON Value in HistoryReader" << endl;
    }
}

void HistoryReader::readBotRec(int /*botId*/, const QJsonValue& /*data*/)
{
   // cout << "GOt BOt" << endl;
}

void HistoryReader::readFoodRecs(const QJsonValue& data)
{
    if (!data.isArray()) {
        cout << "Expected food history to be array!" << endl;
        return;
    }
    const QJsonArray& foodArray = data.toArray();
    for (int i = 0; i < foodArray.size(); i++) {
        readFoodRec(foodArray.at(i));
    }
}

void HistoryReader::readFoodRec(const QJsonValue& data)
{
    if (!data.isObject()) {
        cout << "Expected food history element to be object!" << endl;
        return;
    }
   // const QJsonObject& food = data.toObject();
    // TODO: in the middle of implementing this...
}

void HistoryReader::read(const std::string& json)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(json), &err);

    if (err.error != QJsonParseError::NoError)
    {
        cout << "Some kind of Json Parse Error\n";
        return;
    }

    if (doc.isArray()) {
        //cout << "got JSON Array" << endl;
        read(doc.array());
    }
    else if (doc.isObject()) {
        cout << "got JSON Object" << endl;
        read(doc.object());
    }
//    cout << "Update: \n";
//    cout << hist << endl;
}
