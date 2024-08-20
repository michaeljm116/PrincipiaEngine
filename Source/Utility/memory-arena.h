#pragma once
#include <vector>
#include <assert.h>

namespace Principia {

	/*
		This will be for the Animation system
		At it's core is a double vector
		You can add and delete vectors
		When one is removed, the spot is shifted
		And the calling parents are informed
	*/

	template <typename T>
	class SmallArena {
		std::vector<T> memory = {};
		size_t max_size = 32;
	public:
		explicit SmallArena(size_t max_size) : max_size(max_size) {
			memory.reserve(max_size);
		};
		~SmallArena() {};

		T* allocate(const T& object) {
			// Add if there's room
			assert(memory.size() < max_size);
			memory.emplace_back(object);
			return memory.data() + memory.size() - 1;
		};

		T* allocate(const std::vector<T>& object)
		{
			// Add if there's room
			assert(memory.size() + object.size() < max_size);
			memory.insert(memory.end(), object.begin(), object.end());
			return memory.data() + memory.size() - object.size();
		};

		void deallocate(size_t index) {
			assert(index < memory.size());
			// For every object after the index, shift it to the left
			for (auto it = memory.begin() + index; it != memory.end(); it++) {
				assert((*it).parent != nullptr);
				(*it).parent->child_index--;
			}
			// Then delete the object
			memory.erase(memory.begin() + index);
		};

		void deallocate(size_t index, size_t count) {
			assert(index < memory.size());
			// For every object after the index, shift it to the left
			for (auto it = memory.begin() + index + count; it != memory.end(); it++) {
				assert((*it).parent != nullptr);
				(*it).parent->child_index -= count;
			}
			// Then delete the object
			memory.erase(memory.begin() + index, memory.begin() + index + count);
		};

		auto begin() const {
			return memory.begin();
		};

		auto end() const {
			return memory.end();
		};

		size_t size() const {
			return memory.size();
		};

	};
}