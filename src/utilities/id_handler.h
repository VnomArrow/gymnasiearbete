#pragma once
#include "memory_leak_detector.h"
#include "logger.h"
#include <list>
#include <vector>
#include "mutex.h"

/*
For assigning ids to class instances


Advantages:
 - Adds ability to have ids acting as pointers 
    while developing for multiplayer
 - You can get a list of all active ids
 - Removes issues with deleted pointers
 - Makes multithreading much easier if it can be
     compartamentalized

Disadvanatges:
 - Slightly decreases pointer time acces
 - Takes up more memory than pointers

Implementation:
	Basic: 

	Create a class and inherit the IdDealer
	Create a class and inherit the IdBatchManager
		the OBJECT being the pointer-type you want to store
		the DEALER being the class inheriting IdDealer

	The basic idea is that you input a pointer and get a id 
		instead. Instead of storing pointers you then store that 
		id, when you want the pointer you can then ask for it by
		using the id. Then when you are done with the object 
		and have asked to remove the id, everyone asking for the id
		will recieve a nullptr.

	The IdBatchManagers job is to keep track of and destribute
		IdBatches, an IdBatch is basically an array that contains 
		1024 positions where pointers can be stored and then
		accesed using an id. Batch 0 will have positions 
		0 - 1023, batch 1 will have positions 1024 - 2047 etc.
	The IdBatchManagers job is also to create dealers. 

	The dealers job is to destribute the ids from the batches
	    it owns. If the inputObject(ptr) is called, it will find
		an available position and store the ptr there,
		and then return an id the position can be accesed through. 
		If no available position could be found it will ask the 
		IdBatchManager for a new Batch, adding 1024 new fresh positions 
		to assign to.
	When the getObject(id) is called
		it will return the pointer that could be found on the
		position given by the id. If it could not find the id
		it will return a nullptr.
	Once the removeObject(id) is called, every getObject(id) for 
	    that id will return nullpointers, and that id will never
		be used ever again. The position however can be used multiple
		times. This is because every id does not only contain a position, 
		but also a key. If an id tries to acces a position with a key
		that does not match it will return nullptr, no matter what is on 
		the position. Everytime a new pointer is assigned to a position, 
		that positions key will increase and the id that is returned will
		have a key that matches that. The id is built up by the first 
		32bits representing the postion and the last 32bits the key.
	The reason for having multiple dealers comes down to thread 
	    safety. If a dealer tries to acces another dealers id the
		program will exit with an error. This makes it easy to make
		sure that two threads does not try to alter the others objects
		by having them using different dealers. If you wish to transfer 
		a pointer to another dealer you can just call removeObject(id) on 
		the old dealer and then inputObject(ptr) on the new, note that this 
		will generate a new id. You can also always acces an id by first 
		calling getResponsibleDealer(id), which returns the dealer that owns 
		the id, and then ask that dealer for the id. 


	Advanced:

	Other than ids that are assigned to certain dealers, there are also something 
		called static ids. When calling IdBatchManager(static_id_count), the first
		static_id_count amount of ids will be static ids. If static_id_count is 100,
		the ids ranging from 0 - 99 will be static. 
	The static ids will be accesible by all dealers, which makes them perfect for having
		static objects since multi thread acces of objects that are not altered will not
		pose a thread risk. It is also great if you want to guaraty that a certain id points
		to a certain object. You can acces the position by using staticIdRef(static_id).

	When dealing with multiplayer, the IdTracker will also come in handy. It is very much
		like a dealer, but you can set the keys and pointers of the positions yourself.
		When you use the setId(id, object), the position given by the id will be 
		overwritten with the new object and key. This is meant as a way to mirror the
		IdDealer from somewhere else. Say the id dealer updates, then you can set the new
		id and pointer for that position yourself somewhere else and thereby track the dealer. 
		The IdDealer could be used on server side for example, and the IdTracker could be used 
		on client side.

	The job of the IdStatusTracker is to keep track of which ids you are tracking. If you 
		want to show that you are tracking an id, use set(). When you are no longer tracking
		the id, use unset(). If you call isSet(), you can then get if an id is tracked. 
		There is also a list of the ids you are currently tracking. If a set call has been made,
		subsequent set calls to that id will not change anything unless you call unset. 
*/

