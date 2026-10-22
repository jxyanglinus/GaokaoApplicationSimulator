#ifndef PREFERENCE_H
#define PREFERENCE_H

#include <vector>
#include "university.h"
#include <array>

#define MAX_MAJORS 6

struct SelectedMajor {
    int id;
    std::string name;
};

enum class AddMajorResult { OverLimit, NotExisted, Succeeded }; // 专业数量过多，专业不存在，成功添加

class Preference {
public:
    Preference(int uId, std::string uName, int gId) : universityId(uId), universityName(uName), groupId(gId) {}

    Preference(int uId, std::string uName, int gId, std::vector<SelectedMajor> selectedMajors)
        : universityId(uId), universityName(uName), groupId(gId), majorList(selectedMajors) {}


    AddMajorResult addMajorById(int id) {
        if (majorList.size() >= MAX_MAJORS) {
            return AddMajorResult::OverLimit;
        }
        return AddMajorResult::Succeeded;
    }

    // setters
    void setUniversityById(int id) { }

    void setUniversityByName(std::string name) { }

    // getters
    int getUniversityId() const {
        return universityId;
    }

    std::string getUniversityName() const {
        return universityName;
    }

private:
    int universityId;
    std::string universityName;
    int groupId;
    std::vector<SelectedMajor> majorList;
};

#endif // PREFERENCE_H
