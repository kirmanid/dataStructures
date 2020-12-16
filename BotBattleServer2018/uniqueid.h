#ifndef UNIQUEID_H
#define UNIQUEID_H

#include <vector>

class UniqueId
{
    std::vector<int> releasedIds;
    int nextId {1};
public:
    int getId();
    void releaseId(int id);
};


#endif // UNIQUEID_H
