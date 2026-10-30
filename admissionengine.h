#ifndef ADMISSIONENGINE_H
#define ADMISSIONENGINE_H

#include <algorithm>
#include <cstddef>
#include <map>
#include <utility>
#include <vector>

#include "student.h"
#include "university.h"

/*
 * 投档引擎（AdmissionEngine）：本程序的核心，完成平行志愿投档。
 *
 * 输入：考生（成绩、志愿表）+ 院校（专业组、计划、选科/语种要求）。
 * 输出：
 *   - 每名考生的投档去向与逐志愿检索结果；
 *   - 每个院校专业组的投档线与末位投档者。
 *
 * 规则：
 *   1. 分数优先：按 7 级同分规则排出位次，位次靠前的先检索志愿；
 *   2. 依次检索志愿，先做组层面资格校验（首选科目、再选科目、外语语种）；
 *   3. 组计划未满（1:1 投档）即投出，本轮后续志愿不再检索；
 *   4. 全部志愿都投不出去即滑档。
 *
 * 投档线 = 该组最后一名（位次最低）投档考生的总分；该考生即“末位投档者”。
 * 专业录取、调剂、退档由高校负责，不在本引擎范围内。
 */

// 单个志愿的检索结果
enum class SlotStatus {
    Submitted,          // 投档成功，后续志愿不再检索
    GroupFull,          // 专业组计划已满
    GroupNotFound,      // 志愿中的院校或专业组不存在
    PrimaryMismatch,    // 首选科目不符
    SecondaryMismatch,  // 再选科目不符
    LanguageMismatch    // 外语语种不符
};

// 一名考生的投档结果；下标与传给 run() 的 students 一致（不受排序影响）
struct StudentSubmission {
    int studentId = 0;
    bool submitted = false;
    int toUniversityId = -1;
    int toGroupId = -1;
    int toPreferenceIndex = -1;             // 命中的志愿序号
    std::vector<SlotStatus> slotStatuses;   // 与志愿一一对应，命中后停止记录
};

// 一个院校专业组的投档结果
struct GroupCutoff {
    int universityId = -1;
    int groupId = -1;
    int quota = 0;
    int submittedCnt = 0;
    bool hasCutoff = false;    // 是否有人投档（没人投档时没有投档线）
    int lastStudentId = -1;    // 末位投档者（位次最低的进档考生）
    int lastTotal = -1;        // 投档线（末位投档者总分）
};

struct AdmissionReport {
    std::vector<StudentSubmission> students;   // 每名考生的投档结果
    std::vector<GroupCutoff> groups;           // 每个专业组的投档线与末位投档者
};

class AdmissionEngine {
public:
    AdmissionReport run(const std::vector<Student> &students,
                        std::vector<University> &universities) {
        AdmissionReport report;
        report.students.resize(students.size());
        for (std::size_t i = 0; i < students.size(); ++i) {
            report.students[i].studentId = students[i].getId();
        }

        // 分数优先：按位次从高到低排序，位次靠前的先检索
        std::vector<std::size_t> order(students.size());
        for (std::size_t i = 0; i < order.size(); ++i) {
            order[i] = i;
        }
        std::sort(order.begin(), order.end(),
                  [&students](std::size_t a, std::size_t b) { return students[a] < students[b]; });

        // 记录每个专业组最后一名投档考生（用于生成投档线）
        std::map<std::pair<int, int>, std::size_t> lastIndex;

        for (std::size_t index : order) {
            const Student &student = students[index];
            StudentSubmission &result = report.students[index];

            const std::vector<Preference> &prefs = student.getPreferences();
            for (std::size_t k = 0; k < prefs.size(); ++k) {
                const Preference &pref = prefs[k];

                University *university = findUniversity(universities, pref.getUniversityId());
                if (!university) {
                    result.slotStatuses.push_back(SlotStatus::GroupNotFound);
                    continue;
                }
                MajorGroup *group = university->findGroup(pref.getGroupId());
                if (!group) {
                    result.slotStatuses.push_back(SlotStatus::GroupNotFound);
                    continue;
                }

                const AcademicScore &score = student.getScore();
                if (group->getPrimary() != score.getPrimSubject()) {
                    result.slotStatuses.push_back(SlotStatus::PrimaryMismatch);
                    continue;
                }
                if (!group->isSecondaryAccepted(score.getSecondaryMask())) {
                    result.slotStatuses.push_back(SlotStatus::SecondaryMismatch);
                    continue;
                }
                if (!group->isLanguageAccepted(score.getForeignLang())) {
                    result.slotStatuses.push_back(SlotStatus::LanguageMismatch);
                    continue;
                }
                if (group->isFull()) {
                    result.slotStatuses.push_back(SlotStatus::GroupFull);
                    continue;
                }

                // 1:1 投档：组未满即投出，本批次不再检索后续志愿
                group->submit();
                lastIndex[std::make_pair(university->getId(), group->getGroupId())] = index;
                result.slotStatuses.push_back(SlotStatus::Submitted);
                result.submitted = true;
                result.toUniversityId = university->getId();
                result.toGroupId = group->getGroupId();
                result.toPreferenceIndex = static_cast<int>(k);
                break;
            }
        }

        // 生成各专业组的投档线
        for (University &university : universities) {
            for (MajorGroup &group : university.getGroups()) {
                GroupCutoff line;
                line.universityId = university.getId();
                line.groupId = group.getGroupId();
                line.quota = group.getQuota();
                line.submittedCnt = group.getSubmittedCnt();

                const auto it = lastIndex.find(std::make_pair(line.universityId, line.groupId));
                if (it != lastIndex.end()) {
                    const Student &last = students[it->second];
                    line.hasCutoff = true;
                    line.lastStudentId = last.getId();
                    line.lastTotal = last.getScore().getTotal();
                }
                report.groups.push_back(line);
            }
        }
        return report;
    }

private:
    static University *findUniversity(std::vector<University> &universities, int id) {
        for (University &u : universities) {
            if (u.getId() == id) return &u;
        }
        return nullptr;
    }
};

#endif // ADMISSIONENGINE_H
