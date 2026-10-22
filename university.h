#ifndef UNIVERSITY_H
#define UNIVERSITY_H

#include <string>
#include "majorgroup.h"

class MajorGroup;

class University {
public:
    University(int _id, std::string _name) : id(_id), name(_name) { }

    University(int _id, std::string _name, const std::vector<MajorGroup> &mjrgrp) : id(_id), name(_name), groupList(mjrgrp) { }

private:
    int id;
    std::string name;
    std::vector<MajorGroup> groupList;
};

#endif // UNIVERSITY_H
