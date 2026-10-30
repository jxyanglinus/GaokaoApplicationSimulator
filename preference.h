#ifndef PREFERENCE_H
#define PREFERENCE_H

#include <string>
#include <utility>
#include <vector>

#include "university.h"

// 每个院校专业组内可填报的专业数上限
inline constexpr int MAX_MAJORS = 6;

struct SelectedMajor {
    int id;
    std::string name;
};

enum class AddMajorResult { OverLimit, NotExisted, Succeeded }; // 超过上限 / 专业不存在 / 添加成功

/*
 * 一个院校专业组志愿：投档单位是“院校专业组”，组内最多 6 个专业。
 *
 * 本程序只做投档，投档只判断整个专业组是否满额；
 * 组内专业列表仅作为志愿表数据保留，不参与投档（专业录取由高校负责）。
 */
class Preference {
public:
    Preference(int uId, std::string uName, int gId)
        : universityId(uId), universityName(std::move(uName)), groupId(gId) { }

    Preference(int uId, std::string uName, int gId, std::vector<SelectedMajor> selectedMajors)
        : universityId(uId), universityName(std::move(uName)), groupId(gId),
          majorList(std::move(selectedMajors)) { }

    AddMajorResult addMajorById(int majorId) {
        if (majorList.size() >= MAX_MAJORS) {
            return AddMajorResult::OverLimit;
        }
        // TODO: 在该专业组的专业目录中查找 majorId，找不到时返回 NotExisted；
        //       找到后把专业信息转成 SelectedMajor 追加到 majorList。
        //       目前没有专业目录数据，因此这里只能完成数量校验。
        (void)majorId;
        return AddMajorResult::Succeeded;
    }

    // 直接追加一个专业志愿（数据装载、测试用，不做目录校验）
    AddMajorResult addSelectedMajor(const SelectedMajor &major) {
        if (majorList.size() >= MAX_MAJORS) {
            return AddMajorResult::OverLimit;
        }
        majorList.push_back(major);
        return AddMajorResult::Succeeded;
    }

    // setters（TODO: 接入院校目录后改为按目录校验，避免填入不存在的院校）
    void setUniversityById(int id) {
        universityId = id;
    }

    void setUniversityByName(std::string name) {
        universityName = std::move(name);
    }

    // getters
    int getUniversityId() const {
        return universityId;
    }

    const std::string &getUniversityName() const {
        return universityName;
    }

    int getGroupId() const {
        return groupId;
    }

    const std::vector<SelectedMajor> &getMajors() const {
        return majorList;
    }

private:
    int universityId;
    std::string universityName;
    int groupId;
    std::vector<SelectedMajor> majorList;
};

#endif // PREFERENCE_H
