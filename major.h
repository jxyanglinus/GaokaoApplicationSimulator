#ifndef MAJOR_H
#define MAJOR_H

#include <string>
#include <utility>

// 专业：隶属于某个院校专业组，记录计划数与已录取人数
class Major {
public:
    Major(int _id, std::string _name, int _quota)
        : id(_id), name(std::move(_name)), quota(_quota) { }

    bool isFull() const {
        return enrolledCnt >= quota;
    }

    // 录取一人（投档引擎调用）
    void enroll() {
        ++enrolledCnt;
    }

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

    int getEnrolledCnt() const {
        return enrolledCnt;
    }

    bool operator==(const Major &that) const {
        return (id == that.id) && (name == that.name);
    }

private:
    int id;
    std::string name;
    int quota;
    int enrolledCnt = 0;
};

#endif // MAJOR_H
