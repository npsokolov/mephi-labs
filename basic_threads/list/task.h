#pragma once

#include <cstdint>
#include <mutex>
#include <shared_mutex>
#include <set>
#include <atomic>
#include <vector>
#include <iostream>
#include <memory>

template<typename T>
struct TNode{
    T data;
    std::shared_ptr<TNode> next;
    std::shared_ptr<TNode> prev;
    std::mutex local_mutex;
    explicit TNode(const T& value) : data(value), next(nullptr), prev(nullptr) {}
    TNode() = default;
};

template<typename T>
class ThreadSafeList {
public:
    /*
     * Класс-итератор, позволяющий обращаться к элементам списка без необходимости использовать мьютекс.
     * При этом должен гарантироваться эксклюзивный доступ потока, в котором был создан итератор, к данным, на которые
     * он указывает.
     * Итератор, созданный в одном потоке, нельзя использовать в другом.
     */
    class Iterator {
    public:
        using pointer = T*;
        using value_type = T;
        using reference = T&;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::bidirectional_iterator_tag;

        explicit Iterator(std::shared_ptr<TNode<T>> ptr) : cur(ptr) {}

        T& operator *() {
            cur->local_mutex.lock();
            return cur->data;
        }

        T operator *() const {
            std::lock_guard<std::mutex> guard(cur->local_mutex);
            return cur->data;
        }

        T* operator ->() {
            std::lock_guard<std::mutex> guard1(cur->local_mutex);
            return &(cur->data);
        }

        const T* operator ->() const {
            std::lock_guard<std::mutex> guard1(cur->local_mutex);
            return &(cur->data);
        }

        Iterator& operator ++() {
            cur->local_mutex.try_lock();
            auto x = cur;
            cur=cur->next;
            x->local_mutex.unlock();
            return *this;
        }

        Iterator operator ++(int) {
            cur->local_mutex.try_lock();
            Iterator copy(*this);
            auto x = cur;
            cur=cur->next;
            x->local_mutex.unlock();
            return copy;
        }

        Iterator& operator --() {
            cur->local_mutex.try_lock();
            auto x = cur;
            cur=cur->prev;
            x->local_mutex.unlock();
            return *this;
        }

        Iterator operator --(int) {
            cur->local_mutex.try_lock();
            Iterator copy(*this);
            auto x = cur;
            cur=cur->prev;
            x->local_mutex.unlock();
            return copy;
        }

        bool operator ==(const Iterator& rhs) const {
            return (cur==rhs.cur);
        }

        bool operator !=(const Iterator& rhs) const {
            return (cur!=rhs.cur);
        }
        friend class ThreadSafeList;
    private:
        std::shared_ptr<TNode<T>> cur;
    };

    ThreadSafeList() : head(nullptr)/*tail(nullptr) tail(std::make_shared<TNode<T>>(new TNode<T>(10)))*/ {
        std::shared_ptr<TNode<T>> ptr(new TNode<T>);
        tail=ptr;
    }

    /*
     * Получить итератор, указывающий на первый элемент списка
     */
    Iterator begin() {
        std::lock_guard<std::mutex> guard(mutex_);
        return Iterator(head);
    }

    /*
     * Получить итератор, указывающий на "элемент после последнего" элемента в списке
     */
    Iterator end() {
        std::lock_guard<std::mutex> guard(mutex_);
        //return Iterator(nullptr);
        return Iterator(tail);
    }

    /*
     * Вставить новый элемент в список перед элементом, на который указывает итератор `position`
     */
    /*
    void insert(Iterator position, const T& value) {
        std::lock_guard<std::mutex> guard(mutex_);
        std::shared_ptr<TNode<T>> ptr(new TNode<T>(value));
        if (head){
            if (head==position.cur){
                head->prev=ptr;
                ptr->next=head;
                head=ptr;
            }else if (position.cur){
                position.cur->prev->next=ptr;
                ptr->prev=position.cur->prev;
                position.cur->prev=ptr;
                ptr->next=position.cur;
            }else{
                tail->next=ptr;
                ptr->prev=tail;
                tail=ptr;
            }
        }else{
            head=ptr;
            tail=ptr;
        }
    }
     */

    void insert(Iterator position, const T& value) {
        std::lock_guard<std::mutex> guard(mutex_);
        std::shared_ptr<TNode<T>> ptr(new TNode<T>(value));
        if (head){
            if (head==position.cur) {
                head->prev = ptr;
                ptr->next = head;
                head = ptr;
            }else if (position.cur==tail){
                tail->prev->next=ptr;
                ptr->prev=tail->prev;
                tail->prev=ptr;
                ptr->next=tail;
            }else{
                position.cur->prev->next=ptr;
                ptr->prev=position.cur->prev;
                position.cur->prev=ptr;
                ptr->next=position.cur;
            }
        }else{
            std::shared_ptr<TNode<T>> ptr1(new TNode<T>);
            tail=ptr1;
            head=ptr;
            head->next=tail;
            tail->prev=ptr;
        }
    }

    void erase(Iterator position) {
        std::lock_guard<std::mutex> guard(mutex_);
        if (head->next==tail){
            head=nullptr;
        }else if (position.cur==head){
            head=head->next;
            head->prev=nullptr;
        }else{
            position.cur->prev->next=position.cur->next;
            position.cur->next->prev=position.cur->prev;
        }

    }
private:
    std::shared_ptr<TNode<T>> head;
    std::shared_ptr<TNode<T>> tail;
    mutable std::mutex mutex_;
};
