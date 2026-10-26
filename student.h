#ifndef STUDENT_H
#define STUDENT_H

#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "academicscore.h"
#include "preference.h"

// 本科批可填报的院校专业组志愿数上限（按“45 个院校专业组 × 每组 6 个专业”的省份口径）
inline constexpr int MAX_PREFERENCES = 45;

enum class AddGroupResult { OverLimit, NotExisted, Succeeded };

// 考生：成绩 + 志愿表
class Student {
public:
    Student(std::string _name, int _id, AcademicScore grades)
        : name(std::move(_name)), id(_id), score(grades) { }

    /*
     * 投档成绩降序比较。
     * 当总分相同时，依次比较以下项目，直至分出先后：
     *   1. 语文、数学两科成绩之和
     *   2. 语文或数学单科最高成绩
     *   3. 外语单科成绩
     *   4. 首选科目单科成绩（物理或历史）
     *   5. 再选科目单科最高成绩
     *   6. 再选科目单科次高成绩
     * 分数完全相同的考生在这里视为等价（两者都返回 false），由引擎决定是否同分同位次。
     * std::sort 默认按 operator< 升序排列，因此这里“大于”的语义会得到从高到低的结果。
     */
    bool operator<(const Student &that) const {
        return std::make_tuple(
                   score.getTotal(),        // 第 1 级：总分
                   score.getChinMathSum(),  // 第 2 级：语数之和
                   score.getChinMathMax(),  // 第 3 级：语数单科最高
                   score.getForeignScore(), // 第 4 级：外语
                   score.getPrimScore(),    // 第 5 级：首选科目
                   score.getSecMax(),       // 第 6 级：再选最高
                   score.getSecMin()        // 第 7 级：再选次高
               ) >
               std::make_tuple(
                   that.score.getTotal(),
                   that.score.getChinMathSum(),
                   that.score.getChinMathMax(),
                   that.score.getForeignScore(),
                   that.score.getPrimScore(),
                   that.score.getSecMax(),
                   that.score.getSecMin()
               );
    }

    // 追加一个院校专业组志愿。
    AddGroupResult addGroupById(int groupId) {
        if (preferList.size() >= MAX_PREFERENCES) {
            return AddGroupResult::OverLimit;
        }
        // TODO: 在院校专业组目录中查找 groupId，找不到时返回 NotExisted；
        //       找到后把对应专业组信息转成 Preference 追加到 preferList。
        //       目前没有目录数据，因此这里只能完成数量校验。
        (void)groupId;
        return AddGroupResult::Succeeded;
    }

    // getters
    const std::string &getName() const {
        return name;
    }

    int getId() const {
        return id;
    }

    const AcademicScore &getScore() const {
        return score;
    }

    const std::vector<Preference> &getPreferences() const {
        return preferList;
    }

private:
    std::string name;
    int id;
    AcademicScore score;
    std::vector<Preference> preferList;
};

#endif // STUDENT_H
