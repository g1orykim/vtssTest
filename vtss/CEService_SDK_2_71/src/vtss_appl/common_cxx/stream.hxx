/*

 Vitesse Switch Software.

 Copyright (c) 2002-2013 Vitesse Semiconductor Corporation "Vitesse". All
 Rights Reserved.

 Unpublished rights reserved under the copyright laws of the United States of
 America, other countries and international treaties. Permission to use, copy,
 store and modify, the software and its source code is granted. Permission to
 integrate into other products, disclose, transmit and distribute the software
 in an absolute machine readable format (e.g. HEX file) is also granted.  The
 source code of the software may not be disclosed, transmitted or distributed
 without the written permission of Vitesse. The software and its source code
 may only be used in products utilizing the Vitesse switch products.

 This copyright notice must appear in any copy, modification, disclosure,
 transmission or distribution of the software. Vitesse retains all ownership,
 copyright, trade secret and proprietary rights in the software.

 THIS SOFTWARE HAS BEEN PROVIDED "AS IS," WITHOUT EXPRESS OR IMPLIED WARRANTY
 INCLUDING, WITHOUT LIMITATION, IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 FOR A PARTICULAR USE AND NON-INFRINGEMENT.

*/
#ifndef __STREAM_HXX__
#define __STREAM_HXX__

#include "string.hxx"
#include "formating_tags.hxx"

namespace VTSS {

struct ostream {
    virtual bool ok() const = 0;
    virtual bool push(char val) = 0;
    virtual void fill(unsigned cnt, char fill) = 0;
    virtual ~ostream() { }
};

template <typename T, typename S = ostream>
class obuffer_iterator {
  public:
    typedef T value_type;
    typedef value_type* pointer;
    typedef value_type& reference;

    explicit  obuffer_iterator(S& os) : os_(os) {}
    obuffer_iterator(const obuffer_iterator& iter) : os_(iter.os_) {}
    obuffer_iterator& operator=(const T& v) { os_ << v; return (*this); }
    obuffer_iterator& operator*()  { return (*this); }
    obuffer_iterator& operator++() { return (*this); }
    obuffer_iterator  operator++(int /*dummy*/)  { return (*this); }
    obuffer_iterator& operator--() { return (*this); }
    obuffer_iterator  operator--(int /*dummy*/)  { return (*this); }

    bool operator== (const obuffer_iterator& i) const {
        return (os_.pos() == i.os_.pos());
    }

    bool operator< (const obuffer_iterator& i) const {
        return (os_.pos() < i.os_.pos());
    }

  private:
    S& os_;
};

struct BufPtrStream : public ostream {
    explicit BufPtrStream(Buf *b)
            : ostream(), buf_(b), ok_(true) { pos_ = begin(); }

    explicit BufPtrStream()
            : ostream(), buf_(0), ok_(false), pos_(0)
    { }

    ~BufPtrStream() { }

    bool ok() const { return ok_; }

    bool push(char c) {
        if (!buf_) {
            ok_ = false;
            return false;
        }

        if (pos_ != buf_->end()) {
            *pos_++ = c;
            return true;

        } else {
            ok_ = false;
            return false;

        }
    }

    void fill(unsigned i, char c = ' ') {
        while (i--) push(c);
    }

    char * begin() {
        if (buf_) return buf_->begin();
        else      return NULL;
    }

    const char * begin() const {
        if (buf_) return buf_->begin();
        else      return NULL;
    }

    char * end() { return pos_; }
    const char * end() const { return pos_; }

    char * buf_end() {
        if (buf_) return buf_->end();
        else      return NULL;
    }

    const char * buf_end() const {
        if (buf_) return buf_->end();
        else      return NULL;
    }


    void clear() {
        pos_ = begin();
        ok_ = true;
    }

    Buf *detach() {
        Buf *tmp = buf_;
        ok_ = false;
        pos_ = 0;
        buf_ = 0;
        return tmp;
    }

    void attach(Buf *tmp) {
        buf_ = tmp;
        pos_ = buf_->begin();
        ok_ = true;
    }

