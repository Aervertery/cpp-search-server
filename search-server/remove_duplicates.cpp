#include "remove_duplicates.h"

void RemoveDuplicates(SearchServer& search_server) {
	std::set<int> ids_to_remove;
	std::map<std::set<std::string>, int> words_to_id;
	for (const int it : search_server) {
		std::set<std::string> words;
		for (const auto& [key, value] : search_server.GetWordFrequencies(it)) {
			words.emplace(key);
		}
		auto it_ = words_to_id.find(words);
		if (it_ == words_to_id.end()) {
			words_to_id[words] = it;
		}
		else {
			(it < it_->second) ? ids_to_remove.emplace(it_->second) : ids_to_remove.emplace(it);
		}
	}
	for (int id : ids_to_remove) {
		std::cout << "Found duplicate document id " << id << std::endl;
		search_server.RemoveDocument(id);
	}
}