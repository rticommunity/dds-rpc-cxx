#ifndef OMG_DDS_RPC_INFRASTRUCTURE_H
#define OMG_DDS_RPC_INFRASTRUCTURE_H

#include <string>
#include <stdexcept>
#include <memory>

#include "ndds/ndds_namespace_cpp.h"

#include "rpc_types.h"

#ifdef USE_RTI_CONNEXT

#include "connext_cpp/connext_cpp_infrastructure.h"

namespace dds { namespace rpc {

#ifdef IMPLEMENTATION_DEPENDENT

using connext::Sample;
using connext::SampleRef;
using connext::SampleInfo;
using connext::WriteSample; 
using connext::WriteSampleRef;
using connext::LoanedSamples;
using connext::SampleIterator;
using connext::dds_type_traits;

#endif // IMPLEMENTATION_DEPENDENT

template <class T>
class SharedSamples;

} // namespace rpc
} // namespace dds

#else 

namespace dds { namespace rpc {


typedef DDS_SampleInfo SampleInfo;
typedef DDS_SampleInfoSeq SampleInfoSeq; 

template<typename T, bool> class SampleIterator;
template<typename T>       class Sample;
template<typename T>       class SampleRef;
template<class T>          class LoanedSamples;

template <typename T> 
struct dds_type_traits
{
    typedef typename T::Seq          Seq;
    typedef const typename T::Seq    ConstSeq;
    typedef SampleRef<T>             SampleRefType;
    typedef SampleRef<T>             SampleIteratorValueType;
    typedef SampleRef<const T>       ConstSampleIteratorValueType;
    typedef SampleIterator<T, false> iterator;
    typedef SampleIterator<T, true>  const_iterator;
    typedef typename T::TypeSupport  TypeSupport;
    typedef typename T::DataReader   DataReader;
    typedef typename T::DataWriter   DataWriter;
    typedef LoanedSamples<T>         LoanedSamplesType;
};

template <> 
struct dds_type_traits<SampleInfo> 
{
    typedef SampleInfoSeq       Seq; 
    typedef const SampleInfoSeq ConstSeq;
};

namespace details {

template <bool, typename T, typename U>
struct if_ {
    typedef T type;
};

template <typename T, typename U>
struct if_ <false, T, U> {
    typedef U type;
};

} // namespace details

template <typename T>
class Sample 
{
public:
    typedef T                                        Data;
    typedef T&                                       DataReference;
    typedef const T&                                 ConstDataReference;
    typedef typename dds_type_traits<T>::Seq         Seq;
    typedef typename dds_type_traits<T>::TypeSupport TypeSupport;
    typedef typename dds_type_traits<T>::DataReader  DataReader;
    typedef typename dds_type_traits<T>::DataWriter  DataWriter;

    Sample();
    
    Sample(const Sample<T>& other);

    Sample(SampleRef<T> sample_ref);

    Sample(const T &data, const SampleInfo& info);

    Sample & operator = (SampleRef<T> sample_ref);

    SampleIdentity_t identity() const;

    SampleIdentity_t related_identity() const;
    
    void swap(Sample & other);

    T& data();

    SampleInfo& info();

    const T& data() const;

    const SampleInfo& info() const;

    operator DataReference ();

    operator ConstDataReference () const;

    void set_data(const T &t);

    void set_data(const T *t);

    void set_info(const SampleInfo &i);

    void set_info(const SampleInfo *i);
};

template <typename T>
class SampleRef
{
public:
    SampleRef();

    SampleRef(T * data, SampleInfo * info);

    SampleRef(T & data, SampleInfo & info);

    SampleRef(Sample<T> & sample);

    SampleRef & operator = (Sample<T> & sample);

    T & data() const;

    SampleInfo & info() const;

    operator T & () const;

    SampleRef<T> * operator ->();

    const SampleRef<T> * operator ->() const;

    void set_data(T * data);

    void set_data(T & data);

    void set_info(SampleInfo * info);

    void set_info(SampleInfo & info);

    bool is_nil_data() const;

    bool is_nil_info() const;

    void swap(SampleRef & other) throw();

    SampleIdentity_t identity() const;

    SampleIdentity_t related_identity() const;
};

template <typename T, bool IsConst>
class SampleIterator 
{
public:
   typedef std::random_access_iterator_tag iterator_category;

   typedef typename details::if_<
       IsConst, 
       typename dds_type_traits<T>::ConstSampleIteratorValueType, 
       typename dds_type_traits<T>::SampleIteratorValueType>::type 
         value_type;

   typedef value_type reference;
   typedef value_type pointer;
   typedef std::ptrdiff_t difference_type;

   typedef typename dds_type_traits<T>::Seq Seq;

   typedef typename details::if_<
       IsConst, 
       typename dds_type_traits<T>::ConstSeq, 
       typename dds_type_traits<T>::Seq>::type TSeq;

   typedef typename details::if_<
       IsConst, 
       typename dds_type_traits<SampleInfo>::ConstSeq, 
       typename dds_type_traits<SampleInfo>::Seq>::type InfoSeq;

   SampleIterator();

   explicit SampleIterator(TSeq & seq, 
                           InfoSeq & info_seq, 
                           int position = 0);

   SampleIterator (const SampleIterator<T, false> & si);

   SampleIterator & operator = (const SampleIterator<T, false> & si);

   value_type operator * () const;

   value_type operator ->() const;