const int id_batch_size = 1024;

typedef unsigned __int64 object_id;

template<class OBJECT, class DEALER>
class IdDealer;


namespace id_ {
	template<class OBJECT>
	struct IdBatch {
		std::vector<OBJECT> object;
		std::vector<unsigned __int32> key;
		inline void resize(int size) {
			object.resize(size);
			key.resize(size);
		}
		inline int size() { return object.size(); }
	};
}


template<class OBJECT, class DEALER>
class IdBatchManager {
public:
	IdBatchManager(unsigned __int32 static_id_count);
	IdBatchManager();
	~IdBatchManager();

	// Only works for static ids
	void setId(object_id static_id, OBJECT object);
	void unsetId(object_id static_id) { setId(static_id, 0); }
	OBJECT getObject(object_id static_id);

	bool isStaticId(object_id id);
	DEALER* createDealer();
	DEALER* getResponsibleDealer(object_id id);
private:
	// Get an available batch id
	int m_getAvailableBatchId();

	// Double the size of a dealer
	void assignBatches(DEALER* dealer, int batch_count);

	// Static ids
	std::vector<id_::IdBatch<OBJECT>*> m_static_batch;
	unsigned __int32 static_id_count = 0;

	// For tracking who owns the batches
	std::vector<DEALER*> m_batch;

	// How many batches that are used
	int m_used_batch_count;

	// To avoid thread collisions while altering who owns which batch
	std::mutex m_batch_lock;
	friend IdDealer<OBJECT, DEALER>;
};

template<class OBJECT, class DEALER>
class IdDealer {
public:
	IdDealer();

	// Input an object and get a id
	object_id inputObject(OBJECT object);

	// Get an object by using its id
	OBJECT getObject(object_id id);

	// Remove an object by its id
	bool removeObject(object_id id);

	// See if this class instance is the one that can create this id(does not mean that it has created it)
	bool isValidId(object_id id);

	// Get the number of ids the class can hold at one time, dynamically increases when necessary
	int getSize();

	~IdDealer();
private:
	// List of owned batch numbers
	std::list<int> m_owned_batches;

	// Vector to acces batches
	std::vector<id_::IdBatch<OBJECT>*> m_batch;

	std::list<int> m_available_acces_numbers;
	IdBatchManager<OBJECT, DEALER>* m_manager;
	DEALER* m_child_ptr;
	friend IdBatchManager<OBJECT, DEALER>;
};


// Copy objects from manager and store them in tracker for thread safe acces of objects
template<class OBJECT>
class IdTracker {
public:
	~IdTracker();
	void setId(object_id id, OBJECT object);
	void unsetId(object_id id) { setId(id, 0); }
	OBJECT getObject(object_id id);
private:
	std::vector<id_::IdBatch<OBJECT>*> m_batch;
};

class IdStatusTracker {
public:
	~IdStatusTracker();
	void set(object_id id);
	void unset(object_id id);
	bool isSet(object_id id);
	// Should only be read
	std::list<object_id> set_id_list = {};
private:
	IdTracker<std::list<object_id>::iterator*> m_iterator_tracker;
};

template<class OBJECT>
struct IdAndStatusTracker: public IdTracker<OBJECT>, protected IdStatusTracker {
	void setId(object_id id, OBJECT object) {
		((IdTracker<OBJECT>*)this)->setId(id, object);
		((IdStatusTracker*)this)->set(id);
	}
	void unsetId(object_id id) {
		((IdTracker<OBJECT>*)this)->unsetId(id);
		((IdStatusTracker*)this)->unset(id);
	}
	OBJECT getObject(object_id id) {
		return ((IdTracker<OBJECT>*)this)->getObject(id);
	}