  protected:
    Buf *buf_;
    bool ok_;
    char * pos_;
};

template<typename Buf>
struct BufStream : public ostream {
    BufStream() : impl(&buf_) { }
    virtual ~BufStream() { }

    bool ok() const { return impl.ok(); }
    bool push(char c) { return impl.push(c); }
    void fill(unsigned i, char c = ' ') { impl.fill(i, c); }
    char * end() { return impl.end(); }
    const char * end() const { return impl.end(); }
    char * begin() { return impl.begin(); }
    const char * begin() const { return impl.begin(); }
    void clear() { impl.clear(); }

  protected:
    Buf buf_;
    BufPtrStream impl;
};

// The StreamBuf tries to ease the implemntation of the following steps:
// - Parse and optional store a message header
// - Buffer up the incomming message
// - Process the input message (not implemented in this class)
// - Build a response message (only storage management is done here)
// - Build a response header (only storage management is done here)
// - Write the response header followed by the response message
//
// Internal the storage layout (might) looks like this:
//    +------------+
//    | Input-hdr  |
//    +------------+
//    | Input-msg  |
//    +------------+
//    | Output-msg |
//    +------------+
//    | Output-hdr |
//    +------------+
//    | Free-space |
//    +------------+
struct StreamBuf {
    StreamBuf (Buf *b) : buf_(b) { reset(); }

    virtual ~StreamBuf() { }

    const char *raw_buf_begin() const;
    const char *raw_buf_end() const;
    const BufPtr raw_buf() const;

    const char *input_hdr_begin() const { return buf_->begin(); }
    const char *input_hdr_end() const { return i_hdr_end(); }
    const BufPtr input_hdr() const;

    const char *input_msg_begin() const { return i_hdr_end(); }
    const char *input_msg_end() const { return i_msg_end(); }
    const str input_msg() const { return str(i_hdr_end(), i_msg_end()); }
    BufPtr input_msg() { return BufPtr(i_hdr_end(), i_msg_end()); }

    const char *output_msg_begin() const { return i_msg_end(); }
    const char *output_msg_end() const { return o_msg_end(); }
    const str output_msg() const { return str(i_msg_end(), o_msg_end()); }

    void reset() {
        overflow_ = false;
        input_header_size_ = 0;
        input_message_size_ = 0;
        output_header_size_ = 0;
        output_message_size_ = 0;
        output_header_written_ = 0;
        output_message_written_ = 0;
    }

    bool overflow() const { return overflow_; }

    // Push a single char to the input header area
    bool input_header_push(char c) {
        VTSS_ASSERT(input_message_size_ == 0);
        VTSS_ASSERT(output_message_size_ == 0);
        VTSS_ASSERT(output_header_size_ == 0);

        if (overflow_ || space_used() >= buf_->size()) {
            overflow_ = true;
            return false;
        }

        *i_hdr_end() = c;
        input_header_size_ += 1;
        return true;
    }

    // Push a range to the input header area
    bool input_header_push(const char *b, const char *e) {
        VTSS_ASSERT(input_message_size_ == 0);
        VTSS_ASSERT(output_message_size_ == 0);
        VTSS_ASSERT(output_header_size_ == 0);

        if (overflow_ || space_used() + (e - b) > buf_->size()) {
            overflow_ = true;
            return false;
        }

        copy(b, e, i_hdr_end());
        input_header_size_ += (e - b);
    }

    // Clear the input header area
    bool input_header_clear() {
        VTSS_ASSERT(input_message_size_ == 0);
        VTSS_ASSERT(output_message_size_ == 0);
        VTSS_ASSERT(output_header_size_ == 0);
        input_header_size_ = 0;
    }

    // Clear both the input header and the input message area
    bool input_header_message_clear() {
        VTSS_ASSERT(output_message_size_ == 0);
        VTSS_ASSERT(output_header_size_ == 0);
        input_message_size_ = 0;
        input_header_size_ = 0;
    }

    // Push content to the input message
    bool input_message_push(char c) {
        VTSS_ASSERT(output_message_size_ == 0);
        VTSS_ASSERT(output_header_size_ == 0);

        if (overflow_ || space_used() >= buf_->size()) {
            overflow_ = true;
            return false;
        }

        *(i_msg_end()) = c;
        input_message_size_ += 1;
        return true;
    }

