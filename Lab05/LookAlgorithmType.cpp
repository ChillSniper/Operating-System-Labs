#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

class DiskScheduler {
public:
    enum Direction { LEFT = 0, RIGHT = 1 };

    DiskScheduler(int maxCyl, int head, Direction dir, const std::vector<int>& reqs)
        : maxCylinder(maxCyl), initialHead(head), initialDir(dir), requests(reqs)
    {
        if (maxCylinder < 0 || initialHead < 0 || initialHead > maxCylinder)
            throw std::invalid_argument("柱面号或初始磁头位置超出范围");
        for (int c : requests) {
            if (c < 0 || c > maxCylinder)
                throw std::invalid_argument("请求柱面号超出范围");
        }
    }

    struct Result {
        std::vector<int> serviceOrder;   // 服务序列
        std::vector<int> moveDistances;  // 每步移动距离
        int totalDistance;               // 总寻道距离
        double averageDistance;          // 平均寻道距离
    };

    // 执行 LOOK 调度
    Result runLOOK() {
        // 1. 排序所有请求
        std::vector<int> sortedReq = requests;
        std::sort(sortedReq.begin(), sortedReq.end());

        // 2. 分离同向与逆向
        std::vector<int> sameDir, oppDir;
        for (int cyl : sortedReq) {
            if (initialDir == RIGHT) {
                (cyl >= initialHead ? sameDir : oppDir).push_back(cyl);
            } else {
                (cyl <= initialHead ? sameDir : oppDir).push_back(cyl);
            }
        }

        // 3. 构建服务序列（LOOK：不扫至 0 或 maxCylinder，仅服务完所有请求就结束）
        std::vector<int> seq;
        if (initialDir == RIGHT) {
            // 同向升序，反向降序
            seq.insert(seq.end(), sameDir.begin(), sameDir.end());
            std::sort(oppDir.begin(), oppDir.end(), std::greater<int>());
            seq.insert(seq.end(), oppDir.begin(), oppDir.end());
        } else {
            // 同向降序，反向升序
            std::sort(sameDir.begin(), sameDir.end(), std::greater<int>());
            seq.insert(seq.end(), sameDir.begin(), sameDir.end());
            std::sort(oppDir.begin(), oppDir.end());
            seq.insert(seq.end(), oppDir.begin(), oppDir.end());
        }

        // 4. 统计移动距离
        return computeStatistics(seq);
    }

private:
    int maxCylinder;
    int initialHead;
    Direction initialDir;
    std::vector<int> requests;

    Result computeStatistics(const std::vector<int>& seq) {
        Result res;
        res.totalDistance = 0;
        int curr = initialHead;

        for (int target : seq) {
            int d = std::abs(target - curr);
            res.serviceOrder.push_back(target);
            res.moveDistances.push_back(d);
            res.totalDistance += d;
            curr = target;
        }

        int m = static_cast<int>(requests.size());
        res.averageDistance = (m > 0)
            ? static_cast<double>(res.totalDistance) / m
            : 0.0;
        return res;
    }
};

int main() {
    try {
        int n, head, dir, maxCyl;
        std::cout << "请求数 n: ";
        std::cin >> n;
        if (n < 0) throw std::invalid_argument("请求数必须为非负整数");

        std::vector<int> reqs(n);
        std::cout << "各请求柱面号（空格分隔）:\n";
        for (int i = 0; i < n; ++i) {
            std::cin >> reqs[i];
        }

        std::cout << "磁头初始位置 head: ";
        std::cin >> head;
        std::cout << "初始方向（0 向左, 1 向右）: ";
        std::cin >> dir;
        std::cout << "最大柱面号 maxCyl: ";
        std::cin >> maxCyl;

        DiskScheduler::Direction d =
            (dir == 0 ? DiskScheduler::LEFT : DiskScheduler::RIGHT);

        DiskScheduler scheduler(maxCyl, head, d, reqs);
        auto result = scheduler.runLOOK();

        std::cout << "\n服务顺序及移动距离:\n";
        for (size_t i = 0; i < result.serviceOrder.size(); ++i) {
            std::cout << "[" << i+1 << "] -> "
                      << result.serviceOrder[i]
                      << " (移动 " << result.moveDistances[i] << ")\n";
        }
        std::cout << "\n总寻道长度: " << result.totalDistance
                  << "\n平均寻道长度: " << result.averageDistance
                  << std::endl;
    }
    catch (const std::exception& ex) {
        std::cerr << "错误: " << ex.what() << std::endl;
        return 1;
    }
    return 0;
}
