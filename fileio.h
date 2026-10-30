#ifndef FILEIO_H
#define FILEIO_H

#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>
#include <vector>

#include "admissionengine.h"

/*
 * 文件读写：院校/专业组/专业与考生志愿从 CSV 读入，投档结果写回 CSV。
 *
 * 输入文件（UTF-8，允许 # 注释行，允许 BOM）：
 *   院校：院校号,院校名称,专业组,首选科目,再选要求,专业号,专业名称,计划数,语种要求
 *        再选要求：不限 / 化学 / 化学+生物 …（与枚举顺序一致：化学 地理 政治 生物）
 *        语种要求：不限 / 英语 / 英语;日语 …
 *   考生：考生号,姓名,语文,数学,外语语种,外语,首选科目,首选,化学,地理,政治,生物,志愿
 *        后四科填 0 表示未选考；志愿格式 院校号:专业组;院校号:专业组
 *
 * 输出文件（UTF-8 带 BOM，便于 Excel 打开）：
 *   student_results.csv：每名考生的投档去向
 *   group_cutoffs.csv ：每个专业组的投档线与末位投档者各科成绩
 */

namespace fileio {

// ---------- 基础工具 ----------

inline std::string trim(const std::string &text) {
    const char *ws = " \t\r\n";
    const std::size_t begin = text.find_first_not_of(ws);
    if (begin == std::string::npos) return std::string();
    const std::size_t end = text.find_last_not_of(ws);
    return text.substr(begin, end - begin + 1);
}

inline std::vector<std::string> split(const std::string &text, char sep) {
    std::vector<std::string> parts;
    std::string current;
    for (char ch : text) {
        if (ch == sep) {
            parts.push_back(current);
            current.clear();
        } else {
            current.push_back(ch);
        }
    }
    parts.push_back(current);
    return parts;
}

inline std::vector<std::string> splitTrimmed(const std::string &text, char sep) {
    std::vector<std::string> parts = split(text, sep);
    for (std::string &part : parts) {
        part = trim(part);
    }
    return parts;
}

inline bool parseInt(const std::string &text, int &value) {
    try {
        std::size_t used = 0;
        const int parsed = std::stoi(trim(text), &used);
        if (used != text.size()) return false;
        value = parsed;
        return true;
    } catch (...) {
        return false;
    }
}

// ---------- 科目名称 ----------

inline const char *primaryName(PrimarySubject subject) {
    return subject == PrimarySubject::Physics ? "物理" : "历史";
}

inline const char *languageName(ForeignLanguage language) {
    switch (language) {
    case ForeignLanguage::English:
        return "英语";
    case ForeignLanguage::Japanese:
        return "日语";
    case ForeignLanguage::Russian:
        return "俄语";
    case ForeignLanguage::German:
        return "德语";
    case ForeignLanguage::French:
        return "法语";
    case ForeignLanguage::Spanish:
        return "西班牙语";
    }
    return "?";
}

// index：0 化学 1 地理 2 政治 3 生物
inline const char *secondaryName(int index) {
    switch (index) {
    case 0:
        return "化学";
    case 1:
        return "地理";
    case 2:
        return "政治";
    case 3:
        return "生物";
    }
    return "?";
}

inline bool parsePrimary(const std::string &text, PrimarySubject &subject) {
    if (text == "物理") {
        subject = PrimarySubject::Physics;
        return true;
    }
    if (text == "历史") {
        subject = PrimarySubject::History;
        return true;
    }
    return false;
}

inline bool parseLanguage(const std::string &text, ForeignLanguage &language) {
    if (text == "英语") {
        language = ForeignLanguage::English;
    } else if (text == "日语") {
        language = ForeignLanguage::Japanese;
    } else if (text == "俄语") {
        language = ForeignLanguage::Russian;
    } else if (text == "德语") {
        language = ForeignLanguage::German;
    } else if (text == "法语") {
        language = ForeignLanguage::French;
    } else if (text == "西班牙语") {
        language = ForeignLanguage::Spanish;
    } else {
        return false;
    }
    return true;
}

inline bool parseSecondaryBit(const std::string &text, int &bit) {
    if (text == "化学") {
        bit = 0;
    } else if (text == "地理") {
        bit = 1;
    } else if (text == "政治") {
        bit = 2;
    } else if (text == "生物") {
        bit = 3;
    } else {
        return false;
    }
    return true;
}

// "不限" → 0；"化学+生物" → 0b1001
inline bool parseSecondaryMask(const std::string &text, int &mask, std::string &error) {
    mask = 0;
    if (text.empty() || text == "不限") return true;
    for (const std::string &part : split(text, '+')) {
        int bit = 0;
        if (!parseSecondaryBit(trim(part), bit)) {
            error = "无法识别的再选科目要求：" + part;
            return false;
        }
        mask |= (1 << bit);
    }
    return true;
}

// "不限" → 空列表；"英语;日语" → {English, Japanese}
inline bool parseLanguages(const std::string &text, std::vector<ForeignLanguage> &languages,
                           std::string &error) {
    languages.clear();
    if (text.empty() || text == "不限") return true;
    for (const std::string &part : split(text, ';')) {
        ForeignLanguage language;
        const std::string name = trim(part);
        if (!parseLanguage(name, language)) {
            error = "无法识别的语种要求：" + name;
            return false;
        }
        languages.push_back(language);
    }
    return true;
}

// ---------- 读取 ----------

inline University *findOrCreateUniversity(std::vector<University> &universities, int id,
                                          const std::string &name) {
    for (University &u : universities) {
        if (u.getId() == id) return &u;
    }
    universities.push_back(University(id, name));
    return &universities.back();
}

inline bool loadUniversities(const std::string &path, std::vector<University> &universities,
                             std::string &error) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        error = "无法打开院校文件：" + path;
        return false;
    }

    universities.clear();
    std::string line;
    int lineNo = 0;

    while (std::getline(in, line)) {
        ++lineNo;
        if (lineNo == 1 && line.rfind("\xEF\xBB\xBF", 0) == 0) {
            line.erase(0, 3);   // 去掉 BOM
        }
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;

        const std::vector<std::string> fields = splitTrimmed(line, ',');
        if (fields.size() < 9) {
            error = "院校文件第 " + std::to_string(lineNo) + " 行：列数不足（需要 9 列）";
            return false;
        }

        int universityId = 0;
        int groupId = 0;
        int majorId = 0;
        int quota = 0;
        if (!parseInt(fields[0], universityId)) {
            error = "院校文件第 " + std::to_string(lineNo) + " 行：院校号不是整数";
            return false;
        }
        if (!parseInt(fields[2], groupId)) {
            error = "院校文件第 " + std::to_string(lineNo) + " 行：专业组号不是整数";
            return false;
        }
        if (!parseInt(fields[5], majorId)) {
            error = "院校文件第 " + std::to_string(lineNo) + " 行：专业号不是整数";
            return false;
        }
        if (!parseInt(fields[7], quota)) {
            error = "院校文件第 " + std::to_string(lineNo) + " 行：计划数不是整数";
            return false;
        }

        PrimarySubject primary;
        if (!parsePrimary(fields[3], primary)) {
            error = "院校文件第 " + std::to_string(lineNo) + " 行：首选科目应为 物理/历史";
            return false;
        }

        int mask = 0;
        std::string detail;
        if (!parseSecondaryMask(fields[4], mask, detail)) {
            error = "院校文件第 " + std::to_string(lineNo) + " 行：" + detail;
            return false;
        }

        std::vector<ForeignLanguage> languages;
        if (!parseLanguages(fields[8], languages, detail)) {
            error = "院校文件第 " + std::to_string(lineNo) + " 行：" + detail;
            return false;
        }

        University *university = findOrCreateUniversity(universities, universityId, fields[1]);
        MajorGroup *group = university->findGroup(groupId);
        if (!group) {
            university->getGroups().push_back(
                MajorGroup(groupId, universityId, primary, mask, languages));
            group = university->findGroup(groupId);
        }
        group->addMajor(Major(majorId, fields[6], quota));
    }
    return true;
}

