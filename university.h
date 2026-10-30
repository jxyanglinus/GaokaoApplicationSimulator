#ifndef UNIVERSITY_H
#define UNIVERSITY_H

#include <string>
#include <utility>
#include <vector>

#include "majorgroup.h"

// 院校：若干院校专业组的集合
class University {
public:
    University(int _id, std::string _name) : id(_id), name(std::move(_name)) { }

    University(int _id, std::string _name, const std::vector<MajorGroup> &groups)
        : id(_id), name(std::move(_name)), groupList(groups) { }

    void addGroup(const MajorGroup &group) {
        if (hasGroup(group.getGroupId())) return;
        groupList.push_back(group);
    }

    bool hasGroup(int groupId) const {
        for (const MajorGroup &g : groupList) {
            if (g.getGroupId() == groupId) return true;
        }
        return false;
    }

    // 按组号查找专业组；投档引擎需要可写的组（投档会改变已投档人数）
    MajorGroup *findGroup(int groupId) {
        for (MajorGroup &g : groupList) {
            if (g.getGroupId() == groupId) return &g;
        }
        return nullptr;
    }

    const MajorGroup *findGroup(int groupId) const {
        for (const MajorGroup &g : groupList) {
            if (g.getGroupId() == groupId) return &g;
        }
        return nullptr;
    }

    // getters
    int getId() const {
        return id;
    }

    const std::string &getName() const {
        return name;
    }

    std::vector<MajorGroup> &getGroups() {
        return groupList;
    }

    const std::vector<MajorGroup> &getGroups() const {
        return groupList;
    }

private:
    int id;
    std::string name;
    std::vector<MajorGroup> groupList;
};

#endif // UNIVERSITY_H
