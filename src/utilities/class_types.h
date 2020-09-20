#pragma once
#include "memory_leak_detector.h"
#include <list>
#include <vector>

/*
implement:
make your class inherit IdClass<your class>
put m_trackThis(this) in your class constructor

info:
all class objects created from your class will get 
an id that can be accesed using getId(). After the
object destructs the id will automatically be available
for new objects that are created.

Using the getRef(id) you can get a pointer to the object 
assosiated with the specified id. In case the id is 0(null)
or a object with given id does not exist yet, it will return
a "undefined pointer", which by default is nullptr.

The "undefined pointer" can be assigned using 
setUndefinedPtr(pointer).
*/
template<typename T>
class IdClass {
public:
	virtual void destructor() = 0;

	int getId() { return m_id; }

	//get how many ids that does not yet belong to any objects
	static size_t getFreeIdCount() { return m_available_ids.size(); }

	//get the maximum amount of ids that can be assigned at one time
	static int getMaxId() { return m_max_id; }

	//increase maximum amount of ids
	static void IncreaseIdCount(unsigned int added_ids_count){
		m_reference.resize(m_reference.size() + added_ids_count);
		while (m_max_id < m_reference.size()) {
			m_available_ids.push_back(m_max_id);
			m_reference[m_max_id] = undefined_ptr;
			m_max_id++;
		}
	}

	//get pointer to object with the specified id
	static T* getRef(int id) { 
		if (id < m_max_id) {
			return m_reference[id];
		}
		else {
			return undefined_ptr;
		}
	}

	//see if a id is associated with an object
	static bool isDefined(int id) {
		if (getRef(id) != undefined_ptr) {
			return true;
		}
		return false;
	}

	//delete every single class objects that inherit IdClass<your class> (including undefined pointer)
	static void deleteAllObjects() {
		for (int i = 1; i < m_max_id; ++i) {
			if (isDefined(i)) {
				T* ref = getRef(i);
				ref->destructor();
			}
		}
		if (undefined_ptr != nullptr) {
			delete(undefined_ptr);
			undefined_ptr = nullptr;
			for (int i = 0; i < m_max_id; ++i) {
				m_reference[i] = nullptr;
			}
		}
	}
protected:
	IdClass() {
		if (m_available_ids.size() != 0) {
			m_id = m_available_ids.front();
			m_available_ids.pop_front();
		}
		else {
			IncreaseIdCount(m_max_id);
			m_id = m_available_ids.front();
			m_available_ids.pop_front();
		}
	}
	~IdClass() {
		m_reference[m_id] = undefined_ptr;
		m_available_ids.push_back(m_id);
	}

	//use this in constructor to make it accesible using id
	void m_trackThis(T* ref) {
		m_reference[m_id] = ref;
	}

	//create child class and put this in constructor to make all undefined ids point here
	void setUndefinedPtr(T* ref) {
		//set all ptrs that are undefined to this
		for (int i = 0; i < m_max_id; ++i) {
			if (m_reference[i] == undefined_ptr) {
				m_reference[i] = ref;
			}
		}
		if (undefined_ptr) {
			delete(undefined_ptr);
		}
		undefined_ptr = ref;

		//push back previusly assigned id since it will now be 0
		m_available_ids.push_back(m_id);
		m_id = 0;
	}
private:

	int m_id;
	static std::list<int> m_available_ids;
	static std::vector<T*> m_reference;
	static int m_max_id;
	static T* undefined_ptr;
};

template<typename T>
int IdClass<T>::m_max_id = 2;
template<typename T>
std::vector<T*> IdClass<T>::m_reference = { nullptr, nullptr };
template<typename T>
std::list<int> IdClass<T>::m_available_ids = { 1 };
template<typename T>
T* IdClass<T>::undefined_ptr = nullptr;







/*
custom list with added functionality

info:
The list and parent class enables objects to track the lists 
they are part of, and automatically remove themself if 
deleted. For objects to track the lists, the list must be
SmartList. 

implementation:
make objects that should be in the list inherit SmartListObject<your class>.
create new lists by creating SmartList<your class> object, where your class are 
the object types it should store.
*/
template <typename T>
class SmartListReference;

template <typename T>
class SmartList;

template <typename T>
class SmartListObject {
public:
	//virtual void destructor() = 0;

	~SmartListObject() {
		typename std::list<SmartListReference<T>*>::iterator it = smart_references.begin();
		while (it != smart_references.end()) {
			SmartListReference<T>* ref = *it;
			it++;
			delete(ref);
		}
	}
private:
	typename std::list<SmartListReference<T>*> smart_references;
	friend SmartListReference<T>;
};

