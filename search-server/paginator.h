#pragma once
#include <iostream>
#include <vector>

template <typename Iterator>
class IteratorRange {
public:
    IteratorRange(Iterator begin, Iterator end) :
        first_(begin),
        last_(end),
        size_(distance(first_, last_)) {
    }

    Iterator begin() const {
        return first_;
    }

    Iterator end() const {
        return last_;
    }

    size_t size() const {
        return size_;
    }
private:
    Iterator first_, last_;
    size_t size_;
};

template <typename Iterator>
std::ostream& operator<<(std::ostream& os, const IteratorRange<Iterator>& range) {
    for (Iterator it = range.begin(); it != range.end(); ++it) {
        os << *it;
    }
    return os;
}

template <typename Iterator>
class Paginator {
public:
    explicit Paginator(Iterator begin, Iterator end, size_t page_size) {
        for (size_t left = distance(begin, end); left > 0;) {
            const size_t current_page_size = std::min(page_size, left);
            const Iterator current_page_end = next(begin, current_page_size);
            page_iterators_.push_back({ begin, current_page_end });
            left -= current_page_size;
            begin = current_page_end;
        }
    }

    auto begin() const {
        return page_iterators_.begin();
    }

    auto end() const {
        return page_iterators_.end();
    }

    size_t size() const {
        return page_iterators_.size();
    }
private:
    std::vector<IteratorRange<Iterator>> page_iterators_;
};

template <typename Container>
auto Paginate(const Container& container, size_t page_size) {
    return Paginator(begin(container), end(container), page_size);
}