inline const University *findUniversity(const std::vector<University> &universities, int id) {
    for (const University &u : universities) {
        if (u.getId() == id) return &u;
    }
    return nullptr;
}

inline bool loadStudents(const std::string &path, const std::vector<University> &universities,
                         std::vector<Student> &students, std::string &error) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        error = "无法打开考生文件：" + path;
        return false;
    }

    students.clear();
    std::string line;
    int lineNo = 0;

    while (std::getline(in, line)) {
        ++lineNo;
        if (lineNo == 1 && line.rfind("\xEF\xBB\xBF", 0) == 0) {
            line.erase(0, 3);
        }
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;

        const std::vector<std::string> fields = splitTrimmed(line, ',');
        if (fields.size() < 12) {
            error = "考生文件第 " + std::to_string(lineNo) + " 行：列数不足（至少 12 列）";
            return false;
        }

        int studentId = 0;
        int chinese = 0;
        int math = 0;
        int foreignScore = 0;
        int primaryScore = 0;
        int chem = 0, geo = 0, poli = 0, bio = 0;
        if (!parseInt(fields[0], studentId) ||
            !parseInt(fields[2], chinese) ||
            !parseInt(fields[3], math) ||
            !parseInt(fields[5], foreignScore) ||
            !parseInt(fields[7], primaryScore) ||
            !parseInt(fields[8], chem) ||
            !parseInt(fields[9], geo) ||
            !parseInt(fields[10], poli) ||
            !parseInt(fields[11], bio)) {
            error = "考生文件第 " + std::to_string(lineNo) + " 行：成绩或编号不是整数";
            return false;
        }

        ForeignLanguage language;
        if (!parseLanguage(fields[4], language)) {
            error = "考生文件第 " + std::to_string(lineNo) + " 行：无法识别的外语语种 " + fields[4];
            return false;
        }

        PrimarySubject primary;
        if (!parsePrimary(fields[6], primary)) {
            error = "考生文件第 " + std::to_string(lineNo) + " 行：首选科目应为 物理/历史";
            return false;
        }

        Student student(fields[1], studentId,
                        AcademicScore(chinese, math, language, foreignScore, primary, primaryScore,
                                      chem, geo, poli, bio));

        if (fields.size() >= 13 && !fields[12].empty()) {
            for (const std::string &item : split(fields[12], ';')) {
                const std::vector<std::string> pair = splitTrimmed(item, ':');
                if (pair.size() != 2) {
                    error = "考生文件第 " + std::to_string(lineNo) + " 行：志愿格式应为 院校号:专业组";
                    return false;
                }
                int universityId = 0;
                int groupId = 0;
                if (!parseInt(pair[0], universityId) || !parseInt(pair[1], groupId)) {
                    error = "考生文件第 " + std::to_string(lineNo) + " 行：志愿编号不是整数";
                    return false;
                }
                const University *university = findUniversity(universities, universityId);
                student.addPreference(Preference(universityId,
                                                 university ? university->getName() : std::string(),
                                                 groupId));
            }
        }

        students.push_back(student);
    }
    return true;
}

