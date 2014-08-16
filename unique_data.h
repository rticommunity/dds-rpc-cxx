#ifndef UNIQUE_DATA_H
#define UNIQUE_DATA_H

#include <utility>

template <class T>
class unique_data
{
  T * ptr_;

  unique_data(const unique_data &);
  unique_data & operator = (const unique_data &);

public:
  unique_data()
    : ptr_(T::TypeSupport::create_data())
  {
    if (!ptr_)
      throw std::runtime_error("Can't create data");
  }

  unique_data(unique_data && ud)
    : ptr_(ud.ptr_)
  {
    ud.ptr_ = 0;
  }
  
  unique_data & operator = (unique_data && ud)
  {
    std::swap(ptr_, ud.ptr_);
    return *this;
  }

  unique_data(T *t)
    : ptr_(t)
  {}

  T * operator -> () {
    return ptr_;
  }

  T * get() {
    return ptr_;
  }

  T & operator * () {
    return *ptr_;
  }

  ~unique_data() {
    if (ptr_)
      T::TypeSupport::delete_data(ptr_);
  }
};

#endif // UNIQUE_DATA_H
