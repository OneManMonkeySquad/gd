#pragma once

#include "api.h"
#include <vector>

namespace fd
{
    struct event_stream_t;

    struct event_header_t
    {
        std::uint16_t type;
        std::uint16_t payload_size;
    };

    struct API event_stream_iterator_t
    {
        const event_stream_t& _stream;
        size_t _header_idx = 0;
        size_t _payload_offset = 0;

        event_stream_iterator_t(const event_stream_t& stream, size_t header_idx);

        std::pair<int, const std::uint8_t*> operator*();

        bool operator != (const event_stream_iterator_t& other);

        event_stream_iterator_t& operator++();
    };

    struct event_stream_t
    {
        std::vector<event_header_t> _headers;
        std::vector<uint8_t> _data;

        void append(int type)
        {
            event_header_t header;
            header.type = type;
            header.payload_size = 0;
            _headers.push_back(header);
        }

        template<typename T>
        void append(int type, const T& payload)
        {
            auto prev_data_size = _data.size();
            _data.resize(prev_data_size + sizeof(payload));

            auto data_ptr = _data.data() + prev_data_size;
            memcpy(data_ptr, &payload, sizeof(T));

            event_header_t header;
            header.type = type;
            header.payload_size = sizeof(T);
            _headers.push_back(header);
        }

        void clear()
        {
            _headers.clear();
            _data.clear();
        }

        event_stream_iterator_t begin()
        {
            return { *this, 0 };
        }

        event_stream_iterator_t end()
        {
            return { *this, _headers.size() };
        }

        event_stream_iterator_t cbegin() const
        {
            return { *this, 0 };
        }

        event_stream_iterator_t cend() const
        {
            return { *this, _headers.size() };
        }
    };
}