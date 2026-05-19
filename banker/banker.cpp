/*
 * 银行家算法模拟（死锁避免）
 * 运行环境：Visual Studio 2019/2022 (C++17)
 * 功能：手动/随机配置参数、安全性检查、资源请求处理、操作日志、动态演示、查看日志
 */

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <random>
#include <iomanip>
#include <fstream>
#include <chrono>
#include <sstream>
#include <cctype>

using namespace std;

// 全局日志流
class Logger {
private:
    ofstream logFile;
public:
    Logger() {
        logFile.open("banker_log.txt", ios::app);
        if (!logFile.is_open()) {
            cerr << "警告：无法打开日志文件，日志将只输出到控制台。" << endl;
        }
    }
    ~Logger() {
        if (logFile.is_open()) logFile.close();
    }
    void log(const string& msg) {
        auto now = chrono::system_clock::now();
        auto now_time = chrono::system_clock::to_time_t(now);
        struct tm now_tm;
        localtime_s(&now_tm, &now_time); // 安全版本，VS 专用
        stringstream ss;
        ss << put_time(&now_tm, "[%Y-%m-%d %H:%M:%S] ");
        string timestamp = ss.str();
        if (logFile.is_open()) {
            logFile << timestamp << msg << endl;
            logFile.flush();
        }
        cout << timestamp << msg << endl;
    }
};

// 银行家算法类
class BankerAlgorithm {
private:
    int processCount;           // 进程数
    int resourceCount;          // 资源种类数
    vector<vector<int>> Allocation;   // 已分配矩阵 (n x m)
    vector<vector<int>> Max;          // 最大需求矩阵 (n x m)
    vector<vector<int>> Need;         // 剩余需求矩阵 (n x m)
    vector<int> Available;            // 可用资源向量 (m)
    Logger logger;                    // 日志对象

    // 安全性算法：检测当前状态是否安全，并返回安全序列
    bool isSafeState(vector<int>& safeSequence) {
        vector<int> work(Available);
        vector<bool> finish(processCount, false);
        safeSequence.clear();

        while (safeSequence.size() < processCount) {
            bool found = false;
            for (int i = 0; i < processCount; ++i) {
                if (!finish[i]) {
                    bool canAlloc = true;
                    for (int j = 0; j < resourceCount; ++j) {
                        if (Need[i][j] > work[j]) {
                            canAlloc = false;
                            break;
                        }
                    }
                    if (canAlloc) {
                        for (int j = 0; j < resourceCount; ++j) {
                            work[j] += Allocation[i][j];
                        }
                        finish[i] = true;
                        safeSequence.push_back(i);
                        found = true;
                    }
                }
            }
            if (!found) break; // 没有可完成的进程，不安全
        }
        return (safeSequence.size() == processCount);
    }

    // 更新 Need 矩阵
    void updateNeed() {
        Need.assign(processCount, vector<int>(resourceCount, 0));
        for (int i = 0; i < processCount; ++i) {
            for (int j = 0; j < resourceCount; ++j) {
                Need[i][j] = Max[i][j] - Allocation[i][j];
                if (Need[i][j] < 0) {
                    logger.log("错误：Max[" + to_string(i) + "][" + to_string(j) + "] < Allocation，数据非法！");
                    exit(1);
                }
            }
        }
    }

