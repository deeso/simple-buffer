//
// Created by apridgen on 6/5/21.
//

#ifndef SIMPLE_BUFFER_CLOOPER_H
#define SIMPLE_BUFFER_CLOOPER_H
#include <thread>
#include <atomic>
#include <memory>
#include <functional>
#include <stdexcept>
#include <mutex>
#include <queue>
#include <iostream>
//https://www.bfilipek.com/2019/12/threading-loopers-cpp17.html

namespace simple_buffer {
    class CLooper
    {

    public:
        using Runnable = std::function<void()>;

        class CDispatcher
        {
            friend class CLooper; // Allow the looper to access the private constructor.

        public:
            bool post(CLooper::Runnable &&aRunnable)
            {
                return mAssignedLooper.post(std::move(aRunnable));
            }

        private: // construction, since we want the looper to expose it's dispatcher exclusively!
            explicit CDispatcher(CLooper &aLooper)
                    : mAssignedLooper(aLooper)
            {}

        private:
            CLooper &mAssignedLooper;
        };

    public:
        CLooper()
                : mRunning(false)
                , mAbortRequested(false)
                , mRunnables()
                , mRunnablesMutex()
                , mDispatcher(std::shared_ptr<CDispatcher>(new CDispatcher(*this)))
        { }
        // Copy denied, Move to be implemented

        ~CLooper()
        {
            abortAndJoin();
        }

        bool running() const
        {
            return mRunning.load();
        }

        bool run()
        {
            try
            {
                mThread = std::thread(&CLooper::runFunc, this);
            }
            catch(...)
            {
                return false;
            }

            return true;
        }

        void stop()
        {
            abortAndJoin();
        }

        std::shared_ptr<CDispatcher> getDispatcher()
        {
            return mDispatcher;
        }

    private:
        void runFunc()
        {
            mRunning.store(true);

            while(false == mAbortRequested.load())
            {
                try
                {
                    Runnable r = next();
                    if(nullptr != r)
                    {
                        r();
                    }
                }
                catch(std::runtime_error& e)
                {
                    // Some more specific
                }
                catch(...)
                {
                    // Make sure that nothing leaves the thread for now...
                }
            }

            mRunning.store(false);
        }

        void abortAndJoin()
        {
            mAbortRequested.store(true);
            if(mThread.joinable())
            {
                mThread.join();
            }
        }

        // Runnables
        Runnable next()
        {
            std::lock_guard guard(mRunnablesMutex);

            if(mRunnables.empty())
            {
                return nullptr;
            }
            Runnable runnable = mRunnables.front();
            mRunnables.pop();

            return runnable;
        }

        bool post(Runnable &&aRunnable)
        {
            if(not running())
            {
                // Deny insertion
                std::cout << "Denying insertion, as the looper is not running.\n";
                return false;
            }

            try
            {
                std::lock_guard guard(mRunnablesMutex);

                mRunnables.push(std::move(aRunnable));
            }
            catch(...) {
                return false;
            }

            return true;
        }

    private:
        std::thread      mThread;
        std::atomic_bool mRunning;
        std::atomic_bool mAbortRequested;

        std::queue<Runnable>  mRunnables;
        std::recursive_mutex  mRunnablesMutex;

        std::shared_ptr<CDispatcher> mDispatcher;
    };
}

#endif //SIMPLE_BUFFER_CLOOPER_H
