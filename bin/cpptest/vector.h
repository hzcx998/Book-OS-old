#ifndef RVECTOR
#define RVECTOR
extern "C"{
    #include <stdlib.h>
}
template<typename T>
struct vectorNode {
    T data;
    vectorNode<T>* next;
};

template<typename T>
class vector {
private:
    vectorNode<T>* first;
    vectorNode<T>* last;
    unsigned long long size = 0;
public:
    void push_back(T data) {
        if(first == nullptr) {
            first = (vectorNode<T>*)malloc(sizeof(vectorNode<T>));
            last = first;
            first->data = data;
            size ++;
            return;
        }
        auto temp = (vectorNode<T>*)malloc(sizeof(vectorNode<T>));
        temp->data = data;
        last->next = temp;
        last = temp;
        size ++;
    }
    T pop_back() {
        T data = last->data;
        vectorNode<T>* ptr = first;
        if (ptr == last) {
            free(first);
            first = nullptr;
            last = nullptr;
            size --;
            return data;
        }
        while(true) {
            if(ptr->next == last) {
                break;
            }
        }
        free(last);
        last = ptr;
        size --;
        return data;
    }
};
#endif