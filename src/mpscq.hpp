#ifndef MPSCQ_H_
#define MPSCQ_H_

#include <atomic>
#include <functional>

template <typename T>
class Node {
 public:
  Node(T* data)
      : data(data) {
  }

  Node<T>* next = nullptr;
  T* data;
};

namespace mpscq {

template <typename T>
class Queue {
 public:
  void push(T* data) {
    Node<T>* node = new Node<T>(data);
    do {
      node->next = head;
    } while (!head.compare_exchange_weak(node->next, node));
  }

  void drain(std::function<void(T*)> callback) {
    visit(head, callback);
  }

 private:
  std::atomic<Node<T>*> head = nullptr;

  void visit(Node<T>* node, std::function<void(T*)> callback) {
    if (node != nullptr) {
      visit(node->next, callback);
      if (node->data != nullptr) {
        callback(node->data);
        delete node->data;
        node->data = nullptr;
        if (node->next != nullptr) {
          delete node->next;
          node->next = nullptr;
        }
      }
    }
  }
};

}  // namespace mpscq

#endif  // MPSCQ_H_