template <typename T>
class SmartListReference {
public:
	T* getObject() { return m_object; }
	~SmartListReference() {
		m_object->smart_references.erase(m_object_it);
		m_smart_list_ref->erase(m_smart_list_it);
	}
private:
	SmartListReference(T* object) {
		m_object = object;
		m_object->smart_references.push_front(this);
		m_object_it = m_object->smart_references.begin();
	}
	T* m_object;
	typename std::list<SmartListReference<T>*>::iterator m_object_it;

	SmartList<T>* m_smart_list;
	typename std::list<SmartListReference<T>*>* m_smart_list_ref;
	typename std::list<SmartListReference<T>*>::iterator m_smart_list_it;

	friend SmartList<T>;
	friend SmartListObject<T>;
};

template <typename T>
class SmartList {
public:
	SmartList() {

	}
	~SmartList() {
		typename std::list<SmartListReference<T>*>::iterator it = m_list_references.begin();
		while (it != m_list_references.end()) {
			SmartListReference<T>* ref = *it;
			it++;
			delete(ref);
		}
	}
	typename std::list<SmartListReference<T>*>::iterator begin() { return m_list_references.begin(); }
	typename std::list<SmartListReference<T>*>::iterator end() { return m_list_references.end(); }
	SmartListReference<T>* pushFront(T* object) {
		SmartListReference<T>* ref = new SmartListReference<T>(object);
		m_pushFront(ref);
		return ref;
	}
	SmartListReference<T>* pushBack(T* object) {
		SmartListReference<T>* ref = new SmartListReference<T>(object);
		m_pushBack(ref);
		return ref;
	}
	int getSize() { return m_list_references.size(); }
	SmartListReference<T>* pushFrontExtract(SmartListReference<T>* ref) {
		ref->m_smart_list_ref->erase(ref->m_smart_list_it);
		m_pushFront(ref);
		return ref;
	}
	SmartListReference<T>* pushBackExtract(SmartListReference<T>* ref) {
		ref->m_smart_list_ref->erase(ref->m_smart_list_it);
		m_pushBack(ref);
		return ref;
	}
private:
	inline void m_pushFront(SmartListReference<T>* ref) {
		ref->m_smart_list = this;
		ref->m_smart_list_ref = &m_list_references;
		m_list_references.push_front(ref);
		ref->m_smart_list_it = m_list_references.begin();
	}
	inline void m_pushBack(SmartListReference<T>* ref) {
		ref->m_smart_list = this;
		ref->m_smart_list_ref = &m_list_references;
		m_list_references.push_back(ref);
		ref->m_smart_list_it = std::prev(m_list_references.end());
	}
	typename std::list<SmartListReference<T>*> m_list_references;
};



template<typename T>
class RectBoundry {
public:
	RectBoundry(T t_x, T t_y, T t_width, T t_height) {
		bound_cent_x = t_x;
		bound_cent_y = t_y;
		width = t_width;
		height = t_height;
	}

	bool isInside(T t_x, T t_y) {
		return(
			(bound_cent_x - t_x) * 2 < width &&
			(t_x - bound_cent_x) * 2 < width &&
			(bound_cent_y - t_y) * 2 < height &&
			(t_y - bound_cent_y) * 2 < height);
	}
	bool isOverlap(RectBoundry<T>* t_boundry) {
		if (abs(t_boundry->bound_cent_x - bound_cent_x)*2 < t_boundry->width + width) {
			if (abs(t_boundry->bound_cent_y - bound_cent_y)*2 < t_boundry->height + height) {
				return true;
			}
		}
		return false;
	}

	T bound_cent_x;
	T bound_cent_y;
	T width;
	T height;
};





/*template <typename T>
class SmartVectorReference;

template <typename T>
class SmartVector;

template <typename T>
class SmartVectorObject {
public:
	~SmartVectorObject() {
		typename std::list<SmartVectorReference<T>*>::iterator it = smart_references.begin();
		while (it != smart_references.end()) {
			SmartVectorReference<T>* ref = *it;
			it++;
			delete(ref);
		}
	}
private:
	typename std::list<SmartVectorReference<T>*> m_list_references;
	friend SmartVectorReference<T>;
};

template <typename T>
class SmartVectorReference {
public:
	T* getObject() { return m_object; }
	~SmartListReference() {
	}
private:
	SmartListReference(T* object) {
		m_object = object;
		m_object->smart_references.push_front(this);
		m_object_it = m_object->smart_references.begin();
	}
	T* m_object;
	typename std::list<SmartListReference<T>*>::iterator m_object_it;

	SmartVector<T>* m_smart_vector;
	int m_num;

	friend SmartVector<T>;
	friend SmartVectorObject<T>;
};

template <typename T>
class SmartVector {
	SmartVector(int size) {

	}
private:

};*/