	using IdStatusTracker::set_id_list;
	using IdStatusTracker::isSet;
};

template<class OBJECT, class DEALER>
struct IdDealerAndStatusTracker: public IdDealer<OBJECT, DEALER>, protected IdStatusTracker {
	// Input an object and get a id
	object_id inputObject(OBJECT object) {
		object_id return_id = ((IdDealer<OBJECT, DEALER>*)this)->inputObject(object);
		((IdStatusTracker*)this)->set(return_id);
		return return_id;
	}

	// Remove an object by its id
	bool removeObject(object_id id) {
		((IdStatusTracker*)this)->unset(id);
		return ((IdDealer<OBJECT, DEALER>*)this)->removeObject(id);
	}

	using IdStatusTracker::set_id_list;
	using IdStatusTracker::isSet;
};




// .cpp

namespace id_ {
	inline void insertCheckKey(object_id& id, object_id key) {
		object_id temp = 0x00000000ffffffff;
		id = id & temp;
		key = key << 32;
		id += key;
	}
	inline unsigned __int32 getCheckKey(object_id id) {
		id = id >> 32;
		return (unsigned __int32)id;
	}
	inline void insertAccesNumber(object_id& id, unsigned __int32 num) {
		object_id temp = 0xffffffff00000000;
		id = id & temp;
		id += num;
	}
	inline unsigned __int32 getAccesNumber(object_id id) {
		object_id temp = 0x00000000ffffffff;
		id = id & temp;
		return (unsigned __int32)id;
	}
	inline unsigned __int32 getBatchNumber(unsigned __int32 num) {
		return num / 1024;
	}
	inline unsigned __int32 getNumberInsideBatch(unsigned __int32 num) {
		return num % 1024;
	}
}

template<class OBJECT, class DEALER>
IdBatchManager<OBJECT, DEALER>::IdBatchManager(unsigned __int32 t_static_id_count) {
	using namespace id_;
	static_id_count = t_static_id_count;

	// Setup static batches
	if (static_id_count > 0) {
		m_static_batch.resize((static_id_count - 1) / id_batch_size + 1);
		for (int i = 0; i < m_static_batch.size() - 1; ++i) {
			m_static_batch[i] = new id_::IdBatch<OBJECT>();
			m_static_batch[i]->resize(id_batch_size);
		}
		m_static_batch[m_static_batch.size() - 1] = new id_::IdBatch<OBJECT>();
		m_static_batch[m_static_batch.size() - 1]->resize(((static_id_count - 1) % id_batch_size) + 1);
	}

	m_batch = {};
	m_batch.resize(m_static_batch.size() + 1);
	m_used_batch_count = m_static_batch.size();
}

template<class OBJECT, class DEALER>
IdBatchManager<OBJECT, DEALER>::IdBatchManager() {
	// Setup static batches
	m_static_batch = {};
	m_batch = {};
	m_batch.resize(m_static_batch.size() + 1);
	m_used_batch_count = m_static_batch.size();
}

template<class OBJECT, class DEALER>
IdBatchManager<OBJECT, DEALER>::~IdBatchManager() {
	for (int i = 0; i < m_static_batch.size(); ++i) {
		delete(m_static_batch[i]);
	}
}

template<class OBJECT, class DEALER>
void IdBatchManager<OBJECT, DEALER>::setId(object_id static_id, OBJECT object) {
	using namespace id_;
	m_static_batch[getBatchNumber(static_id)]->object[getNumberInsideBatch(static_id)] = object;
}

template<class OBJECT, class DEALER>
OBJECT IdBatchManager<OBJECT, DEALER>::getObject(object_id static_id) {
	using namespace id_;
	return m_static_batch[getBatchNumber(static_id)]->object[getNumberInsideBatch(static_id)];
}

template<class OBJECT, class DEALER>
bool IdBatchManager<OBJECT, DEALER>::isStaticId(object_id id) {
	if (id < static_id_count) {
		return true;
	}
	return false;
}

