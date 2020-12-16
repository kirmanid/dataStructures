#include <algorithm>
#include "uniqueid.h"
#include <iostream>

int UniqueId::getId()
{
    if (releasedIds.empty()) {
        //std::cout << "Upped Cap: " << nextId+1 << std::endl;
        return nextId++;
    }
    int id = releasedIds.back();
    releasedIds.pop_back();
    return id;
}

void UniqueId::releaseId(int id)
{
    releasedIds.push_back(id);
    std::sort(releasedIds.begin(), releasedIds.end());
    while (releasedIds.back() == (nextId-1)) {
        releasedIds.pop_back();
        nextId--;
    }
}
