#ifndef STUDENT_H
#define STUDENT_H

#include <string>
#include "academicscore.h"
#include "preference.h"

#define MAX_PREFERENCES 45

enum class AddGroupResult { OverLimit, NotExisted, Succeeded };

// 学生
class Student {
public:
    Student();
    Student(std::string _name, int _id, AcademicScore grades) : name(_name), id(_id), score(grades) { }

    bool operator < (const Student &that) const {
        /*
        * 重载排序，因为只会从大到小排，这里直接重载 <
        * 核心排序逻辑：当多名考生投档成绩（总分）相同时，会依次比较以下项目的成绩，直至分出先后：
        * 语文、数学两科成绩之和
        * 语文或数学单科最高成绩（即比较两科中分数更高的那一科）
        * 外语单科成绩
        * 首选科目单科成绩（物理或历史）
        * 再选科目单科最高成绩（在政治、地理、化学、生物中，取分数最高的那一科）
        * 再选科目单科次高成绩（在政治、地理、化学、生物中，取分数次高的那一科）
        */
        auto key1 = std::make_tuple(
                        score.getTotal(),       // 第1级
                        score.getChinMathSum(), // 第2级
                        score.getChinMathMax(), // 第3级
                        score.getForeignScore(),// 第4级
                        score.getPrimScore(),   // 第5级
                        score.getSecMax(),      // 第6级
                        score.getSecMin()       // 第7级
                    );
        auto key2 = std::make_tuple(
                        that.score.getTotal(),
                        that.score.getChinMathSum(),
                        that.score.getChinMathMax(),
                        that.score.getForeignScore(),
                        that.score.getPrimScore(),
                        that.score.getSecMax(),
                        that.score.getSecMin()
                    );
        return key1 > key2;
    }

    AddGroupResult addGroupById(int id) {
        if (preferList.size() >= MAX_PREFERENCES) {
            return AddGroupResult::OverLimit;
        }
        return AddGroupResult::Succeeded;
    }

private:
    std::string name;
    int id;
    AcademicScore score;
    std::vector<Preference> preferList;
};

#endif // STUDENT_H