template<class OBJECT, class DEALER>
DEALER* IdBatchManager<OBJECT, DEALER>::createDealer() {
	using namespace id_;
	// Create the dealer
	DEALER* new_dealer = new DEALER();
	new_dealer->m_manager = this;
	new_dealer->m_child_ptr = new_dealer;

	// Assign pointer to the static ids
	new_dealer->m_batch.resize(m_static_batch.size());
	for (int i = 0; i < m_static_batch.size(); ++i) {
		new_dealer->m_batch[i] = m_static_batch[i];
	}
	return new_dealer;
}

template<class OBJECT, class DEALER>
DEALER* IdBatchManager<OBJECT, DEALER>::getResponsibleDealer(object_id id) {
	using namespace id_;
	unsigned __int32 acces_number = getAccesNumber(id);
	if (getBatchNumber(acces_number) < m_batch.size()) {
		return m_batch[getBatchNumber(acces_number)];
	}
	return nullptr;
}

template<class OBJECT, class DEALER>
int IdBatchManager<OBJECT, DEALER>::m_getAvailableBatchId() {
	using namespace id_;
	if (m_used_batch_count < m_batch.size()) {
		// Already available batches
	}
	else {
		// Create some new batches

		// Double batch count
		m_batch.resize(m_batch.size() * 2);
	}

	int batch_id = m_used_batch_count;
	m_used_batch_count++;
	return batch_id;
}

template<class OBJECT, class DEALER>
void IdBatchManager<OBJECT, DEALER>::assignBatches(DEALER* dealer, int batch_count) {
	using namespace id_;
	// Avoid thread kollisions
	m_batch_lock.lock();

	// Assign batch ids
	for (int i = 0; i < batch_count; ++i) {
		int new_batch_id = m_getAvailableBatchId();
		dealer->m_owned_batches.push_back(new_batch_id);
		m_batch[new_batch_id] = dealer;
	}

	// Resize dealer to have space for the new batches
	dealer->m_batch.resize(m_used_batch_count);

	m_batch_lock.unlock();
}

template<class OBJECT, class DEALER>
IdDealer<OBJECT, DEALER>::IdDealer() {
	using namespace id_;
	m_owned_batches = {};
	m_batch = {};
	m_available_acces_numbers = {};
	m_manager = nullptr;
	m_child_ptr = nullptr;
}

template<class OBJECT, class DEALER>
object_id IdDealer<OBJECT, DEALER>::inputObject(OBJECT object) {
	using namespace id_;
	unsigned __int32 acces_number;
	if (m_available_acces_numbers.size() > 0) {
		// Acces numbers available
		acces_number = m_available_acces_numbers.front();
		m_available_acces_numbers.pop_front();
	}
	else {
		// Request a new batch from manager
		m_manager->assignBatches(m_child_ptr, 1);
		unsigned int new_batch_id = m_owned_batches.back();
		acces_number = new_batch_id * id_batch_size;
		for (unsigned int i = acces_number + 1; i < acces_number + id_batch_size; ++i) {
			m_available_acces_numbers.push_back(i);
		}

		m_batch[new_batch_id] = new id_::IdBatch<OBJECT>();
		m_batch[new_batch_id]->resize(id_batch_size);
	}

	// Get the batch by the acces number
	id_::IdBatch<OBJECT>* batch = m_batch[getBatchNumber(acces_number)];

	// Set pointer to object
	batch->object[getNumberInsideBatch(acces_number)] = object;

	// Create id
	object_id id = 0;
	insertAccesNumber(id, acces_number);
	insertCheckKey(id, batch->key[getNumberInsideBatch(acces_number)]);
	return id;
}
	
template<class OBJECT, class DEALER>
OBJECT IdDealer<OBJECT, DEALER>::getObject(object_id id) {
	using namespace id_;
	if (!isValidId(id)) {
		log__fatal_error("Invalid ID!");
	}

	unsigned __int32 acces_number = getAccesNumber(id);
	if(m_batch[getBatchNumber(acces_number)]->key[getNumberInsideBatch(acces_number)] == getCheckKey(id)) {
		return m_batch[getBatchNumber(acces_number)]->object[getNumberInsideBatch(acces_number)];
	}
	return nullptr;
}

