#pragma once

#ifdef USE_BOOST_FUTURE
#define BOOST_THREAD_PROVIDES_FUTURE
#define BOOST_THREAD_PROVIDES_FUTURE_CONTINUATION
#define BOOST_RESULT_OF_USE_DECLTYPE
#include "boost/thread/future.hpp"
#endif

#ifdef USE_PPLTASKS

#include <ppltasks.h>

#define BOOST_RESULT_OF_USE_DECLTYPE
#include "boost/utility/result_of.hpp"

#endif

namespace dds {
    namespace rpc {

#ifdef USE_BOOST_FUTURE

        // Pull future<T> from boost into this namespace.
        using boost::future;

#endif 

#ifdef USE_PPLTASKS

        using concurrency::task;

        template <class R>
        class future;

        template <class ResultType>
        class shared_future
        {
            boost::shared_ptr<future<ResultType>> shfut_;

        public:

            explicit shared_future(future<ResultType> && fut);

            explicit shared_future(future<ResultType> & fut);
        };

        template <class ResultType>
        class future
        {
            task<ResultType> task_;

        public:

            typedef ResultType value_type;

            future();
            future(future && other);
            future& operator=(future && other);

            future(const future & rhs) = delete;
            future& operator=(const future & rhs) = delete;

            task<ResultType> to_task();
            future(const task<ResultType> & task);
            future(task<ResultType> && task);

            template<typename F>
            typename details::Unwrapper<typename boost::result_of<F(future &&)>::type>::return_type
                then(F&& func);

            template<typename F>
            typename details::Unwrapper<typename boost::result_of<F(future &&)>::type>::return_type
                then(const F & func);

            shared_future<ResultType> share();
            void swap(future& other);
            ResultType get();
            bool has_exception() const;
            bool has_value() const;
            bool is_ready() const;

            void wait() const;

            template <class Duration>
            void wait(const Duration &);
        };


#endif // USE_PPLTASKS

    } // namespace rpc
} // namespace dds


namespace dds {
namespace rpc {

#ifdef USE_PPLTASKS

    using concurrency::task;

    template <class ResultType>
    shared_future<ResultType>::shared_future(future<ResultType> && fut)
        : shfut_(boost::make_shared<future<ResultType>>(std::move(fut)))
    {}

    namespace details
    {

        template <typename RetType>
        struct Unwrapper
        {
            typedef future<RetType> return_type;

            template<typename Task, typename F>
            static return_type unwrap(Task & task, F&& func)
            {
                return task.then(std::forward<F>(func));
            }
        };

        template <typename T>
        struct Unwrapper<task<T>>
        {
            typedef future<T> return_type;

            template<typename Task, typename F>
            static return_type unwrap(Task & t, F&& func)
            {
                return t.then(std::forward<F>(func));
            }
        };

        template <typename T>
        struct Unwrapper<future<T>>
        {
            typedef future<T> return_type;

            template <typename Func, typename Task>
            class task_extractor
            {
                Func func;

            public:
                explicit task_extractor(Func && f)
                    : func(std::move(f))
                {}

                explicit task_extractor(const Func & f)
                    : func(f)
                {}

                template <class Arg>
                Task operator ()(Arg&& arg) const {
                    return func(std::forward<Arg>(arg)).to_task();
                }

                template <class Arg>
                Task operator ()(Arg&& arg) {
                    return func(std::forward<Arg>(arg)).to_task();
                }
            };

            template<typename Task, typename F>
            static return_type unwrap(Task & t, F&& func)
            {
                return t.then(task_extractor<F, task<T>>(std::forward<F>(func)));
            }
        };

    } // namespace details

    template<class ResultType>
    future<ResultType>::future()
        : task_() {}

    template<class ResultType>
    future<ResultType>::future(future<ResultType> && other)
        : task_(std::move(other.task_))
    { }

    template<class ResultType>
    future<ResultType> & future<ResultType>::operator =(future<ResultType> && other)
    {
        task_ = std::move(other.task_);
        return *this;
    }

    template<class ResultType>
    shared_future<ResultType>
        future<ResultType>::share()
    {
        return std::move(*this);
    }

    template<class ResultType>
    task<ResultType> future<ResultType>::to_task()
    {
        return std::move(task_);
    }

    template<class ResultType>
    future<ResultType>::future(const task<ResultType> & task)
        : task_(task)
    {}

    template<class ResultType>
    future<ResultType>::future(task<ResultType> && task)
        : task_(std::move(task))
    {}

    template<class ResultType>
    template<typename F>
    typename details::Unwrapper<typename boost::result_of<F(future<ResultType> &&)>::type>::return_type
        future<ResultType>::then(F&& func)
    {
        typedef typename boost::result_of<F(future &&)>::type RetType;
        return details::Unwrapper<RetType>::unwrap(task_, std::move(func));
    }

    template<class ResultType>
    template<typename F>
    typename details::Unwrapper<typename boost::result_of<F(future<ResultType> &&)>::type>::return_type
        future<ResultType>::then(const F & func)
    {
        typedef typename boost::result_of<F(future &&)>::type RetType;
        return details::Unwrapper<RetType>::unwrap(task_, func);
    }

    /* // original implementation
    template<typename F>
    future<typename boost::result_of<F(future &&)>::type>
    then(const F & func)
    {
    return task_.then(func);
    }
    */

    template<class ResultType>
    void future<ResultType>::swap(future& other)
    {
        using std::swap;
        swap(task_, other.task_);
    }

    template<class ResultType>
    ResultType future<ResultType>::get()
    {
        return task_.get();
    }

    template<class ResultType>
    void future<ResultType>::wait() const
    {
        task_.wait();
    }

    template<class ResultType>
    bool future<ResultType>::is_ready() const
    {
        return task_.is_done();
    }

#endif // USE_PPLTASKS

    namespace details
    {
#ifdef USE_BOOST_FUTURE

        using boost::promise;

#endif

#ifdef USE_PPLTASKS

        using concurrency::task_completion_event;

        template <class ResultType>
        class promise
        {
            task_completion_event<ResultType> tce_;

        public:

            future<ResultType> get_future() const
            {
                return future<ResultType>(task<ResultType>(tce_));
            }

            void set_value(ResultType & result) const
            {
                tce_.set(result);
            }

            template <class Ex>
            bool set_exception(Ex ex) const
            {
                return tce_.set_exception(ex);
            }
        };

#endif // USE_PPLTASKS
    
    } // namespace details
  } // namespace rpc
} // namespace dds