    bool input_message_push(const char *b, const char *e) {
        VTSS_ASSERT(output_message_size_ == 0);
        VTSS_ASSERT(output_header_size_ == 0);

        if (overflow_ || space_used() + (e - b) > buf_->size()) {
            overflow_ = true;
            return false;
        }

        copy(b, e, i_msg_end());
        input_message_size_ += (e - b);
        return true;
    }

    // Clear the input message area
    bool input_message_clear() {
        VTSS_ASSERT(output_message_size_ == 0);
        VTSS_ASSERT(output_header_size_ == 0);
        input_message_size_ = 0;
    }

    // push a char to the output message area
    bool output_message_push(char c) {
        VTSS_ASSERT(output_header_size_ == 0);

        if (overflow_ || space_used() >= buf_->size()) {
            overflow_ = true;
            return false;
        }

        *o_msg_end() = c;
        output_message_size_ += 1;
        return true;
    }

    // Get a pointer to the free area (the area followed by the input message).
    // Note that if the input header and/or the input message is cleared, then
    // this must be done before calling this function.
    BufPtr output_message_area() {
        VTSS_ASSERT(output_header_size_ == 0);
        return BufPtr(o_msg_end(), buf_->end());
    }

    // Report how much of the output message is used
    bool output_message_commit(size_t s) {
        VTSS_ASSERT(output_header_size_ == 0);
        output_message_size_ += s;

        // Too late to avoid an buffer overflow, so there better not be one!
        VTSS_ASSERT(space_used() <= buf_->size());
        return true;
    }

    struct OutputMessageStream : public ostream {
        OutputMessageStream(StreamBuf &p) : parent(p) { }
        bool ok() const { return !parent.overflow(); }
        bool push(char val) { return parent.output_message_push(val); }
        void fill(unsigned i, char c) { while (i--) push(c); }
        ~OutputMessageStream() { }
      private:
        StreamBuf &parent;
    };

    // Like above, byt returns a stream
    OutputMessageStream output_message_stream() {
        return OutputMessageStream(*this);
    }

    // Like above, but used to build the output message
    bool output_header_push(char c) {
        if (overflow_ || space_used() >= buf_->size()) {
            overflow_ = true;
            return false;
        }

        *o_hdr_end() = c;
        output_header_size_ += 1;
        return true;
    }

    BufPtr output_header_area() {
        return BufPtr(o_hdr_end(), buf_->end());
    }

    bool output_header_commit(size_t s) {
        output_header_size_ += s;

        // Too late to avoid an buffer overflow, so there better not be one!
        VTSS_ASSERT(space_used() <= buf_->size());
        return true;
    }

    struct OutputHeaderStream : public ostream {
        OutputHeaderStream(StreamBuf &p) : parent(p) { }
        bool ok() const { return !parent.overflow(); }
        bool push(char val) { return parent.output_header_push(val); }
        void fill(unsigned i, char c) { while (i--) push(c); }
        ~OutputHeaderStream() { }
      private:
        StreamBuf &parent;
    };

    // Like above, byt returns a stream
    OutputHeaderStream output_header_stream() {
        return OutputHeaderStream(*this);
    }

    str  output_write_chunk() {
        if (output_header_size_ > output_header_written_) {
            return str(o_msg_end(), o_hdr_end());
        }

        if (output_message_size_ > output_message_written_) {
            return str(i_msg_end(), o_msg_end());
        }

        return str();
    }

    void output_write_commit(size_t s) {
        if (output_header_size_ > output_header_written_) {
            output_header_written_ += s;
            VTSS_ASSERT(output_header_written_ <= output_header_size_);
            return;
        }

        if (output_message_size_ > output_message_written_) {
            output_message_written_ += s;
            VTSS_ASSERT(output_message_written_ <= output_message_size_);
            return;
        }

        VTSS_ASSERT(0);
    }

    bool output_write_completed() const {
        return output_header_written_ == output_header_size_ &&
            output_message_written_ == output_message_size_;
    }

  private:
    size_t space_used() {
        return input_header_size_ + input_message_size_ +
            output_message_size_ + output_header_size_;
    }

