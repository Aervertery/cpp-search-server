#include "remove_duplicates.h"

void RemoveDuplicates(SearchServer& search_server) {
	std::set<int> ids_to_remove;
	int count = 1;
	for (const int it : search_server) {
		if (*prev(search_server.end()) == it) {
			break;
		}
		auto test1 = search_server.GetWordFrequencies(it);
		for (auto it_ = next(search_server.begin(), count++); it_ != search_server.end(); ++it_) {
			auto test2 = search_server.GetWordFrequencies(*it_);
			if (key_compare(test1, test2)) {
				if (*it_ > it) {
					ids_to_remove.emplace(*it_);
					break;
				}
				else {
					ids_to_remove.emplace(it);
					break;
				}
			}
		}
	}
	for (int id : ids_to_remove) {
		std::cout << "Found duplicate document id " << id << std::endl;
			search_server.RemoveDocument(id);
	}
}