    // 显示当前所有矩阵
    void displayState() {
        cout << "\n========== 当前系统状态 ==========" << endl;
        cout << "进程数: " << processCount << "  资源种类数: " << resourceCount << endl;
        cout << "可用资源 Available: ";
        for (int v : Available) cout << v << " ";
        cout << endl;

        cout << "\n矩阵    Allocation矩阵   Max矩阵   Need矩阵 " << endl;
        //cout << setw(10) << " " << endl;
        cout << "进程" << "    ";
        for (int j = 0; j < resourceCount; ++j)
        {
            cout << "R" << j << " ";
        }   
        cout << endl;
        for (int i = 0; i < processCount; ++i) {
            cout << "P" << i << "    ";
            for (int j = 0; j < resourceCount; ++j)
            {
                cout << setw(3) << Allocation[i][j] ;
            }
            cout << "     ";
            for (int j = 0; j < resourceCount; ++j)
            {
                cout << setw(3) << Max[i][j];
            }
            cout << "     ";
            for (int j = 0; j < resourceCount; ++j)
            {
                cout << setw(3) << Need[i][j];
            }
            cout << endl;
        }

        cout << "\nMax 矩阵 :" << endl;
        cout << setw(10) << " ";
        for (int j = 0; j < resourceCount; ++j) cout << "R" << j << " ";
        cout << endl;
        for (int i = 0; i < processCount; ++i) {
            cout << "P" << i << "      ";
            for (int j = 0; j < resourceCount; ++j) cout << Max[i][j] << "  ";
            cout << endl;
        }

        cout << "\nNeed 矩阵 :" << endl;
        cout << setw(10) << " ";
        for (int j = 0; j < resourceCount; ++j) cout << "R" << j << " ";
        cout << endl;
        for (int i = 0; i < processCount; ++i) {
            cout << "P" << i << "      ";
            for (int j = 0; j < resourceCount; ++j) cout << Need[i][j] << "  ";
            cout << endl;
        }
        cout << "==================================\n" << endl;
    }

public:
    //构造函数：初始化所有矩阵，并确保 Need 正确
    BankerAlgorithm(int p, int r, const vector<int>& avail,
        const vector<vector<int>>& alloc, const vector<vector<int>>& max)
        : processCount(p), resourceCount(r), Available(avail), Allocation(alloc), Max(max) {
        if (processCount <= 0 || resourceCount <= 0) {
            logger.log("错误：进程数或资源种类数必须为正数！");
            exit(1);
        }
        updateNeed();
        logger.log("系统初始化完成。");
    }

    // 手动输入配置
    static BankerAlgorithm* manualCreate() {
        int p, r;
        cout << "请输入进程数: ";
        cin >> p;
        cout << "请输入资源种类数: ";
        cin >> r;
        if (p <= 0 || r <= 0) throw runtime_error("进程数/资源种类数必须大于0");

        vector<int> total(r);
        cout << "请输入每种资源的总量 (共" << r << "个，空格分隔): ";
        for (int i = 0; i < r; ++i) {
            cin >> total[i];
            if (total[i] < 0) throw runtime_error("资源总量不能为负数");
        }

        vector<vector<int>> alloc(p, vector<int>(r, 0));
        vector<vector<int>> max(p, vector<int>(r, 0));

        cout << "请输入 Allocation 矩阵 (" << p << " x " << r << "):" << endl;
        for (int i = 0; i < p; ++i) {
            cout << "进程 P" << i << ": ";
            for (int j = 0; j < r; ++j) {
                cin >> alloc[i][j];
                if (alloc[i][j] < 0) throw runtime_error("分配资源不能为负数");
            }
        }

        cout << "请输入 Max 矩阵 (" << p << " x " << r << "):" << endl;
        for (int i = 0; i < p; ++i) {
            cout << "进程 P" << i << ": ";
            for (int j = 0; j < r; ++j) {
                cin >> max[i][j];
                if (max[i][j] < alloc[i][j]) {
                    throw runtime_error("Max 不能小于 Allocation，进程 " + to_string(i) + " 资源 " + to_string(j));
                }
            }
        }

        // 计算 Available = 总量 - sum(Allocation 列和)
        vector<int> avail(r, 0);
        for (int j = 0; j < r; ++j) {
            int sumAlloc = 0;
            for (int i = 0; i < p; ++i) sumAlloc += alloc[i][j];
            avail[j] = total[j] - sumAlloc;
            if (avail[j] < 0) throw runtime_error("可用资源为负数，请检查分配是否超过总量");
        }

        return new BankerAlgorithm(p, r, avail, alloc, max);
    }

