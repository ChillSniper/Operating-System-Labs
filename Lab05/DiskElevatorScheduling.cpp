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
            throw std::invalid_argument("����Ż��ʼ��ͷλ�ó�����Χ");
        for (int c : requests) {
            if (c < 0 || c > maxCylinder)
                throw std::invalid_argument("��������ų�����Χ");
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

        // ����ͬ��������
        std::vector<int> sameDir, oppDir;
        for (int cyl : sortedReq) {
            if (initialDir == RIGHT) {
                (cyl >= initialHead ? sameDir : oppDir).push_back(cyl);
            } else {
                (cyl <= initialHead ? sameDir : oppDir).push_back(cyl);
            }
        }

        // ����������������
        std::vector<int> seq = buildScanSequence(sameDir, oppDir);

        // �����ƶ�����
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
            // ������ɨ��ͬ���ٽ���ɨ������
            seq.insert(seq.end(), sameDir.begin(), sameDir.end());
            std::sort(oppDir.begin(), oppDir.end(), std::greater<int>());
            seq.insert(seq.end(), oppDir.begin(), oppDir.end());
            // ��󵽴�ĩ��
            seq.push_back(maxCylinder);
        } else {
            // �Ƚ���ɨ��ͬ��������ɨ������
            std::sort(sameDir.begin(), sameDir.end(), std::greater<int>());
            seq.insert(seq.end(), sameDir.begin(), sameDir.end());
            std::sort(oppDir.begin(), oppDir.end());
            seq.insert(seq.end(), oppDir.begin(), oppDir.end());
            // ��󵽴� 0
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
        std::cout << "������ n: ";
        std::cin >> n;
        if (n < 0) throw std::invalid_argument("����������Ϊ�Ǹ�����");

        std::vector<int> reqs(n);
        std::cout << "����������ţ��ո�ָ���:\n";
        for (int i = 0; i < n; ++i) std::cin >> reqs[i];

        std::cout << "��ͷ��ʼλ�� head: ";
        std::cin >> head;
        std::cout << "��ʼ����0 ����, 1 ���ң�: ";
        std::cin >> dir;
        std::cout << "�������� maxCyl: ";
        std::cin >> maxCyl;

        DiskScheduler::Direction d = (dir == 0 ? DiskScheduler::LEFT : DiskScheduler::RIGHT);

        DiskScheduler scheduler(maxCyl, head, d, reqs);
        auto result = scheduler.runSCAN();

        // ������
        std::cout << "\n����˳���ƶ�����:\n";
        for (size_t i = 0; i < result.serviceOrder.size(); ++i) {
            std::cout << "[" << i+1 << "] -> "
                      << result.serviceOrder[i]
                      << " (�ƶ� " << result.moveDistances[i] << ")\n";
        }
        std::cout << "\n��Ѱ������: " << result.totalDistance << "\n"
                  << "ƽ��Ѱ������: " << result.averageDistance << "\n";
    }
    catch (const std::exception& ex) {
        std::cerr << "����: " << ex.what() << std::endl;
        return 1;
    }
    return 0;
}
