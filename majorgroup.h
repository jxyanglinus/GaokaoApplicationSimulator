#ifndef MAJORGROUP_H
#define MAJORGROUP_H

#include "academicscore.h"
#include "major.h"

class MajorGroup {
public:
    MajorGroup(int groupId, int unvstId, PrimarySubject prim, int secMask = 0, const std::vector<ForeignLanguage> &fl = { })
        : id(groupId), universityId(unvstId), requiredPrimary(prim), requiredSecondaryMask(secMask), permittedLang(fl), enrolledCnt(0) {
    }

    void setLanguage(const std::vector<ForeignLanguage> &fl = std::vector<ForeignLanguage>()) {
        permittedLang = fl;
    }

    void addLanguage(ForeignLanguage fl) {
        permittedLang.push_back(fl);
    }

    void removeLanguage(ForeignLanguage fl) {
        int len = permittedLang.size();
        for (int i = 0; i < len; i++) {
            if (permittedLang[i] == fl) {
                permittedLang.erase(permittedLang.begin() + i);
                break;
            }
        }
    }

    void updateQuotaEnroll() {
        quota = 0, enrolledCnt = 0;
        for (const Major &i : majorList) {
            quota += i.getQuota();
            enrolledCnt += i.getEnrolledCnt();
        }
    }

    bool isMajorExisting(const Major &major) {
        for (const Major &i : majorList) {
            if (i == major) return true;
        }
        return false;
    }

    void addMajor(Major major) {
        if (isMajorExisting(major)) return;
        majorList.push_back(major);
        updateQuotaEnroll();
    }

public:
    bool isLanguageAccepted(ForeignLanguage lang) const {
        if (permittedLang.empty()) return true;
        for (const auto &i : permittedLang) {
            if (i == lang) return true;
        }
        return false;
    }

    // getters
    std::vector<ForeignLanguage> getLanguage() const {
        return permittedLang;
    }

    int getSecondaryMask() {
        return requiredSecondaryMask;
    }

    PrimarySubject getPrimary() {
        return requiredPrimary;
    }

    int getGroupId() {
        return id;
    }

    int getQuota() {
        int tot = 0;
        for (const Major &i : majorList) {
            tot += i.getQuota();
        }
        return tot;
    }

    int getEnrolledCnt() {
        int tot = 0;
        for (const Major &i : majorList) {
            tot += i.getEnrolledCnt();
        }
        return tot;
    }

private:
    int id;
    int universityId;
    PrimarySubject requiredPrimary;
    int requiredSecondaryMask;
    std::vector<ForeignLanguage> permittedLang;
    std::vector<Major> majorList;
    int quota;
    int enrolledCnt;
};

#endif // MAJORGROUP_H
