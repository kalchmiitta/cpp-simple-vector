#pragma once

#include <cassert>
#include <initializer_list>
#include "array_ptr.h"
#include <algorithm>
#include <stdexcept>
#include <math.h>
#include <iostream>

struct ReserveProxyObj {
    ReserveProxyObj () noexcept = default;
    
    ReserveProxyObj (size_t capacity_to_reserve)
            : reserve_(capacity_to_reserve) {
    }
    
    size_t reserve_ = 0;
};


template<typename Type>
class SimpleVector {
public:
    using Iterator = Type *;
    using ConstIterator = const Type *;
    
    SimpleVector () noexcept = default;
    
    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector (size_t size) : size_(size), capacity_(size), vector_(size) {
        std::fill(begin(), end(), Type());
    }
    
    SimpleVector (ReserveProxyObj reserve_proxy_obj)
            : size_(0), capacity_(reserve_proxy_obj.reserve_), vector_(size_) {
    }
    
    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector (size_t size, const Type &value) : size_(size), capacity_(size), vector_(size) {
        std::fill(begin(), end(), value);
    }
    
    // Создаёт вектор из std::initializer_list
    SimpleVector (std::initializer_list<Type> init)
            : size_(init.size()), capacity_(init.size()), vector_(init.size()) {
        std::copy(init.begin(), init.end(), begin());
    }
    
    SimpleVector (const SimpleVector &other)
            : size_(other.size_), capacity_(other.capacity_), vector_(other.size_) {
        std::copy(other.begin(), other.end(), begin());
    }
    
    SimpleVector &operator= (const SimpleVector &rhs) {
        if (this != &rhs) {
            SimpleVector current_vector(rhs);
            this->swap(current_vector);
        }
        return *this;
    }
    
    SimpleVector (SimpleVector &&other) noexcept {
        swap(other);
    }
    
    SimpleVector &operator= (SimpleVector &&rhs) noexcept {
        SimpleVector<Type> tmp(std::move(rhs));
        swap(tmp);
        return *this;
    }
    
    // Возвращает количество элементов в массиве
    size_t GetSize () const noexcept {
        return size_;
    }
    
    // Возвращает вместимость массива
    size_t GetCapacity () const noexcept {
        return capacity_;
    }
    
    // Сообщает, пустой ли массив
    bool IsEmpty () const noexcept {
        if (size_) {
            return false;
        } else {
            return true;
        }
    }
    
    // Возвращает ссылку на элемент с индексом index
    Type &operator[] (size_t index) noexcept {
        return vector_[index];
    }
    
    // Возвращает константную ссылку на элемент с индексом index
    const Type &operator[] (size_t index) const noexcept {
        return vector_[index];
    }
    
    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type &At (size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Invalid index");
        }
        return vector_[index];
    }
    
    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type &At (size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("Invalid index");
        }
        return vector_[index];
    }
    
    // Обнуляет размер массива, не изменяя его вместимость
    void Clear () noexcept {
        size_ = 0;
    }
    
    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize (size_t new_size) {
        if (new_size < size_ || new_size <= capacity_) {
            size_ = new_size;
        } else {
            Reserve(std::max(new_size, 2 * capacity_));
            size_ = new_size;
        }
    }
    
    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin () noexcept {
        return vector_.Get();
    }
    
    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end () noexcept {
        return &vector_[size_];
    }
    
    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin () const noexcept {
        return vector_.Get();
    }
    
    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end () const noexcept {
        return &vector_[size_];
    }
    
    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin () const noexcept {
        return vector_.Get();
    }
    
    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend () const noexcept {
        return &vector_[size_];
    }
    
    
    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack (const Type &item) {
        if (capacity_ > size_) {
            vector_[size_] = item;
        } else {
            Reserve(std::max(size_t(1), capacity_ * 2));
            vector_[size_] = item;
        }
        ++ size_;
    }
    
    void PushBack (Type &&item) {
        if (capacity_ > size_) {
            vector_[size_] = std::move(item);
        } else {
            size_t new_capacity = std::max(size_t(1), capacity_ * 2);
            ArrayPtr<Type> new_array_ptr(new_capacity);
            std::copy(std::make_move_iterator(begin()), std::make_move_iterator(end()), new_array_ptr.Get());
            new_array_ptr[size_] = std::move(item);
            new_array_ptr.swap(vector_);
            capacity_ = new_capacity;
        }
        ++ size_;
    }
    
    void Reserve (size_t capacity) {
        if (capacity > capacity_) {
            ArrayPtr<Type> new_vector(capacity);
            std::move(begin(), end(), new_vector.Get());
            vector_.swap(new_vector);
            capacity_ = capacity;
        }
        
    }
    
    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert (ConstIterator pos, const Type &value) {
        size_t num_pos = std::distance(cbegin(), pos);
        if (size_ < capacity_) {
            if (pos != end()) {
                std::move_backward(&vector_[num_pos], end(), &vector_[size_ + 1]);
            }
        } else {
            if (capacity_ == 0) {
                Reserve(1);
            } else {
                Reserve(2 * capacity_);
                std::move_backward(&vector_[num_pos], end(), &vector_[size_ + 1]);
            }
        }
        vector_[num_pos] = value;
        ++ size_;
        return &vector_[num_pos];
    }
    
    Iterator Insert (ConstIterator pos, Type &&value) {
        size_t num_pos = std::distance(cbegin(), pos);
        if (size_ < capacity_) {
            if (pos != end()) {
                std::move_backward(&vector_[num_pos], end(), &vector_[size_ + 1]);
            }
        } else {
            if (capacity_ == 0) {
                Reserve(1);
            } else {
                Reserve(2 * capacity_);
                std::move_backward(&vector_[num_pos], end(), &vector_[size_ + 1]);
            }
        }
        vector_[num_pos] = std::move(value);
        ++ size_;
        return &vector_[num_pos];
    }
    
    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack () noexcept {
        assert(! IsEmpty());
        -- size_;
    }
    
    // Удаляет элемент вектора в указанной позиции
    Iterator Erase (ConstIterator pos) {
        assert(pos != end());
        Iterator res_it = &vector_[std::distance(cbegin(), pos)];
        std::move(res_it + 1, end(), res_it);
        -- size_;
        return res_it;
    }
    
    // Обменивает значение с другим вектором
    void swap (SimpleVector &other) noexcept {
        std::swap(size_, other.size_);
        vector_.swap(other.vector_);
        std::swap(capacity_, other.capacity_);
    }

private:
    
    size_t size_ = 0;
    size_t capacity_ = 0;
    ArrayPtr<Type> vector_;
    
};

template<typename Type>
inline bool operator== (const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs) {
    return lhs.GetSize() == rhs.GetSize() && std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template<typename Type>
inline bool operator!= (const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs) {
    return ! (lhs == rhs);
}

template<typename Type>
inline bool operator< (const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
    
}

template<typename Type>
inline bool operator<= (const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs) {
    return (lhs < rhs || lhs == rhs);
}

template<typename Type>
inline bool operator> (const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs) {
    return ! (lhs <= rhs);
}

template<typename Type>
inline bool operator>= (const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs) {
    return (lhs == rhs || ! (lhs < rhs));
}

ReserveProxyObj Reserve (size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
};