    // 随机生成配置（改进版：确保每列分配和不超过总量，且 Max >= Allocation）
    static BankerAlgorithm* randomCreate() {
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> pDist(3, 6);      // 进程数 3~6
        uniform_int_distribution<> rDist(2, 4);      // 资源种类 2~4
        uniform_int_distribution<> totalDist(5, 15); // 每种资源总量 5~15

        int p = pDist(gen);
        int r = rDist(gen);
        vector<int> total(r);
        for (int j = 0; j < r; ++j) total[j] = totalDist(gen);

        vector<vector<int>> alloc(p, vector<int>(r, 0));
        vector<vector<int>> max(p, vector<int>(r, 0));

        // 1. 随机生成 Allocation，确保每列和不超过 total[j]
        for (int j = 0; j < r; ++j) {
            int remaining = total[j];
            // 为每个进程分配一个随机数，但不能让剩余变负
            for (int i = 0; i < p - 1; ++i) {
                int maxAlloc = remaining / (p - i); // 平均分配剩余资源
                if (maxAlloc < 0) maxAlloc = 0;
                uniform_int_distribution<> allocDist(0, maxAlloc);
                alloc[i][j] = allocDist(gen);
                remaining -= alloc[i][j];
                if (remaining < 0) remaining = 0;
            }
            alloc[p - 1][j] = remaining; // 最后一个进程获得余量
        }

        // 2. 随机生成 Max，满足 Max[i][j] >= alloc[i][j]，且 Max[i][j] 不超过 total[j] 的一定范围
        uniform_int_distribution<> extraDist(0, 5); // 额外需求 0~5
        for (int i = 0; i < p; ++i) {
            for (int j = 0; j < r; ++j) {
                int extra = extraDist(gen);
                max[i][j] = alloc[i][j] + extra;
                if (max[i][j] > total[j]) max[i][j] = total[j]; // 不超过资源总量
            }
        }

        // 3. 计算可用资源
        vector<int> avail(r, 0);
        for (int j = 0; j < r; ++j) {
            int sumAlloc = 0;
            for (int i = 0; i < p; ++i) sumAlloc += alloc[i][j];
            avail[j] = total[j] - sumAlloc;
            // 由于构造 Allocation 时保证和不超过 total，avail 非负
        }

        cout << "\n随机生成的参数如下：" << endl;
        cout << "进程数 = " << p << ", 资源种类 = " << r << endl;
        cout << "资源总量: ";
        for (int v : total) cout << v << " ";
        cout << endl;
        cout << "Allocation 矩阵:" << endl;
        for (auto& row : alloc) {
            for (int v : row) cout << v << " ";
            cout << endl;
        }
        cout << "Max 矩阵:" << endl;
        for (auto& row : max) {
            for (int v : row) cout << v << " ";
            cout << endl;
        }
        return new BankerAlgorithm(p, r, avail, alloc, max);
    }

    // 资源请求处理
    void requestResources(int pid, const vector<int>& request) {
        if (pid < 0 || pid >= processCount) {
            logger.log("错误：无效的进程号 " + to_string(pid));
            return;
        }
        logger.log("进程 P" + to_string(pid) + " 发出请求: " + vectorToString(request));

        // 合法性检查1: 请求 <= 需求
        for (int j = 0; j < resourceCount; ++j) {
            if (request[j] > Need[pid][j]) {
                logger.log("拒绝请求：请求超过该进程的剩余需求！");
                return;
            }
        }
        // 合法性检查2: 请求 <= 可用资源
        for (int j = 0; j < resourceCount; ++j) {
            if (request[j] > Available[j]) {
                logger.log("拒绝请求：请求超过系统当前可用资源，进程必须等待！");
                return;
            }
        }

        // 尝试分配
        vector<int> oldAvail = Available;
        vector<vector<int>> oldAlloc = Allocation;
        vector<vector<int>> oldNeed = Need;

        for (int j = 0; j < resourceCount; ++j) {
            Available[j] -= request[j];
            Allocation[pid][j] += request[j];
            Need[pid][j] -= request[j];
        }

        displayState(); // 显示试探分配后的状态

        vector<int> safeSeq;
        if (isSafeState(safeSeq)) {
            logger.log("分配成功，系统处于安全状态。安全序列: " + sequenceToString(safeSeq));
            logger.log("资源已分配给 P" + to_string(pid));
        }
        else {
            // 回滚
            Available = oldAvail;
            Allocation = oldAlloc;
            Need = oldNeed;
            logger.log("分配失败：若分配则系统进入不安全状态，已回滚。");
        }
    }

    // 主动释放资源（进程结束）
    void releaseResources(int pid) {
        if (pid < 0 || pid >= processCount) {
            logger.log("错误：无效的进程号 " + to_string(pid));
            return;
        }
        bool allZero = true;
        for (int j = 0; j < resourceCount; ++j) {
            if (Allocation[pid][j] != 0) {
                allZero = false;
                break;
            }
        }
        if (allZero) {
            logger.log("进程 P" + to_string(pid) + " 未占用任何资源，无需释放。");
            return;
        }

        // 释放资源
        for (int j = 0; j < resourceCount; ++j) {
            Available[j] += Allocation[pid][j];
            Allocation[pid][j] = 0;
            Need[pid][j] = Max[pid][j]; // 释放后需求恢复为 Max
        }
        logger.log("进程 P" + to_string(pid) + " 已释放所有资源。");
        updateNeed(); // 更新 Need（因为 Allocation 变了）
        displayState();

        vector<int> safeSeq;
        if (isSafeState(safeSeq)) {
            logger.log("释放后系统处于安全状态，安全序列: " + sequenceToString(safeSeq));
        }
        else {
            logger.log("警告：释放资源后系统竟然不安全？这通常不会发生，但已记录。");
        }
    }

