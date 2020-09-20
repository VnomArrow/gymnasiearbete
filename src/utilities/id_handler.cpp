#include "id_handler.h"

IdStatusTracker::~IdStatusTracker() {
	while (set_id_list.size()) {
		unset(set_id_list.front());
	}
}

void IdStatusTracker::set(object_id id) {
	if (!isSet(id)) {
		set_id_list.push_front(id);
		std::list<object_id>::iterator new_begin = set_id_list.begin();

		// Create a new iterator
		std::list<object_id>::iterator* new_begin_ref =
			(std::list<object_id>::iterator*) malloc(sizeof(std::list<object_id>::iterator));
		std::memcpy(new_begin_ref, &new_begin, sizeof(std::list<object_id>::iterator));

		m_iterator_tracker.setId(id, new_begin_ref);
	}
}

void IdStatusTracker::unset(object_id id) {
	if (isSet(id)) {
		set_id_list.erase(*m_iterator_tracker.getObject(id));
		free(m_iterator_tracker.getObject(id));
		m_iterator_tracker.setId(id, nullptr);
	}
}

bool IdStatusTracker::isSet(object_id id) {
	return(m_iterator_tracker.getObject(id) != nullptr);
}