    char *i_hdr_end() { return buf_->begin() + input_header_size_; }
    char *i_msg_end() { return i_hdr_end() + input_message_size_; }
    char *o_msg_end() { return i_msg_end() + output_message_size_; }
    char *o_hdr_end() { return o_msg_end() + output_header_size_; }
    const char *i_hdr_end() const { return buf_->begin() + input_header_size_; }
    const char *i_msg_end() const { return i_hdr_end() + input_message_size_; }
    const char *o_msg_end() const { return i_msg_end() + output_message_size_; }
    const char *o_hdr_end() const { return o_msg_end() + output_header_size_; }

    Buf *buf_;
    bool overflow_;
    size_t input_header_size_,      input_message_size_;
    size_t output_message_size_,    output_header_size_;
    size_t output_message_written_, output_header_written_;
};

#if defined(VTSS_OPSYS_ECOS)
//struct CliOstream : public ostream {
//    CliOstream() {
//        pIO = reinterpret_cast<cli_iolayer_t *>(cli_get_io_handle());
//    }
//
//    explicit CliOstream(cli_iolayer_t *io) : pIO(io) { }
//
//    bool ok() const {
//        return pIO != 0;
//    }
//
//    bool push(char c ) {
//        if (!pIO) {
//            return false;
//        }
//
//        pIO->cli_putchar(pIO, c);
//        return true;
//    }
//
//    void fill(unsigned i, char c = ' ') {
//        while (i--) push(c);
//    }
//
//  private:
//    cli_iolayer_t *pIO;
//};
#endif

template<unsigned S>
struct AlignStream :
    public StaticBuffer<S>,
    public ostream
{
    AlignStream ( ostream& o ) :
        o_(o), ok_(true),
        pos_(StaticBuffer<S>::begin()),
        flush_(StaticBuffer<S>::begin()) { }

    bool ok() const { return o_.ok(); }
    virtual void flush() = 0;

    bool push(char c) {
        if (pos_ != StaticBuffer<S>::end()) {
            *pos_++ = c;
            return true;
        } else {
            flush();
            return o_.push(c);
        }
    }

    void fill(unsigned i, char c) { while (i--) push(c); }
    char * pos() { return pos_; }
    const char * pos() const { return pos_; }

  protected:
    ostream& o_;
    bool ok_;
    char * pos_;
    char * flush_;
};

template<unsigned S>
struct RightAlignStream : public AlignStream<S>
{
    RightAlignStream( ostream& o, char f ) :
        AlignStream<S>( o ), fill_char(f) { }
    ~RightAlignStream() { flush(); }

    void flush() {
        typedef AlignStream<S> B;
        B::o_.fill(B::end() - B::pos_, fill_char);
        copy(B::flush_, B::pos_, obuffer_iterator<char>(B::o_));
        B::flush_ = B::pos_;
    }

    const char fill_char;
};

template<unsigned S>
struct LeftAlignStream : public AlignStream<S>
{
    LeftAlignStream( ostream& o, char f ) :
        AlignStream<S>( o ), fill_char(f) { }
    ~LeftAlignStream() { flush(); }
    void flush() {
        typedef AlignStream<S> B;
        copy(B::flush_, B::pos_, obuffer_iterator<char>(B::o_));
        B::o_.fill(B::end() - B::pos_, fill_char);
        B::flush_ = B::pos_;
    }

    const char fill_char;
};

template<unsigned S, typename T0>
ostream& operator<< (ostream& o, const FormatLeft<S, T0>& l)
{
    LeftAlignStream<S> los(o, l.fill);
    los << l.t;
    return o;
}

template<unsigned S, typename T0>
ostream& operator<< (ostream& o, const FormatRight<S, T0>& r)
{
    RightAlignStream<S> ros(o, r.fill);
    ros << r.t0;
    return o;
}

inline ostream& operator<<(ostream& o, const char &s) { o.push(s); return o; }

ostream& operator<<(ostream& o, const str &s);
ostream& operator<<(ostream& o, const char *s);
ostream& operator<<(ostream& o, const size_t s);

}  // namespace VTSS

#endif