// ---------- 写出 ----------

inline void writeRow(std::ofstream &out, const std::vector<std::string> &fields) {
    for (std::size_t i = 0; i < fields.size(); ++i) {
        if (i) out << ',';
        out << fields[i];
    }
    out << '\n';
}

inline std::string intToText(int value) {
    return std::to_string(value);
}

inline const Student *findStudentById(const std::vector<Student> &students, int id) {
    for (const Student &student : students) {
        if (student.getId() == id) return &student;
    }
    return nullptr;
}

inline bool saveResults(const std::string &directory, const AdmissionReport &report,
                        const std::vector<Student> &students,
                        const std::vector<University> &universities, std::string &error) {
    std::error_code ec;
    std::filesystem::create_directories(directory, ec);
    if (ec) {
        error = "无法创建输出目录：" + directory + "（" + ec.message() + "）";
        return false;
    }

    const std::string studentPath = directory + "/student_results.csv";
    const std::string groupPath = directory + "/group_cutoffs.csv";

    std::ofstream studentOut(studentPath, std::ios::binary);
    if (!studentOut) {
        error = "无法写入文件：" + studentPath;
        return false;
    }
    studentOut.write("\xEF\xBB\xBF", 3);
    writeRow(studentOut, {"考生号", "姓名", "总分", "是否投档", "院校号", "院校名称", "专业组"});

    for (std::size_t i = 0; i < students.size() && i < report.students.size(); ++i) {
        const Student &student = students[i];
        const StudentSubmission &result = report.students[i];
        std::vector<std::string> row{intToText(student.getId()), student.getName(),
                                     intToText(student.getScore().getTotal())};
        if (result.submitted) {
            const University *university = findUniversity(universities, result.toUniversityId);
            row.push_back("是");
            row.push_back(intToText(result.toUniversityId));
            row.push_back(university ? university->getName() : std::string());
            row.push_back(intToText(result.toGroupId));
        } else {
            row.push_back("否");
            row.push_back("");
            row.push_back("");
            row.push_back("");
        }
        writeRow(studentOut, row);
    }

    std::ofstream groupOut(groupPath, std::ios::binary);
    if (!groupOut) {
        error = "无法写入文件：" + groupPath;
        return false;
    }
    groupOut.write("\xEF\xBB\xBF", 3);
    writeRow(groupOut, {"院校号", "院校名称", "专业组", "计划数", "投档人数", "是否投满", "投档线",
                        "末位考生号", "末位姓名", "语文", "数学", "外语语种", "外语", "首选科目", "首选",
                        "再选一", "再选一分数", "再选二", "再选二分数"});

    for (const GroupCutoff &line : report.groups) {
        const University *university = findUniversity(universities, line.universityId);
        std::vector<std::string> row{intToText(line.universityId),
                                     university ? university->getName() : std::string(),
                                     intToText(line.groupId), intToText(line.quota),
                                     intToText(line.submittedCnt),
                                     line.submittedCnt >= line.quota ? "是" : "否"};
        if (!line.hasCutoff) {
            row.insert(row.end(), 13, std::string());
            writeRow(groupOut, row);
            continue;
        }

        row.push_back(intToText(line.lastTotal));
        row.push_back(intToText(line.lastStudentId));

        const Student *last = findStudentById(students, line.lastStudentId);
        if (!last) {
            row.insert(row.end(), 11, std::string());
            writeRow(groupOut, row);
            continue;
        }

        const AcademicScore &score = last->getScore();
        row.push_back(last->getName());
        row.push_back(intToText(score.getChineseScore()));
        row.push_back(intToText(score.getMathScore()));
        row.push_back(languageName(score.getForeignLang()));
        row.push_back(intToText(score.getForeignScore()));
        row.push_back(primaryName(score.getPrimSubject()));
        row.push_back(intToText(score.getPrimScore()));

        int printed = 0;
        for (int k = 0; k < 4 && printed < 2; ++k) {
            if (score.getSecondaryMask() & (1 << k)) {
                row.push_back(secondaryName(k));
                row.push_back(intToText(score.getSecondaryScore(k)));
                ++printed;
            }
        }
        while (printed < 2) {
            row.push_back("");
            row.push_back("");
            ++printed;
        }
        writeRow(groupOut, row);
    }

    return true;
}

} // namespace fileio

#endif // FILEIO_H
