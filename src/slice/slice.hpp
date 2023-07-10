#pragma once

#include <assert.h>
#include <cstring>
#include <malloc.h>
#include <ostream>
#include <string>

namespace dsbus {

class Slice {
    using LEN_TYPE = uint32_t;
    using ALLOC_TYPE = uint32_t;
    const size_t LEN_OFFSET = 0;
    const size_t ALLOC_OFFSET = LEN_OFFSET + sizeof(LEN_TYPE);
    const size_t STR_OFFSET = ALLOC_OFFSET + sizeof(ALLOC_OFFSET);
public:
    /**
     *  @brief Create an empty slice.
     */
    Slice() : Slice("") {}

    /**
     *  @brief Create a slice that refers to d[0, n-1].
     */
    Slice(const char *d, const size_t n) {
        buf_ = (char *)SliceAlloc(n);
        SetLen(n);
        SetAlloc(n);
        memcpy(Content(), d, n);
    }

    Slice(const std::string &s) : Slice(s.data(), s.size()) {}

    Slice(const char *d) : Slice(d, strlen(d)) {}

    /**
     *  @brief Copy constructor.
     * 
     *  Only allocate memory needed by the contents in this slice.
     */
    Slice(const Slice &other_slice) : Slice(other_slice.Data(), other_slice.Size()) {}

    ~Slice() { Free(); }

    /**
     *  @brief Returns the length of the contents.
     */
    size_t Size() const { return *GetLen(); }

    /**
     *  @brief Clear all contents.
     *  
     *  It does not free memory.
     */
    void Clear() { SetLen(0); }

    /**
     *  @brief Return const pointer to contents.
     *
     *  This is a pointer to internal data.  It is undefined to modify
     *  the contents through the returned pointer.
     */
    const char *Data() const {
        return Content();
    }
    
    std::string ToString() const {
        return std::string(Content());
    }

    /**
     *  @brief Free the allocated memory of this slice.
     * 
     *  [!!!CAUTION] The caller should ensures that this slice is no longer used.
     */
    void Free() {
        Free(buf_);
    }

    /**
     *  @brief Append new contents at the end of current content.
     * 
     *  If the appended content can be accommodated in the allocated memory, 
     *  the memory will not be reallocated.
     */
    void Append(const char *s, size_t append_len) {
        auto len = *GetLen();
        auto alloc = *GetAlloc();

        if (alloc - len > append_len) {
            memcpy(Content() + len, s, append_len);
            len += append_len;
            SetLen(len);
            return;
        }
        char *t_buf = (char *)SliceAlloc(alloc + append_len);
        *(LEN_TYPE *)(t_buf + LEN_OFFSET) = len + append_len;
        *(ALLOC_TYPE *)(t_buf + ALLOC_OFFSET) = alloc + append_len;
        memcpy(t_buf + STR_OFFSET, Content(), len);
        memcpy(t_buf + STR_OFFSET + len, s, append_len);
        Free(buf_);
        buf_ = t_buf;
        Content()[*GetLen()] = '\0';
    }

    void Append(const char *s) { Append(s, strlen(s)); }

    void Append(const Slice &slice) { Append(slice.Data(), slice.Size()); }

    /**
     *  @brief Get a sub slice.
     * 
     *  @param pos Index of first character.
     *  @param n Number of characters in sub slice.
     */
    Slice SubSlice(size_t pos, size_t n) const {
        size_t slice_len = Size();
        if (pos >= slice_len) return Slice("");
        size_t sub_len = pos + n > slice_len ? slice_len - pos : n;
        return Slice(Content() + pos, sub_len);
    }

    /**
     *  @brief Turn the slice into a smaller (or equal) slice containing only the
     *         sub slice specified by the 'start' and 'end' indexes.
     *         i.e., slice[begin, end]
     *  
     *  start and end can be negative, where -1 means the last character of the
     *  slice, -2 the penultimate character, and so forth.
     */
    void SubRange(int start, int end) {
        int slice_len = (int)Size();
        start = start >= 0 ? start : slice_len + start;
        end = end >= 0 ? end : slice_len + end;
        if (start > end || start > slice_len) {
            SetEmpty();
            return;
        }
        if (end > slice_len) end = slice_len - 1;
        int sub_slice_len = end - start + 1;
        if (sub_slice_len == slice_len) return;
        if (sub_slice_len == 0) {
            SetEmpty();
            return;
        }
        assert(sub_slice_len < slice_len);
        memcpy(Content(), Content() + start, sub_slice_len);
        SetLen(sub_slice_len);
    }

    /**
     *  @brief Set this slice empty.
     * 
     *  Reserve allocated memory.
     */
    void SetEmpty() {
        Clear();
    }

    /**
     *  @brief Compare whether the contents of two Slice are the same.
     * 
     *  The comparison will not be terminated prematurely 
     *  due to the content contains \0.
     */
    bool operator==(const Slice &rhs) {
        if (Size() != rhs.Size()) return false;
        auto lcontent = Content();
        auto rcontent = rhs.Data();
        for (size_t i = 0; i < Size(); ++i) {
            if (lcontent[i] != rcontent[i]) {
                return false;
            }
        }
        return true;
    }

    // Slice operator+(const Slice &rhs) {

    // }

    friend std::ostream & operator<<(std::ostream &out, Slice &slice);

private:

    /**
     *  @brief Alloc n bytes memory.
     */
    void *Alloc(size_t n) {
        return (void *)malloc(sizeof(char) * n);
    }

    /**
     *  @brief Free the specified memory.
     */
    void Free(void *d) {
        if (buf_ == nullptr) return;
        free(buf_);
    }

    /**
     *  @brief Alloc memory for slice.
     *  
     *  len_size + alloc_size + content_size + '\0'
     */
    void *SliceAlloc(size_t content_size) {
        return Alloc(sizeof(LEN_TYPE) + sizeof(ALLOC_TYPE) + content_size + 1);
    }

    /**
     *  @brief Return the pointer to the len of this slice.
     */
    LEN_TYPE *GetLen() const { return (LEN_TYPE *)(buf_ + LEN_OFFSET); }

    /**
     *  @brief Return the pointer to the alloc of this slice.
     */
    ALLOC_TYPE *GetAlloc() const { return (ALLOC_TYPE *)(buf_ + ALLOC_OFFSET); }

    /**
     *  @brief Set the len attribute.
     */
    void SetLen(size_t len) { *GetLen() = len; Content()[len] = '\0'; }

    /**
     *  @brief Set the alloc attribute.
     */
    void SetAlloc(size_t alloc) { *GetAlloc() = alloc; }

    /**
     *  @brief Return the pointer to the content of this slice.
     */
    char *Content() const { return buf_ + STR_OFFSET; }

private:
    /**
     *  @brief terminated with '\0'
     *         | len_ | alloc_ | contents | '\0' | 
     *  @ref len_(size_t) slice length 
     *  @ref alloc_(size_t) size of allocation 
     */
    char *buf_ = nullptr;
};

std::ostream & operator<<(std::ostream &out, Slice &slice) {
    out << (slice.buf_ + slice.STR_OFFSET);
    return out;
}


} // dsbus