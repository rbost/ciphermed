#include <boost/thread/thread.hpp>
#include <boost/asio.hpp>

// the actual thread pool
struct ThreadPool {
    ThreadPool(std::size_t);
    template<class F>
    void enqueue(F f);
    ~ThreadPool();
    private:
    // the io_service we are wrapping
    boost::asio::io_service service;
//    using asio_worker = std::unique_ptr<boost::asio::io_service::work>;
    std::unique_ptr<boost::asio::io_service::work> working;
    // need to keep track of threads so we can join them
    std::vector<std::unique_ptr<boost::thread>> workers;
};

// the constructor just launches some amount of workers
ThreadPool::ThreadPool(size_t threads)
:service()
,working(new std::unique_ptr<boost::asio::io_service::work>::element_type(service))
{
    for ( std::size_t i = 0; i < threads; ++i ) {
        workers.push_back(
                          std::unique_ptr<boost::thread>(
                                                         new boost::thread([this]
                                                                           {
                                                                               service.run();
                                                                           })
                                                         )
                          );
    }
}

// add new work item to the pool
template<class F>
void ThreadPool::enqueue(F f) {
    service.post(f);
}

// the destructor joins all threads
ThreadPool::~ThreadPool() {
    //service.stop();
    //for ( std::size_t i = 0; i < workers.size(); ++i )
    //   workers[i]->join();
    working.reset();
    service.run();
}
