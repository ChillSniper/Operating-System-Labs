#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <limits>

using namespace std;

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
        std::vector<int> serviceOrder;
        std::vector<int> moveDistances;
        int totalDistance;
        double averageDistance;
    };

    Result runSCAN() {
        std::vector<int> sortedReq = requests;
        std::sort(sortedReq.begin(), sortedReq.end());

        // 分离同向与逆向
        std::vector<int> sameDir, oppDir;
        for (int cyl : sortedReq) {
            if (initialDir == RIGHT) {
                (cyl >= initialHead ? sameDir : oppDir).push_back(cyl);
            } else {
                (cyl <= initialHead ? sameDir : oppDir).push_back(cyl);
            }
        }

        // 构建完整服务序列
        std::vector<int> seq = buildScanSequence(sameDir, oppDir);

        // 计算移动距离
        return computeStatistics(seq);
    }

private:
    int maxCylinder, initialHead;
    Direction initialDir;
    std::vector<int> requests;

    std::vector<int> buildScanSequence(
        std::vector<int>& sameDir, std::vector<int>& oppDir)
    {
        std::vector<int> seq;
        if (initialDir == RIGHT) {
            // 先升序扫描同向，再降序扫描逆向
            seq.insert(seq.end(), sameDir.begin(), sameDir.end());
            std::sort(oppDir.begin(), oppDir.end(), std::greater<int>());
            seq.insert(seq.end(), oppDir.begin(), oppDir.end());
            // 最后到达末端
            seq.push_back(maxCylinder);
        } else {
            // 先降序扫描同向，再升序扫描逆向
            std::sort(sameDir.begin(), sameDir.end(), std::greater<int>());
            seq.insert(seq.end(), sameDir.begin(), sameDir.end());
            std::sort(oppDir.begin(), oppDir.end());
            seq.insert(seq.end(), oppDir.begin(), oppDir.end());
            // 最后到达 0
            seq.push_back(0);
        }
        return seq;
    }

    Result computeStatistics(const std::vector<int>& seq) {
        Result res;
        res.totalDistance = 0;
        int curr = initialHead;

        for (int target : seq) {
            int dist = std::abs(target - curr);
            res.serviceOrder.push_back(target);
            res.moveDistances.push_back(dist);
            res.totalDistance += dist;
            curr = target;
        }

        int m = static_cast<int>(requests.size());
        res.averageDistance = m > 0
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
        for (int i = 0; i < n; ++i) std::cin >> reqs[i];

        std::cout << "磁头初始位置 head: ";
        std::cin >> head;
        std::cout << "初始方向（0 向左, 1 向右）: ";
        std::cin >> dir;
        std::cout << "最大柱面号 maxCyl: ";
        std::cin >> maxCyl;

        DiskScheduler::Direction d = (dir == 0 ? DiskScheduler::LEFT : DiskScheduler::RIGHT);

        DiskScheduler scheduler(maxCyl, head, d, reqs);
        auto result = scheduler.runSCAN();

        // 输出结果
        std::cout << "\n服务顺序及移动距离:\n";
        for (size_t i = 0; i < result.serviceOrder.size(); ++i) {
            std::cout << "[" << i+1 << "] -> "
                      << result.serviceOrder[i]
                      << " (移动 " << result.moveDistances[i] << ")\n";
        }
        std::cout << "\n总寻道长度: " << result.totalDistance << "\n"
                  << "平均寻道长度: " << result.averageDistance << "\n";
    }
    catch (const std::exception& ex) {
        std::cerr << "错误: " << ex.what() << std::endl;
        return 1;
    }
    return 0;
}
