#include <iostream>
#include <cstdlib>
#include <cstdarg>

template<typename T>
struct memory_chunk
{
	T* memory = nullptr;
	memory_chunk<T>* next_mem = nullptr;
};

template<typename T>
struct memory_blocks
{
	void insert(T* memory)
	{
		memory_chunk<T>* new_mem = free_node;
		free_node = free_node->next_mem;
		new_mem->next_mem = first_mem;
		new_mem->memory = memory;
		first_mem = new_mem;
	}

	T* get()
	{
		if (first_mem == nullptr)
		{
			return nullptr;
		}

		T* memory = first_mem->memory;
		memory_chunk<T>* mem_chunk = first_mem;
		first_mem = first_mem->next_mem;
		if (free_node == nullptr)
		{
			free_node = mem_chunk;
		}
		else
		{
			mem_chunk->next_mem = free_node;
			free_node = mem_chunk;
		}
		free_node->memory = nullptr;

		return memory;
	}

	size_t size_mem;
	memory_chunk<T>* first_mem = nullptr;
	memory_chunk<T>* free_node = nullptr;
};


template<typename T, int... Args>
class CMyallocator
{
public:
	using type = T;
	using value_type = T;
	using pointer = T*;
	using const_pointer = const T*;
	using reference = T&;
	using const_reference = const T&;
	using size_type = size_t;
	using difference_type = ptrdiff_t;

	CMyallocator()
	{
		int args[sizeof...(Args)] = { Args... };
		construct_lists(sizeof...(Args), args);
		construct_blocks(sizeof...(Args), args);
	}

	~CMyallocator()
	{
		std::free(memory_pool_);
		memory_pool_ = nullptr;
	}

	pointer allocate(size_type n)
	{
		for (int i = 0; i < count_memory_lists_; ++i)
		{
			if (memory_lists_[i].size_mem == n)
			{
				pointer mem = memory_lists_[i].get();
				if (mem != nullptr)
				{
					return mem;
				}
			}
		}
		
		throw std::bad_alloc();
	}

	void deallocate(pointer p, size_type n)
	{
		if (p < memory_pool_ || p >= memory_pool_ + size_memory_pool_)
		{
			// error
		}
		for (int i = 0; i < count_memory_lists_; ++i)
		{
			if (memory_lists_[i].size_mem == n)
			{
				memory_lists_[i].insert(p);
				return;
			}
		}
	}

private:
	void construct_lists(int cnt, int args[])
	{
		size_t size_chunk;
		size_t size_memory = 0;

		for (int i = 0; i < cnt; ++i)
		{
			if (i % 2 == 0)
			{
				size_chunk = args[i] / sizeof(T);
			}
			else
			{
				size_memory += size_chunk * args[i];
			}
		}

		memory_pool_ = static_cast<pointer>(std::malloc(size_memory * sizeof(T)));
		size_memory_pool_ = size_memory;
		memory_lists_ = new memory_blocks<T>[cnt / 2];
		count_memory_lists_ = cnt / 2;
	}

	void construct_blocks(int cnt, int args[]) 
	{
		size_t size_chunk;
		size_t memory_offset = 0;

		for (int i = 0; i < cnt; ++i)
		{
			if (i % 2 == 0)
			{
				size_chunk = args[i] / sizeof(T);
			}
			else
			{
				memory_lists_[i / 2].size_mem = size_chunk;
				memory_chunk<T>* first_mem = new memory_chunk<T>;
				first_mem->next_mem = nullptr;
				first_mem->memory = memory_pool_ + memory_offset;
				memory_lists_[i / 2].first_mem = first_mem;
				memory_chunk<T>* pred_mem = first_mem;
				memory_offset += size_chunk;
				for (int j = 0; j < args[i]; ++j)
				{
					memory_chunk<T>* new_mem = new memory_chunk<T>;
					new_mem->next_mem = nullptr;
					new_mem->memory = memory_pool_ + memory_offset;
					pred_mem->next_mem = new_mem;
					pred_mem = new_mem;
					memory_offset += size_chunk / sizeof(T);
				}
			}
		}
	}

private:
	T* memory_pool_;
	size_t size_memory_pool_;
	size_t count_memory_lists_;
	memory_blocks<T>* memory_lists_;
};