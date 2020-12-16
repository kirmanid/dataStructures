#ifndef HISTORYREADER_H
#define HISTORYREADER_H

#include <string>

class QJsonArray;
class QJsonObject;
class QJsonValue;

class HistoryReader
{
public:
    HistoryReader();
    void read(const std::string& json);
private:
    void read(const QJsonArray& array);
    void read(const QJsonObject& array);
    void read(const QJsonValue& array);
    void readBotRec(int botId, const QJsonValue& data);
    void readFoodRecs(const QJsonValue& data);
    void readFoodRec(const QJsonValue& data);
};

#endif // HISTORYREADER_H
