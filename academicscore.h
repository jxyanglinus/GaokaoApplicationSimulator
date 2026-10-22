#ifndef ACADEMICSCORE_H
#define ACADEMICSCORE_H

#include <array>
#include <algorithm>

enum class PrimarySubject { Physics, History }; // 首选科目
enum class SecondarySubject { Chemistry, Geography, Politics, Biology }; // 再选科目，按考试时间排序
enum class ForeignLanguage { English, Japanese, Russian, German, French, Spanish }; // 外语

class AcademicScore {
public:
    AcademicScore();
    AcademicScore(int ch, int ma, ForeignLanguage fl, int fo, PrimarySubject prim, int pr,
                  int chemi, int geo, int poli, int bio) : chinScore(ch), mathScore(ma),
        foreignLang(fl), foreignScore(fo), primSubject(prim), primScore(pr) {
        // 再选科目初始化
        secScore[0] = chemi;
        secScore[1] = geo;
        secScore[2] = poli;
        secScore[3] = bio;

        // 再选科目掩位码
        secSubjectMask = 0;
        for (int i = 0; i <= 3; i++) {
            if (secScore[i] > 0) {
                secSubjectMask |= (1 << i);
            }
        }

        // 计算排序会用到的分数
        total = 0;
        total = chinScore + mathScore + foreignScore + primScore;
        for (const int &i : secScore) {
            total += i;
        }
        chinMathSum = chinScore + mathScore;
        chinMathMax = std::max(chinScore, mathScore);
        secondaryMax = -0x3f3f3f3f;
        secondaryMin = 0x3f3f3f3f;
        for (const int &i : secScore) {
            if (i) {
                secondaryMin = std::min(secondaryMin, i);
                secondaryMax = std::max(secondaryMax, i);
            }
        }
    }

    // getters
    int getSecondaryMask() const {
        return secSubjectMask;    // 获取再选科目掩位码
    }

    int getTotal() const {
        return total;
    }

    int getChinMathSum() const {
        return chinMathSum;
    }

    int getChinMathMax() const {
        return chinMathMax;
    }

    int getPrimScore() const {
        return primScore;
    }

    int getForeignScore() const {
        return foreignScore;
    }

    int getSecMax() const {
        return secondaryMax;
    }

    int getSecMin() const {
        return secondaryMin;
    }

private:
    // 排序会用到的分数
    int total;
    int chinMathSum;
    int chinMathMax;
    int secondaryMax;
    int secondaryMin;

    // 语数
    int chinScore;
    int mathScore;

    // 外语
    ForeignLanguage foreignLang;
    int foreignScore;

    // 首选科目
    PrimarySubject primSubject;
    int primScore;

    // 再选科目
    int secSubjectMask;
    std::array<int, 4> secScore;
};

#endif // ACADEMICSCORE_H