   value_type operator [] (difference_type offset) const;

   SampleIterator & operator ++ ();

   SampleIterator operator ++ (int);

   SampleIterator & operator -- ();

   SampleIterator operator -- (int);

   SampleIterator & operator -= (difference_type i);

   SampleIterator & operator += (difference_type i);

   SampleIterator operator - (difference_type i) const;

   SampleIterator operator + (difference_type i) const;

   difference_type operator - (const SampleIterator & s2) const;

   bool operator < (const SampleIterator & s2) const;

   bool operator > (const SampleIterator & s2) const;

   bool operator <= (const SampleIterator & s2) const;

   bool operator >= (const SampleIterator & s2) const;

   bool operator == (const SampleIterator & s2) const;

   bool operator != (const SampleIterator & s2) const;

   bool is_end() const;
};

template <class T>
class LoanedSamples;

template <typename T>
LoanedSamples<T> move(LoanedSamples<T> & ls) throw();

template <typename T>
class LoanedSamples
{
public:
   typedef typename dds_type_traits<T>::Seq                             TSeq;
   typedef typename dds_type_traits<T>::DataReader                      TDataReader;

   typedef SampleIterator<T, false>                                     iterator;

   typedef SampleIterator<T, true>                                      const_iterator;

   typedef typename dds_type_traits<T>::SampleIteratorValueType         value_type;
   typedef typename dds_type_traits<T>::ConstSampleIteratorValueType    const_value_type;
   typedef std::ptrdiff_t                                               difference_type;

  struct LoanProxy 
  {
    void * data;
  };

public:

    LoanedSamples();
    
    static LoanedSamples move_construct_from_loans(
        TDataReader * reader, 
        TSeq & data_seq, 
        SampleInfoSeq & info_seq);

    void release(
        TDataReader *& reader_ptr, 
        TSeq & data_seq, 
        SampleInfoSeq & info_seq); 

#ifdef OMG_DDS_RPC_HASCPP11    

    LoanedSamples(const LoanedSamples &) = delete;
    LoanedSamples & operator = (const LoanedSamples &) = delete;

    LoanedSamples(LoanedSamples &&) noexcept;
    LoanedSamples & operator = (LoanedSamples &&) noexcept;

#else
    LoanedSamples(LoanProxy loan_memento) throw();

    LoanedSamples & operator = (LoanProxy loan_memento) throw();

    operator LoanProxy () throw();

    friend LoanedSamples move<> (LoanedSamples &) throw();
#endif 

    ~LoanedSamples() throw();

    TSeq & data_seq(); 

    SampleInfoSeq & info_seq(); 

    const TSeq & data_seq() const;

    const SampleInfoSeq & info_seq() const;

    value_type operator [] (size_t index);

    const_value_type operator [] (size_t index) const;

    int length() const;

    void set_datareader(TDataReader * reader);

    void return_loan();

    void swap(LoanedSamples & that) throw();

    iterator begin();

    iterator end();

    const_iterator begin() const;

    const_iterator end() const;
};

template <typename T>
class WriteSampleRef;

template <typename T>
class WriteSample  
{
public:

  WriteSample();
  
  WriteSample(const WriteSample<T>& other);
  
  WriteSample(DataInitType data, WriteParams_t& info);

  WriteSample(DataInitType data);

  WriteSample(WriteSampleRef<T> wsref);

  WriteSample & operator = (WriteSampleRef<T> wsref);

  SampleIdentity_t identity() const;
};

template <typename T>
class WriteSampleRef 
{
public:

  WriteSampleRef();

  WriteSampleRef(T & data, WriteParams_t & wparams);

  WriteSampleRef(WriteSample<T> & ws);

  WriteSampleRef & operator = (WriteSample<T> & ws);

  T & data() const;

  WriteParams_t & info() const;

  void set_data(T & data);

  void set_data(T * data);

  void set_info(WriteParams_t & info);

  void set_info(WriteParams_t * info);

  bool is_nil_data() const;

  bool is_nil_info() const;

  void swap(WriteSampleRef & other);

  SampleIdentity_t identity() const;
};


#ifdef OMG_DDS_RPC_HASCPP11    

template <typename T>
LoanedSamples<T> move(LoanedSamples<T> & ls) noexcept 
{
    return std::move(ls);
}

#else
template <typename T>
LoanedSamples<T> move(LoanedSamples<T> & ls) throw()
{
    return LoanedSamples<T>(typename LoanedSamples<T>::LoanProxy(ls));
}
#endif // OMG_DDS_RPC_HASCPP11

template <typename T>
typename LoanedSamples<T>::iterator begin(LoanedSamples<T> & ls)
{
    return ls.begin();
}

template <typename T>
typename LoanedSamples<T>::const_iterator begin(const LoanedSamples<T> & ls)
{
    return ls.begin();
}

template <typename T>
typename LoanedSamples<T>::iterator end(LoanedSamples<T> & ls)
{
    return ls.end();
}

template <typename T>
typename LoanedSamples<T>::const_iterator end(const LoanedSamples<T> & ls)
{
    return ls.end();
}

template <typename T>
void swap(LoanedSamples<T> &ls1, LoanedSamples<T> &ls2) throw()
{
    ls1.swap(ls2);
}

} // namespace rpc
} // namespace DDS 

#endif // USE_RTI_CONNEXT


#endif // OMG_DDS_RPC_INFRASTRUCTURE_H

