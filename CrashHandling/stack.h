#pragma once
#include "definition.h"

#include <vector>
#include <list>

template <typename T, typename Container = std::list<T> >
class stack
{
public:
    typedef typename Container::value_type             value_type;
    typedef typename Container::pointer                pointer;
    typedef typename Container::reference              reference;
    typedef typename Container::const_reference        const_reference;
    typedef typename Container::size_type              size_type;
    typedef typename Container::distance_type          distance_type;
    typedef typename Container::reverse_iterator       iterator;
    typedef typename Container::const_reverse_iterator const_iterator;
    typedef typename Container::iterator               reverse_iterator;
    typedef typename Container::const_iterator         const_reverse_iterator;

    stack() {}
    ~stack() {}

    inline const bool empty()const
    {
        return container.empty();
    }

    inline const size_type size()const
    {
        return container.size();
    }

    inline reference top()
    {
        return container.back();
    }

    inline reference bottom()
    {
        return container.front();
    }

    inline const_reference top()const
    {
        return container.back();
    }

    inline const_reference bottom()const
    {
        return container.front();
    }

    inline void push(const T& x)
    {
        container.push_back(x);
    }

    inline void push_unique(const T& x)
    {
        container.push_back_unique(x);
    }

    inline void pop()
    {
        container.pop_back();
    }

    inline iterator begin()
    {
        return container.rbegin();
    }

    inline const_iterator begin()const
    {
        return container.rbegin();
    }

    inline iterator end()
    {
        return container.rend();
    }

    inline const_iterator end()const
    {
        return container.rend();
    }

    inline const_reference operator[](size_type n)const
    {
        return container[container.size() - n - 1];
    }

    inline reference operator[](size_type n)
    {
        return container[container.size() - n - 1];
    }
protected:
    Container container;
};
