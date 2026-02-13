
#include "libshm_node_mem_manager.h"
#include <iostream>
#include <string>

namespace tvushm {

#if 0
    struct ExampleData {
        int id;
        std::string name;

        ExampleData(int i = 0, const std::string& n = "") : id(i), name(n) {}

        void print() const {
            std::cout << "ID: " << id << ", Name: " << name << std::endl;
        }
    };

    static int test_main() {
        // 创建节点内存管理器
        NodeMemoryManager<ExampleData> manager(10);

        // 申请节点
        auto* node1 = manager.allocate(ExampleData(1, "Node1"));
        if (node1) {
            node1->data.print();
        }

        auto* node2 = manager.allocate(ExampleData(2, "Node2"));
        if (node2) {
            node2->data.print();
        }

        // 使用RAII句柄
        NodeHandle<ExampleData> handle1(&manager, manager.allocate(ExampleData(3, "Handle1")));
        if (handle1.isValid()) {
            handle1->print();
        }

        // 获取节点
        auto* retrieved_node = manager.getNode(0);
        if (retrieved_node && retrieved_node->is_used) {
            retrieved_node->data.print();
        }

        // 释放节点
        manager.deallocate(node1);
        handle1.reset(); // RAII自动释放

        std::cout << "Used count: " << manager.getUsedCount() << std::endl;
        std::cout << "Capacity: " << manager.getCapacity() << std::endl;

        // 遍历所有使用中的节点
        manager.forEachUsedNode([](NodeMemoryManager<ExampleData>::Node& node) {
            node.data.print();
        });

        return 0;
    }

#endif

}

