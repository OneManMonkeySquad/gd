#include "event_stream.h"

namespace fd
{
    event_stream_iterator_t::event_stream_iterator_t(const event_stream_t& stream, size_t header_idx)
        : _stream(stream)
        , _header_idx(header_idx)
    {
    }

    std::pair<int, const std::uint8_t*> event_stream_iterator_t::operator*() {
        const  auto& header = _stream._headers[_header_idx];

        return std::make_pair<int, const std::uint8_t*>(header.type, _stream._data.data() + _payload_offset);
    }

    bool event_stream_iterator_t::operator != (const event_stream_iterator_t& other) {
        return (_header_idx != other._header_idx);
    }

    event_stream_iterator_t& event_stream_iterator_t::operator++() {
        const  auto& header = _stream._headers[_header_idx];
        _payload_offset += header.payload_size;
        ++_header_idx;
        return *this;
    }
}