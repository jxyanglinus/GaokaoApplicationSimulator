#ifndef MAJOR_H
#define MAJOR_H

#include <string>
#include <utility>

/*
 * 专业：隶属于某个院校专业组，只记录招生计划数。
 * 本程序只做投档，不模拟专业录取，因此不保存录取人数。
 */
class Major {
public:
    Major(int _id, std::string _name, int _quota)
        : id(_id), name(std::move(_name)), quota(_quota) { }

    // getters
    int getId() const {
        return id;
    }

    const std::string &getName() const {
        return name;
    }

    int getQuota() const {
        return quota;
    }

    bool operator==(const Major &that) const {
        return (id == that.id) && (name == that.name);
    }

private:
    int id;
    std::string name;
    int quota;
};

#endif // MAJOR_H
