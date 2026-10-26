#ifndef SUBMISSIONENGINE_H
#define SUBMISSIONENGINE_H

/*
 * 平行志愿投档引擎（尚未实现，目前只有声明）。
 *
 * 规划中的流程：
 *   1. 所有考生按投档成绩降序排序（见 Student::operator<）；
 *   2. 每个考生依次检索自己的 45 个院校专业组志愿；
 *   3. 跳过选科/语种不符或已满额的院校专业组；
 *   4. 命中第一个可投档的组即投出，再按组内专业志愿顺序录取，服从调剂则调剂；
 *   5. 全部志愿都无法投档时记录滑档原因。
 *
 * 当前缺口：数据来源（院校/专业组/计划数）、投档状态、录取结果输出。
 */
class SubmissionEngine {
public:
    SubmissionEngine() = default;
};

#endif // SUBMISSIONENGINE_H
