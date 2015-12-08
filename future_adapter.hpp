#pragma once

#ifdef USE_BOOST_FUTURE

#define BOOST_THREAD_PROVIDES_FUTURE
#define BOOST_THREAD_PROVIDES_FUTURE_CONTINUATION
#define BOOST_RESULT_OF_USE_DECLTYPE

#include "boost/thread/future.hpp"
#include <experimental\resumable>

namespace boost {

  template <typename T>
  bool await_ready(future<T> const & t)
  {
    return t.is_ready();
  }

  template <typename T, typename Callback>
  void await_suspend(future<T> & t, Callback resume)
  {
    t.then([resume](future<T> const &)
    {
      resume();
    });
  }

  template <typename T>
  T await_resume(future<T> & t)
  {
    return t.get();
  }

} // namespace boost

namespace std {
  namespace experimental {

    template <class T, class... Args>
    struct coroutine_traits<boost::future<T>, Args...>
    {
      struct promise_type
      {
        boost::promise<T> promise;

        boost::future<T> get_return_object() { return promise.get_future(); }
        bool initial_suspend() { return false; }
        bool final_suspend() { return false; }

        void set_exception(std::exception_ptr e) {
          promise.set_exception(std::move(e));
        }
        /*
        void set_result(T & t) {
        promise.set_value(t);
        }
        void set_result(T && t) {
        promise.set_value(std::move(t));
        }
        */
        void return_value(T & t) {
          promise.set_value(t);
        }
        void return_value(T && t) {
          promise.set_value(std::move(t));
        }
      };
    };

    template <class... Args>
    struct coroutine_traits<boost::future<void>, Args...>
    {
      struct promise_type
      {
        boost::promise<void> promise;

        boost::future<void> get_return_object() { return promise.get_future(); }
        bool initial_suspend() { return false; }
        bool final_suspend() { return false; }

        void set_exception(std::exception_ptr e) {
          promise.set_exception(std::move(e));
        }
        /*
        void set_result() {
        promise.set_value();
        }
        */
        void return_void() {
          promise.set_value();
        }
      };
    };

  } // namespace experimental
} // namespace std

#endif

#ifdef USE_PPLTASKS

#include <ppltasks.h>

#ifdef USE_AWAIT
#include <pplawait.h>
#endif // USE_AWAIT

#define BOOST_RESULT_OF_USE_DECLTYPE

#include "boost/utility/result_of.hpp"

#endif // USE_PPLTASKS

namespace dds {
    namespace rpc {

#ifdef USE_BOOST_FUTURE

        // Pull future<T> from boost into this namespace.
        using boost::future;

        namespace details {

          template <class T>
          boost::future<std::decay_t<T>> make_ready_future(T&& t)
          {
            typename std::experimental::coroutine_traits<future<std::decay_t<T>>>::promise_type promise;
            future<std::decay_t<T>> fut = promise.get_return_object();
            promise.return_value(std::forward<T>(t));
            return fut;
          }

        } // namespace details
#endif 

#ifdef USE_PPLTASKS

        namespace details {
            
            template <class R>
            class promise;
        
        } // namespace details
        
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

            typedef dds::rpc::details::promise<ResultType> promise_type;
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
            typename details::Unwrapper<typename std::decay<typename boost::result_of<F(future<ResultType> &&)>::type>::type>::return_type
                then(F&& func);

            template<typename F>
            typename details::Unwrapper<typename std::decay<typename boost::result_of<F(future<ResultType> &&)>::type>::type>::return_type
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

