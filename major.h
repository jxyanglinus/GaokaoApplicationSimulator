#ifndef MAJOR_H
#define MAJOR_H

#include <string>

class Major {
public:
    Major(int _id, std::string _name, int _quota): id(_id), name(_name), quota(_quota), enrolledCnt(0) { }

    // getters
    int getId() const {
        return id;
    }

    std::string getName() const {
        return name;
    }

    int getQuota() const {
        return quota;
    }

    int getEnrolledCnt() const {
        return enrolledCnt;
    }

public:
    bool operator==(const Major &that) const {
        return (id == that.id) && (name == that.name);
    }

private:
    int id;
    std::string name;
    int quota;
    int enrolledCnt;
};

#endif // MAJOR_H
