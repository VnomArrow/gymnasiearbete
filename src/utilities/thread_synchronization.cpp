#include "thread_synchronization.h"
#include "thread.h"
#include "time_functions.h"

void thread_::ThreadPairListener::waitIfStopped() {
	if (caller->stop_lock.try_lock()) {
		caller->stop_lock.unlock();
	}
	else {
		// Registered request to stop

		// Show caller that the thread has stopped
		not_stopped_lock.unlock();

		// Wait for thread to stop being locked
		caller->stop_lock.lock();
		caller->stop_lock.unlock();
	}
}
void thread_::ThreadPairCommander::startThread() {
	// Tell the listener to not stop
	stop_lock.unlock();
}
void thread_::ThreadPairCommander::stopThreadBlocking() {
	// Tell the listener to stop
	stop_lock.lock();

	// Make sure the listener has stopped
	listener->not_stopped_lock.lock();
	listener->not_stopped_lock.unlock();
}
void thread_::ThreadPairCommander::stopThreadNonBlocking() {
	stop_lock.lock();
}
bool thread_::ThreadPairCommander::isThreadStopped() {
	if (listener->not_stopped_lock.try_lock()) {
		listener->not_stopped_lock.unlock();
		return true;
	}
	else {
		return false;
	}
}
bool thread_::ThreadPairCommander::isListenerInited() {
	if (listener->init_lock.try_lock()) {
		listener->init_lock.unlock();
		return false;
	}
	else {
		return true;
	}
}



void thread_::SlaveThread::loop() {
	master_thread->increaseActiveThreadCount();
	while (true) {
		//log__vv("Slave: wait for master to start");
		// Wait for master thread to start executing threads
		start_lock.lock();
		start_lock.unlock();

		//log__vv("Slave: see if thread should be deleted");
		// See if the thread should be deleted
		if (alive_lock.try_lock() == false) {
			// The thread is still alive
		}
		else {
			// The thread is dead and should be deleted
			alive_lock.unlock();
			MasterThread* my_master_thread = master_thread;
			delete(this);
			my_master_thread->reduceActiveThreadCount();
			return;
		}

		//log__vv("Slave: show master that the slave is running");
		// Show master that the thread is running
		is_running_lock.lock();

		//log__vv("Slave: Make sure the master has confirmed that the thread is running");
		// Make sure the master has confirmed that the thread is running
		confirm_lock.lock();
		confirm_lock.unlock();

		//log__vv("Slave: Do work");
		// Do work
		work();

		//log__vv("Slave: Show master that the thread is done");
		// Show master that the thread is done
		is_running_lock.unlock();
	}
}

void thread_::MasterThread::startThreadLoop(SlaveThread* thread) {
	thread->loop();
}

void thread_::MasterThread::insertThread(SlaveThread* thread) {
	// Make sure the thread doesnt start running immediately
	thread->start_lock.lock();

	// Show thread that it should not delete itself
	thread->alive_lock.lock();

	// Insert thread to list
	threads.push_front(thread);
	thread->threads_ref = threads.begin();
	thread->master_thread = this;

	// Start the loop
	thread__createThread(&thread_::MasterThread::startThreadLoop, thread);
}

void thread_::MasterThread::deleteThread(SlaveThread* thread) {
	// Call the child class destructor
	thread->child_destructor();

	// Remove thread from list of threads
	threads.erase(thread->threads_ref);

	// Show thread that it should delete itself
	thread->alive_lock.unlock();

	// Start thread so that it can delete itself
	thread->start_lock.unlock();
}

void thread_::MasterThread::deleteAllThreads() {
	// Delete all threads
	while (threads.size() != 0) {
		deleteThread(threads.front());
	}

	// Make sure all threads has been deleted
	while (getActiveThreadCount() > 0) {
		sleep(5);
	}
}

void thread_::MasterThread::executeThreads() {
	std::list<SlaveThread*> unfinished_threads = {};
	
	//log__vv("Master: start threads")
	std::list<SlaveThread*>::iterator thread_it = threads.begin();
	while (thread_it != threads.end()) {
		// Lock the confirmation lock to make sure the thread has started later
		(*thread_it)->confirm_lock.lock();

		// Start the thread
		(*thread_it)->start_lock.unlock();

		thread_it++;
	}

	//log__vv("Master: add unstarted threads to list")
	thread_it = threads.begin();
	// Make sure all threads have started
	while (thread_it != threads.end()) {
		if ((*thread_it)->is_running_lock.try_lock()) {
			// The thread has not climed the lock and is not ready to run

			// Add it to list of unfinished threads
			unfinished_threads.push_back((*thread_it));

			(*thread_it)->is_running_lock.unlock();
		}
		else {
			// The thread has climed the lock and is ready to work

			// Make sure it stops once it is done
			(*thread_it)->start_lock.lock();

			// Release thread
			(*thread_it)->confirm_lock.unlock();
		}
		thread_it++;
	}

	//log__vv("Master: make sure all threads have started")
	// Wait for all threads to start
	while (unfinished_threads.size() != 0) {
		thread_it = unfinished_threads.begin();
		while (thread_it != unfinished_threads.end()) {
			if ((*thread_it)->is_running_lock.try_lock()) {
				// The thread has not climed the lock and is not ready to run

				(*thread_it)->is_running_lock.unlock();
			}
			else {
				// The thread has climed the lock and is ready to work

				// Make sure it stops once it is done
				(*thread_it)->start_lock.lock();

				// Release thread
				(*thread_it)->confirm_lock.unlock();

				// Remove it from unfinished threads
				std::list<SlaveThread*>::iterator thread_it_copy = thread_it;
				thread_it++;
				unfinished_threads.erase(thread_it_copy);
				continue;
			}
			thread_it++;
		}
	}

	//log__vv("Master: All threads have started")
	// Do some work
	work();

	//log__vv("Master: add threads that are still working to list")
	thread_it = threads.begin();
	// Wait for all threads to finish
	while (thread_it != threads.end()) {
		if ((*thread_it)->is_running_lock.try_lock()) {
			// The thread has released the lock and is done working

			(*thread_it)->is_running_lock.unlock();
		}
		else {
			// The thread has climed the lock and is still working

			// Add it to list of unfinished threads
			unfinished_threads.push_back((*thread_it));
		}
		thread_it++;
	}

	//log__vv("Master: wait for all threads to stop working")
	// Wait for all threads to finish
	while (unfinished_threads.size() != 0) {
		thread_it = unfinished_threads.begin();
		while (thread_it != unfinished_threads.end()) {
			if ((*thread_it)->is_running_lock.try_lock()) {
				// The thread has released the lock and is done working

				(*thread_it)->is_running_lock.unlock();

				// Remove it from unfinished threads
				std::list<SlaveThread*>::iterator thread_it_copy = thread_it;
				thread_it++;
				unfinished_threads.erase(thread_it_copy);
				continue;
			}
			else {
				// The thread has climed the lock and is still working
			}
			thread_it++;
		}
	}

	//log__vv("Master: execute function complete!")
	// All threads finished
	return;
}

thread_::MasterThread::~MasterThread() {
	deleteAllThreads();
}

void thread_::MasterThread::increaseActiveThreadCount() {
	thread_count_lock.lock();
	thread_count++;
	thread_count_lock.unlock();
}

void thread_::MasterThread::reduceActiveThreadCount() {
	thread_count_lock.lock();
	thread_count--;
	thread_count_lock.unlock();
}

int thread_::MasterThread::getActiveThreadCount() {
	int return_val;
	thread_count_lock.lock();
	return_val = thread_count;
	thread_count_lock.unlock();
	return return_val;
}