        // Unwrapper adapts lambdas passed to .then function such that the return
        // type of the lambda is something PPL Task library can understand. 
        // There are two cases: 
        // (1) If the return type of the lambda is anything but 
        //     rpc::future<T>, unwrapping is basically a no-op. I.e., unwrap 
        //     just forwards the closure to the underlying .then(). This includes
        //     types like int, float, and even task<T>. It works because PPL task
        //     library knows how to handle lambdas that return task<T>.
        // (2) If the return type of the lambda is rpc::future<T>, 
        //     Unwrapper<future<T>>::task_extractor is a lambda adapter that extracts 
        //     the task<T> out from it so that the PPL task library does not 
        //     create a ridiculous type like task<rpc::future<T>> when we want
        //     just rpc::future<T>. Note that PPL task library has no knowledge
        //     of rpc::future<T>.
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
            static return_type unwrap(Task & task, F&& func)
            {
                return task.then(std::forward<F>(func));
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
    future<ResultType> & future<ResultType>::operator = (future<ResultType> && other)
    {
        task_ = std::move(other.task_);
        return *this;
    }

    template<class ResultType>
    shared_future<ResultType> future<ResultType>::share()
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
    typename details::Unwrapper<typename std::decay<typename boost::result_of<F(future<ResultType> &&)>::type>::type>::return_type
        future<ResultType>::then(F&& func)
    {
        typedef typename std::decay<typename boost::result_of<F(future &&)>::type>::type FRetType;
        return details::Unwrapper<FRetType>::unwrap(task_, std::forward<F>(func));
    }

    template<class ResultType>
    template<typename F>
    typename details::Unwrapper<typename std::decay<typename boost::result_of<F(future<ResultType> &&)>::type>::type>::return_type
        future<ResultType>::then(const F & func)
    {
        typedef typename std::decay<typename boost::result_of<F(future &&)>::type>::type FRetType;
        return details::Unwrapper<FRetType>::unwrap(task_, func);
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

    template <typename T>
    bool await_ready(future<T> const & t)
    {
        return t.is_ready();
    }

    template <typename T, typename Callback>
    void await_suspend(future<T> & t, Callback resume)
    {
        t.then([resume](future<T> const &)
        {
           resume();
        });
    }
    
    template <typename T>
    T await_resume(future<T> & t)
    {
        return t.get();
    }

#endif // USE_PPLTASKS

    namespace details
    {

#ifdef USE_BOOST_FUTURE

        using boost::promise;

#endif

#ifdef USE_PPLTASKS

        using concurrency::task_completion_event;

        template <class T>
        future<std::decay_t<T>> make_ready_future(T&& t)
        {
          details::promise<std::decay_t<T>> promise;
          future<std::decay_t<T>> fut = promise.get_future();
          promise.set_result(std::forward<T>(t));
          return fut;
        }

        template <class ResultType>
        class promise
        {
            task_completion_event<ResultType> tce_;

        public:

            future<ResultType> get_future() const {
                return future<ResultType>(task<ResultType>(tce_));
            }

            void set_value(ResultType & result) const {
                tce_.set(result);
            }

            bool set_exception(std::exception_ptr exptr) const {
                return tce_.set_exception(std::move(exptr));
            }

            promise& operator = (promise&& other) {
                tce_ = std::move(other.tce_);
                return *this;
            }
            promise& operator = (const promise& rhs) = delete;

            void swap(promise & p2) {
                using std::swap;
                swap(tce_, p2.tce_);
            }

            /* await-compatibility functions */
            future<ResultType> get_return_object() { return get_future(); }
            bool initial_suspend()                 { return false; }
            bool final_suspend()                   { return false; }
            void set_result(ResultType & t) {
                tce_.set(t);
            }
            void set_result(ResultType && t) {
                tce_.set(std::move(t));
            }
            void return_value(ResultType & t) {
                tce_.set(t);
            }
            void return_value(ResultType && t) {
                tce_.set(std::move(t));
            }
        };

        template <>
        class promise <void>
        {
            task_completion_event<void> tce_;

        public:

            future<void> get_future() const  {
                return future<void>(task<void>(tce_));
            }

            void set_value() const {
                tce_.set();
            }

            bool set_exception(std::exception_ptr exptr) const {
                return tce_.set_exception(std::move(exptr));
            }

            promise& operator = (promise&& other) {
                tce_ = std::move(other.tce_);
                return *this;
            }
            promise& operator = (const promise& rhs) = delete;

            void swap(promise & p2) {
                using std::swap;
                swap(tce_, p2.tce_);
            }

            /* await-compatibility functions */
            future<void> get_return_object() { return get_future(); }
            bool initial_suspend()           { return false; }
            bool final_suspend()             { return false; }
            void set_result()                { set_value(); }
            void return_void()               { set_value(); }
        };

#endif // USE_PPLTASKS
    
    } // namespace details
  } // namespace rpc
} // namespace dds
