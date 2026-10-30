#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX   // 避免 windows.h 的 min/max 宏破坏 std::max / std::min
#endif
#include <windows.h>
#endif

#include "fileio.h"

using namespace std;

static const char *slotName(SlotStatus status) {
    switch (status) {
    case SlotStatus::Submitted:
        return "投档成功";
    case SlotStatus::GroupFull:
        return "专业组计划已满";
    case SlotStatus::GroupNotFound:
        return "院校或专业组不存在";
    case SlotStatus::PrimaryMismatch:
        return "首选科目不符";
    case SlotStatus::SecondaryMismatch:
        return "再选科目不符";
    case SlotStatus::LanguageMismatch:
        return "外语语种不符";
    }
    return "未知";
}

static string universityName(const vector<University> &universities, int id) {
    for (const University &u : universities) {
        if (u.getId() == id) return u.getName();
    }
    return "?";
}

/*
 * 用法：GaokaoApplicationSimulator [院校文件] [考生文件] [输出目录]
 *   默认：data/universities.csv、data/students.csv、output/
 */
int main(int argc, char *argv[]) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    const string universityPath = argc > 1 ? argv[1] : "data/universities.csv";
    const string studentPath = argc > 2 ? argv[2] : "data/students.csv";
    const string outputDir = argc > 3 ? argv[3] : "output";

    string error;
    vector<University> universities;
    if (!fileio::loadUniversities(universityPath, universities, error)) {
        cerr << error << "\n";
        return 1;
    }

    vector<Student> students;
    if (!fileio::loadStudents(studentPath, universities, students, error)) {
        cerr << error << "\n";
        return 1;
    }
    if (students.empty()) {
        cerr << "考生文件为空：" << studentPath << "\n";
        return 1;
    }

    AdmissionEngine engine;
    const AdmissionReport report = engine.run(students, universities);

    for (size_t i = 0; i < students.size(); ++i) {
        const Student &student = students[i];
        const StudentSubmission &result = report.students[i];

        cout << student.getName() << "（" << student.getId() << "，"
             << student.getScore().getTotal() << " 分）\n";

        for (size_t k = 0; k < result.slotStatuses.size(); ++k) {
            cout << "    志愿 " << (k + 1) << "：" << slotName(result.slotStatuses[k]);
            if (result.slotStatuses[k] == SlotStatus::Submitted) {
                cout << "（" << universityName(universities, result.toUniversityId)
                     << " 专业组 " << result.toGroupId << "）";
            }
            cout << "\n";
        }

        if (!result.submitted) {
            cout << "    结果：滑档\n";
        }
    }

    cout << "\n各组投档线与末位投档者：\n";
    for (const GroupCutoff &line : report.groups) {
        cout << "  " << universityName(universities, line.universityId)
             << " 专业组 " << line.groupId
             << "：投档 " << line.submittedCnt << " / 计划 " << line.quota;

        if (!line.hasCutoff) {
            cout << "（无人投档）\n";
            continue;
        }

        cout << "，投档线 " << line.lastTotal
             << (line.submittedCnt >= line.quota ? "（已投满）" : "（未投满）") << "\n";

        const Student *last = fileio::findStudentById(students, line.lastStudentId);
        if (!last) continue;
        const AcademicScore &score = last->getScore();

        cout << "      末位投档者：" << last->getName() << "（" << last->getId() << "）"
             << "语文 " << score.getChineseScore()
             << "｜数学 " << score.getMathScore()
             << "｜" << fileio::languageName(score.getForeignLang()) << " "
             << score.getForeignScore()
             << "｜" << fileio::primaryName(score.getPrimSubject()) << " " << score.getPrimScore();
        for (int k = 0; k < 4; ++k) {
            if (score.getSecondaryMask() & (1 << k)) {
                cout << "｜" << fileio::secondaryName(k) << " " << score.getSecondaryScore(k);
            }
        }
        cout << "\n";
    }

    if (!fileio::saveResults(outputDir, report, students, universities, error)) {
        cerr << error << "\n";
        return 1;
    }
    cout << "\n结果已写入 " << outputDir << "/student_results.csv 与 group_cutoffs.csv\n";

    return 0;
}
