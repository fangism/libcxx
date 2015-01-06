#ifndef COUNT_NEW_HPP
#define COUNT_NEW_HPP

# include <cstdlib>
# include <cassert>
# include <new>

#if  __has_feature(address_sanitizer) \
  || __has_feature(memory_sanitizer)
#define DISABLE_NEW_COUNT
#endif

class MemCounter
{
public:
    // Make MemCounter super hard to accidentally construct or copy.
    class MemCounterCtorArg_ {};
    explicit MemCounter(MemCounterCtorArg_) {}

private:
    MemCounter(MemCounter const &);
    MemCounter & operator=(MemCounter const &);

public:
    // All checks return true when disable_checking is enabled.
    static const bool disable_checking;

    int outstanding_new = 0;
    int new_called = 0;
    int delete_called = 0;
    int last_new_size = 0;

    int outstanding_array_new = 0;
    int new_array_called = 0;
    int delete_array_called = 0;
    int last_new_array_size = 0;

public:
    void newCalled(std::size_t s)
    {
        assert(s);
        ++new_called;
        ++outstanding_new;
        last_new_size = s;
    }

    void deleteCalled(void * p)
    {
        assert(p);
        --outstanding_new;
        ++delete_called;
    }

    void newArrayCalled(std::size_t s)
    {
        assert(s);
        ++outstanding_array_new;
        ++new_array_called;
        last_new_array_size = s;
    }

    void deleteArrayCalled(void * p)
    {
        assert(p);
        --outstanding_array_new;
        ++delete_array_called;
    }

    void reset()
    {
        outstanding_new = 0;
        new_called = 0;
        delete_called = 0;
        last_new_size = 0;

        outstanding_array_new = 0;
        new_array_called = 0;
        delete_array_called = 0;
        last_new_array_size = 0;
    }

public:
    bool checkOutstandingNewEq(int n) const
    {
        return disable_checking || n == outstanding_new;
    }

    bool checkOutstandingNewNotEq(int n) const
    {
        return disable_checking || n != outstanding_new;
    }

    bool checkNewCalledEq(int n) const
    {
        return disable_checking || n == new_called;
    }

    bool checkNewCalledNotEq(int n) const
    {
        return disable_checking || n != new_called;
    }

    bool checkDeleteCalledEq(int n) const
    {
        return disable_checking || n == delete_called;
    }

    bool checkDeleteCalledNotEq(int n) const
    {
        return disable_checking || n != delete_called;
    }

    bool checkLastNewSizeEq(int n) const
    {
        return disable_checking || n == last_new_size;
    }

    bool checkLastNewSizeNotEq(int n) const
    {
        return disable_checking || n != last_new_size;
    }

    bool checkOutstandingArrayNewEq(int n) const
    {
        return disable_checking || n == outstanding_array_new;
    }

    bool checkOutstandingArrayNewNotEq(int n) const
    {
        return disable_checking || n != outstanding_array_new;
    }

    bool checkNewArrayCalledEq(int n) const
    {
        return disable_checking || n == new_array_called;
    }

    bool checkNewArrayCalledNotEq(int n) const
    {
        return disable_checking || n != new_array_called;
    }

    bool checkDeleteArrayCalledEq(int n) const
    {
        return disable_checking || n == delete_array_called;
    }

    bool checkDeleteArrayCalledNotEq(int n) const
    {
        return disable_checking || n != delete_array_called;
    }

    bool checkLastNewArraySizeEq(int n) const
    {
        return disable_checking || n == last_new_array_size;
    }

    bool checkLastNewArraySizeNotEq(int n) const
    {
        return disable_checking || n != last_new_array_size;
    }
};

#ifdef DISABLE_NEW_COUNT
  const bool MemCounter::disable_checking = true;
#else
  const bool MemCounter::disable_checking = false;
#endif

MemCounter globalMemCounter((MemCounter::MemCounterCtorArg_()));

#ifndef DISABLE_NEW_COUNT
void* operator new(std::size_t s) throw(std::bad_alloc)
{
    globalMemCounter.newCalled(s);
    return std::malloc(s);
}

void  operator delete(void* p) throw()
{
    globalMemCounter.deleteCalled(p);
    std::free(p);
}


void* operator new[](std::size_t s) throw(std::bad_alloc)
{
    globalMemCounter.newArrayCalled(s);
    return operator new(s);
}


void operator delete[](void* p) throw()
{
    globalMemCounter.deleteArrayCalled(p);
    operator delete(p);
}

#endif // DISABLE_NEW_COUNT

#endif /* COUNT_NEW_HPP */
