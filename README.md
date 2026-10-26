# GaokaoApplicationSimulator · 高考志愿投档模拟器

用 C++17 模拟「3+1+2」新高考的志愿填报与平行志愿投档流程。当前是一个**纯控制台、无 Qt 依赖**的 qmake 工程，处于「数据模型已成型、投档引擎未实现」的骨架阶段。

## 当前状态

| 项目 | 状态 |
| --- | --- |
| 成绩模型（语数外 + 首选 + 再选） | ✅ 完成 |
| 同分排序规则（7 级比较） | ✅ 完成 |
| 院校 / 专业组 / 专业 / 志愿的数据结构 | 🟡 基本完成 |
| 选科、语种要求的校验 | 🟡 已实现判断函数，引擎尚未调用 |
| 平行志愿投档引擎 | ❌ 仅剩空壳声明 |
| 数据读取 / 结果输出 | ❌ 未开始 |
| 主流程 `main()` | ❌ 空实现（运行无任何输出） |
| 测试 | ❌ 无 |

## 构建与运行

- 工具链：C++17、qmake（`CONFIG -= qt`，不链接 Qt 库）；已在 Qt Creator 14 + Desktop Qt 6.5.3 MinGW 64-bit 下验证。
- 命令行构建：

  ```bat
  qmake GaokaoApplicationSimulator.pro
  mingw32-make -j
  debug\GaokaoApplicationSimulator.exe
  ```

- 源码为 UTF-8（含中文注释）；若改用 MSVC，需要额外加 `/utf-8`，否则中文注释与字符串可能报 C4819。

## 目录结构

```
GaokaoApplicationSimulator/
├── GaokaoApplicationSimulator.pro    # qmake 工程（目前只有 main.cpp 一个编译单元）
├── main.cpp                          # 入口，尚未接入流程
├── academicscore.h                   # 成绩：语数外 + 首选 + 再选，含排序用汇总分
├── student.h                         # 考生：成绩、排序、志愿表（上限 45 个专业组）
├── preference.h                      # 志愿：院校专业组 + 组内专业（上限 6 个）
├── university.h                      # 院校：专业组集合
├── majorgroup.h                      # 院校专业组：选科/语种要求 + 专业列表
├── major.h                           # 专业：计划数、已录取人数
├── submissionengine.h                # 投档引擎（占位）
├── README.md
└── debug/、Makefile*、.qmake.stash、*.pro.user   # 本地构建产物（已被 .gitignore 覆盖）
```

## 领域模型

```
AcademicScore ──┐
                ├── Student ──1..45── Preference ──1── University ──*── MajorGroup ──*── Major
  成绩/排序规则 ┘        （志愿表）      （院校专业组志愿）      （院校）      （专业组）      （专业）
                                                                        SubmissionEngine（待实现）
```

- **投档单位是「院校专业组」**：`Preference` 记录院校 + 专业组 + 组内最多 6 个专业。
- **成绩**：`AcademicScore` 保存语、数、外语（含语种）、首选科目（物理/历史）、再选两科（化学/地理/政治/生物），并用 4 位掩码 `secSubjectMask` 表示选了哪两科。
- **计划数**：`Major` 记录 `quota` 与 `enrolledCnt`，`MajorGroup` 的招生计划/已录取数由组内专业求和得到。

## 已实现的规则

1. **同分排序**（`Student::operator<`）：总分 → 语数之和 → 语数单科最高 → 外语 → 首选科目 → 再选最高 → 再选次高；配合 `std::sort` 得到投档成绩从高到低。
2. **选科要求**（`MajorGroup::isSecondaryAccepted`）：要求掩码为 0 表示不限；否则考生再选掩码需包含全部要求科目。
3. **语种要求**（`MajorGroup::isLanguageAccepted`）：`permittedLang` 为空表示不限。
4. **容量上限**：45 个院校专业组志愿、每组 6 个专业，超限返回 `OverLimit`。

## 未完成清单（按优先级）

1. **投档引擎**：`SubmissionEngine` 只有构造函数。需要实现「分数优先、遵循志愿、一轮投档」：按成绩降序遍历考生，逐个检索其志愿，跳过选科/语种不符或已满额的组，投出后组内按专业顺序录取（是否服从调剂待定），记录滑档原因。
2. **数据目录与校验**：`Student::addGroupById`、`Preference::addMajorById` 目前只校验数量，`id` 参数没有使用，`NotExisted` 分支永远走不到。需要一个院校/专业组/专业目录（先硬编码样例，再 CSV/JSON）。
3. **录取状态回写**：`Major::enroll()` 已提供，但没有引擎调用；投档后需要更新 `MajorGroup` 与院校层面的统计。
4. **输入输出**：没有读取招生计划、考生志愿的入口，也没有投档线/录取名单的输出。
5. **测试**：排序、同分、满额跳过、选科不符等用例都还没有。
6. **工程化**：未初始化 git（目录里只有 `.gitignore` / `.gitattributes`，没有 `.git`）；构建产物散落在工程根目录；头文件与源码同层。

## 本次整理

- 补齐头文件的自包含依赖（`<string>` / `<vector>` / `<tuple>` / `<utility>`），不再依赖间接包含。
- 删除只有声明、没有定义的默认构造函数（`AcademicScore()`、`Student()`），避免一旦使用就链接失败。
- 清理 `MajorGroup` 中与 `getQuota()` / `getEnrolledCnt()` 重复且从不被读取的 `quota`、`enrolledCnt` 成员及 `updateQuotaEnroll()`，招生计划统一由组内专业求和。
- 常量宏 `MAX_PREFERENCES` / `MAX_MAJORS` 改为 `inline constexpr`，保留原名字，调用处无需改动。
- 为 `MajorGroup` 补 `isFull()`、`isSecondaryAccepted()`、`getUniversityId()`、`getMajors()`，getter 统一加 `const`。
- 为 `University` 补 getter、`addGroup()`、`hasGroup()`（原来没有任何访问器）。
- 为 `Preference` 补 `getGroupId()`、`getMajors()`，并让两个 `setUniversity*` 真正赋值而不是空实现。
- 为 `Student` 补 getter；`SubmissionEngine` 构造函数改为 `= default` 并补上投档流程说明。
- 重写 README；在未实现处留下 `TODO` 注释，标明缺口而不是静默返回成功。

## 已知问题与建议

- `AddGroupResult` 与 `AddMajorResult` 是两个内容相同的枚举，可合并为一个 `AddResult`。
- `addGroupById` / `addMajorById` 的「成功」目前是假成功：只加了数量校验，没有真正写入条目。接入目录后应改为先查再插。
- `AcademicScore` 使用 `secScore[i] > 0` 判断是否选考，依赖「0 分不作为有效成绩」这一前提；再选科目赋分起点为 30 分，目前成立，但建议改成显式的选考标记。
- 排序注释里的「相同成绩返回 false」意味着完全同分的考生顺序取决于输入顺序，建议显式定义同分同位次的处理。
- 建议把代码移到 `src/` + `include/`，把引擎从 header-only 拆成 `.h/.cpp`，并在 `.pro` 中同步。

## 下一步路线图

1. 用硬编码数据跑通一次完整投档（3 所院校 × 若干专业组 × 5 名考生）。
2. 接入 CSV 读取与结果输出，形成可演示的命令行程序。
3. 补单元测试（排序 / 同分 / 满额 / 选科不符）。
4. 再考虑调剂、征集志愿、批次与投档线统计等扩展。