    // 执行安全性算法并显示结果（不修改状态）
    void checkSafety() {
        vector<int> safeSeq;
        if (isSafeState(safeSeq)) {
            logger.log("当前系统处于安全状态，安全序列: " + sequenceToString(safeSeq));
        }
        else {
            logger.log("当前系统处于不安全状态！可能发生死锁。");
        }
        displayState();
    }

    // 查看操作日志
    void viewLog() {
        ifstream logFile("banker_log.txt");
        if (!logFile.is_open()) {
            cout << "无法打开日志文件，可能尚未生成或路径错误。" << endl;
            return;
        }
        cout << "\n========== 操作日志 ==========" << endl;
        string line;
        while (getline(logFile, line)) {
            cout << line << endl;
        }
        cout << "==============================\n" << endl;
        logFile.close();
    }

    string vectorToString(const vector<int>& vec) {
        stringstream ss;
        ss << "[";
        for (size_t i = 0; i < vec.size(); ++i) {
            if (i > 0) ss << ", ";
            ss << vec[i];
        }
        ss << "]";
        return ss.str();
    }

    string sequenceToString(const vector<int>& seq) {
        stringstream ss;
        for (size_t i = 0; i < seq.size(); ++i) {
            if (i > 0) ss << " -> ";
            ss << "P" << seq[i];
        }
        return ss.str();
    }

    // 运行交互菜单（新增查看日志选项）
    void runInteractive() {
        int choice;
        do {
            cout << "\n******** 银行家算法交互菜单 ********" << endl;
            cout << "1. 显示当前状态" << endl;
            cout << "2. 安全性检查（显示安全序列）" << endl;
            cout << "3. 进程请求资源" << endl;
            cout << "4. 进程释放资源" << endl;
            cout << "5. 查看操作日志" << endl;
            cout << "6. 退出程序" << endl;
            cout << "请输入选项: ";
            cin >> choice;
            if (cin.fail()) {
                cin.clear();
                cin.ignore(10000, '\n');
                cout << "输入非法，请重新输入。" << endl;
                continue;
            }
            switch (choice) {
            case 1:
                displayState();
                break;
            case 2:
                checkSafety();
                break;
            case 3: {
                int pid;
                vector<int> req(resourceCount);
                cout << "请输入进程ID (0~" << processCount - 1 << "): ";
                cin >> pid;
                cout << "请输入请求向量 (共" << resourceCount << "个整数): ";
                for (int j = 0; j < resourceCount; ++j) {
                    cin >> req[j];
                    if (cin.fail() || req[j] < 0) {
                        cin.clear();
                        cin.ignore(10000, '\n');
                        cout << "请求向量包含非法值，请重新输入。" << endl;
                        req.clear();
                        break;
                    }
                }
                if (req.size() == resourceCount) {
                    requestResources(pid, req);
                }
                break;
            }
            case 4: {
                int pid;
                cout << "请输入要释放资源的进程ID (0~" << processCount - 1 << "): ";
                cin >> pid;
                releaseResources(pid);
                break;
            }
            case 5:
                viewLog();
                break;
            case 6:
                logger.log("用户选择退出程序。");
                cout << "退出程序。" << endl;
                break;
            default:
                cout << "无效选项，请重新选择。" << endl;
            }
        } while (choice != 6);
    }
};

// 主函数
int main() {
    cout << "==================== 银行家算法模拟程序 ====================" << endl;
    cout << "请选择配置方式：1. 手动输入  2. 随机生成" << endl;
    int mode;
    cin >> mode;
    BankerAlgorithm* banker = nullptr;
    try {
        if (mode == 1) {
            banker = BankerAlgorithm::manualCreate();
        }
        else if (mode == 2) {
            banker = BankerAlgorithm::randomCreate();
        }
        else {
            cout << "无效输入，默认使用随机生成。" << endl;
            banker = BankerAlgorithm::randomCreate();
        }
        banker->runInteractive();
        delete banker;
    }
    catch (const exception& e) {
        cerr << "错误: " << e.what() << endl;
        if (banker) delete banker;
        return 1;
    }
    return 0;
}