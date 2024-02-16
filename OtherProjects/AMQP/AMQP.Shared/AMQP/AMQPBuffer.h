#pragma once
#include <Helpers/Logger.h>
#include <vector>

class AMQPBuffer {
public:
    AMQPBuffer(size_t size) 
        : m_data(size, 0) 
    {
    }

    size_t write(const char* data, size_t size) {
        if (m_use == m_data.size()) {
            return 0;
        }

        const size_t length = (size + m_use);
        size_t write = length < m_data.size() ? size : m_data.size() - m_use;

        memcpy(m_data.data() + m_use, data, write);
        m_use += write;
        //LOG_DEBUG_D("Amqp data = {}", std::string{ m_data.data(), m_data.size() });
        return write;
    }

    void drain() {
        /* auto size = m_data.size();
         m_data.clear();
         m_data.resize(size, 0);*/
        m_use = 0;
    }

    size_t available() const {
        return m_use;
    }

    size_t size() const {
        return m_data.size();
    }

    const char* data() const {
        return m_data.data();
    }

    void shl(size_t count) {
        const size_t diff = m_use - count;
        std::memmove(m_data.data(), m_data.data() + count, diff);
        m_use = m_use - count;
    }

private:
    std::vector<char> m_data;
    size_t m_use = 0;
};