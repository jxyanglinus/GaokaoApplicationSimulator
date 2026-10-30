#ifndef MAJORGROUP_H
#define MAJORGROUP_H

#include <vector>

#include "academicscore.h"
#include "major.h"

/*
 * 院校专业组：平行志愿的投档单位，带选科要求、语种要求，下设若干专业。
 *
 * 投档只看「组」：组计划（各专业计划之和）未满即可投档；
 * submittedCnt 记录已投档（进档）人数，是本类唯一的运行状态。
 * 档案进校后的专业录取、调剂由高校负责，不在本程序范围内。
 */
class MajorGroup {
public:
    MajorGroup(int groupId, int unvstId, PrimarySubject prim, int secMask = 0,
               const std::vector<ForeignLanguage> &fl = {})
        : id(groupId), universityId(unvstId), requiredPrimary(prim),
          requiredSecondaryMask(secMask), permittedLang(fl) { }

    // 语种要求：传空表示不限
    void setLanguage(const std::vector<ForeignLanguage> &fl = {}) {
        permittedLang = fl;
    }

    void addLanguage(ForeignLanguage fl) {
        permittedLang.push_back(fl);
    }

    void removeLanguage(ForeignLanguage fl) {
        for (auto it = permittedLang.begin(); it != permittedLang.end(); ++it) {
            if (*it == fl) {
                permittedLang.erase(it);
                break;
            }
        }
    }

    bool isMajorExisting(const Major &major) const {
        for (const Major &i : majorList) {
            if (i == major) return true;
        }
        return false;
    }

    void addMajor(const Major &major) {
        if (isMajorExisting(major)) return;
        majorList.push_back(major);
    }

    /*
     * 再选科目要求：requiredSecondaryMask 为 0 表示不限；
     * 否则考生再选科目掩码必须包含全部要求的科目（按位与后等于要求掩码）。
     * 例：要求“化学+生物”时掩码为 0b1001，考生必须同时选考化学和生物。
     */
    bool isSecondaryAccepted(int studentMask) const {
        return (studentMask & requiredSecondaryMask) == requiredSecondaryMask;
    }

    // 语种要求：permittedLang 为空表示不限
    bool isLanguageAccepted(ForeignLanguage lang) const {
        if (permittedLang.empty()) return true;
        for (const auto &i : permittedLang) {
            if (i == lang) return true;
        }
        return false;
    }

    // 投档容量判定：按「已投档人数」计算，而不是按已录取人数。
    // 组计划已满时不再接受投档，考生继续检索下一个志愿。
    bool isFull() const {
        return submittedCnt >= getQuota();
    }

    // 投档一人（投档引擎调用）。只登记进档，不代表已被某个专业录取。
    void submit() {
        ++submittedCnt;
    }

    int getRemainingQuota() const {
        const int rest = getQuota() - submittedCnt;
        return rest > 0 ? rest : 0;
    }

    // getters
    const std::vector<ForeignLanguage> &getLanguage() const {
        return permittedLang;
    }

    int getSecondaryMask() const {
        return requiredSecondaryMask;
    }

    PrimarySubject getPrimary() const {
        return requiredPrimary;
    }

    int getGroupId() const {
        return id;
    }

    int getUniversityId() const {
        return universityId;
    }

    // 按专业号查找（专业分配阶段使用；投档阶段不查专业）
    Major *findMajor(int majorId) {
        for (Major &m : majorList) {
            if (m.getId() == majorId) return &m;
        }
        return nullptr;
    }

    const Major *findMajor(int majorId) const {
        for (const Major &m : majorList) {
            if (m.getId() == majorId) return &m;
        }
        return nullptr;
    }

    std::vector<Major> &getMajors() {
        return majorList;
    }

    const std::vector<Major> &getMajors() const {
        return majorList;
    }

    int getQuota() const {
        int tot = 0;
        for (const Major &i : majorList) {
            tot += i.getQuota();
        }
        return tot;
    }

    int getSubmittedCnt() const {
        return submittedCnt;
    }

private:
    int id;
    int universityId;
    PrimarySubject requiredPrimary;
    int requiredSecondaryMask;
    std::vector<ForeignLanguage> permittedLang;
    std::vector<Major> majorList;
    int submittedCnt = 0;   // 已投档（进档）人数
};

#endif // MAJORGROUP_H