template<class OBJECT, class DEALER>
bool IdDealer<OBJECT, DEALER>::removeObject(object_id id) {
	using namespace id_;
	if (!isValidId(id)) {
		log__fatal_error("Invalid ID!");
	}

	unsigned __int32 acces_number = getAccesNumber(id);
	if (m_batch[getBatchNumber(acces_number)]->key[getNumberInsideBatch(acces_number)] == getCheckKey(id)) {
		// Ensure the key number doesnt overflow so two can get the same id
		if (getCheckKey(id) != (unsigned __int32)-2) {

			// Make acces_number available to others
			m_available_acces_numbers.push_back(acces_number);
		}
		m_batch[getBatchNumber(acces_number)]->object[getNumberInsideBatch(acces_number)] = nullptr;

		// Make sure nobody else gets the same key
		m_batch[getBatchNumber(acces_number)]->key[getNumberInsideBatch(acces_number)]++;
		return true;
	}
	return false;
}

template<class OBJECT, class DEALER>
bool IdDealer<OBJECT, DEALER>::isValidId(object_id id) {
	using namespace id_;
	unsigned __int32 acces_number = getAccesNumber(id);
	if (m_batch.size() > getBatchNumber(acces_number) &&
		m_batch[getBatchNumber(acces_number)] != nullptr &&
		m_batch[getBatchNumber(acces_number)]->size() > getNumberInsideBatch(acces_number)) {
		return true;
	}
	return false;
}

template<class OBJECT, class DEALER>
int IdDealer<OBJECT, DEALER>::getSize() {
	using namespace id_;
	return m_owned_batches.size() * id_batch_size;
}

template<class OBJECT, class DEALER>
IdDealer<OBJECT, DEALER>::~IdDealer() {
	using namespace id_;
	std::list<int>::iterator it = m_owned_batches.begin();
	while (it != m_owned_batches.end()) {
		delete(m_batch[*it]);
		it++;
	}
}

template<class OBJECT>
IdTracker<OBJECT>::~IdTracker() {
	using namespace id_;
	for (int i = 0; i < m_batch.size(); ++i) {
		if (m_batch[i] != nullptr) {
			delete(m_batch[i]);
		}
	}
}
template<class OBJECT>
void IdTracker<OBJECT>::setId(object_id id, OBJECT object) {
	using namespace id_;
	unsigned __int32 acces_number = getAccesNumber(id);
	if (m_batch.size() <= getBatchNumber(acces_number)) {
		m_batch.resize(getBatchNumber(acces_number) + 1);
		m_batch[getBatchNumber(acces_number)] = new id_::IdBatch<OBJECT>();
		m_batch[getBatchNumber(acces_number)]->resize(id_batch_size);
	}
	else if (m_batch[getBatchNumber(acces_number)] == nullptr) {
		m_batch[getBatchNumber(acces_number)] = new id_::IdBatch<OBJECT>();
		m_batch[getBatchNumber(acces_number)]->resize(id_batch_size);
	}
	m_batch[getBatchNumber(acces_number)]->object[getNumberInsideBatch(acces_number)] = object;
	m_batch[getBatchNumber(acces_number)]->key[getNumberInsideBatch(acces_number)] = getCheckKey(id);
}
	
template<class OBJECT>
OBJECT IdTracker<OBJECT>::getObject(object_id id) {
	using namespace id_;
	unsigned __int32 acces_number = getAccesNumber(id);
	if (m_batch.size() > getBatchNumber(acces_number) &&
		m_batch[getBatchNumber(acces_number)] != nullptr &&
		m_batch[getBatchNumber(acces_number)]->key[getNumberInsideBatch(acces_number)] == getCheckKey(id)) {
		return m_batch[getBatchNumber(acces_number)]->object[getNumberInsideBatch(acces_number)];
	}
	return 0;
}