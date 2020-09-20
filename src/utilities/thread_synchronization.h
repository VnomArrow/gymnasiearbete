#pragma once
#include "memory_leak_detector.h"
#include "mutex.h"
#include "logger.h"
#include <list>

namespace thread_ {
	class ThreadPairCommander;

	class ThreadPairListener {
	public:
		void waitIfStopped();
		void sync();
		void init();
		void deinit();
	private:
		ThreadPairListener(ThreadPairCommander* thread_caller) {
			caller = thread_caller;
		}
		ThreadPairCommander* caller;

		std::mutex not_stopped_lock;
		std::mutex init_lock;

		friend ThreadPairCommander;
	};

	class ThreadPairCommander {
	public:
		void startThread();
		void stopThreadBlocking();
		void stopThreadNonBlocking();
		void sync();
		bool try_sync();
		bool listenerTryingToSync();
		bool isThreadStopped();
		bool isListenerInited();
		void init();
		void deinit();
		ThreadPairCommander(ThreadPairListener*& listener_ref) {
			listener = new ThreadPairListener(this);
			listener_ref = listener;
		}
	private:
		ThreadPairListener* listener;

		std::mutex sync_lock_a;
		std::mutex sync_lock_b;
		std::mutex sync_lock_c;

		std::mutex stop_lock;

		friend ThreadPairListener;
	};

	inline void createThreadPairCommunication(ThreadPairCommander*& commander, ThreadPairListener*& listener) {
		commander = new thread_::ThreadPairCommander(listener);
	}

	inline void ThreadPairListener::init() {
		caller->sync_lock_a.lock();
		not_stopped_lock.lock();
		init_lock.lock();
	}
	inline void ThreadPairListener::deinit() {
		caller->sync_lock_a.unlock();
		not_stopped_lock.unlock();
		init_lock.unlock();
	}

	inline void ThreadPairListener::sync() {
		caller->sync_lock_c.lock();
		caller->sync_lock_a.unlock();
		caller->sync_lock_b.lock();

		caller->sync_lock_c.unlock();
		caller->sync_lock_a.lock();
		caller->sync_lock_b.unlock();
	}

	inline void ThreadPairCommander::init() {
		sync_lock_b.lock();
	}
	inline void ThreadPairCommander::deinit() {
		sync_lock_b.unlock();
	}

	inline void ThreadPairCommander::sync() {
		sync_lock_a.lock();
		sync_lock_b.unlock();
		sync_lock_c.lock();

		sync_lock_a.unlock();
		sync_lock_b.lock();
		sync_lock_c.unlock();
	}

	inline bool ThreadPairCommander::try_sync() {
		if (sync_lock_a.try_lock()) {
			sync_lock_b.unlock();
			sync_lock_c.lock();

			sync_lock_a.unlock();
			sync_lock_b.lock();
			sync_lock_c.unlock();
			return true;
		}
		else {
			return false;
		}
	}

	inline bool ThreadPairCommander::listenerTryingToSync() {
		if (sync_lock_a.try_lock()) {
			sync_lock_a.unlock();
			return true;
		}
		else {
			return false;
		}
	}

}



namespace thread_ {
	class MasterThread;

	class SlaveThread {
	public:
		virtual void work() {};
	protected:
		// Will be called on object deletetion
		// Use MasterThread::deleteThread to delete object
		virtual void child_destructor() {};

		// WARNING! Should only be called inside SlaveThread::loop
		// Use MasterThread::deleteThread instead
		~SlaveThread() { log__vv("~SlaveThread"); }
	private:
		void loop();
		std::list<SlaveThread*>::iterator threads_ref;
		std::mutex start_lock;
		std::mutex alive_lock;
		std::mutex is_running_lock;
		std::mutex confirm_lock;
		MasterThread* master_thread;

		friend MasterThread;
	};


	// All functions must be called on the same thread that created the object!
	class MasterThread {
	public:
		virtual void work() {};

		void insertThread(SlaveThread* thread);
		void deleteAllThreads();
		void deleteThread(SlaveThread* thread);
		inline int getThreadCount() { return threads.size(); }

		// Execute threads and wait until they are done
		void executeThreads();

		MasterThread(){}
		~MasterThread();
	private:
		static void startThreadLoop(SlaveThread* thread);
		void increaseActiveThreadCount();
		void reduceActiveThreadCount();
		int getActiveThreadCount();
		int thread_count = 0;
		std::mutex thread_count_lock;

		std::list<SlaveThread*> threads = {};
		friend SlaveThread;

	};
}