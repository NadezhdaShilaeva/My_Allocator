#include <iostream>
#include <cstdlib>

struct memory_chunk
{
	size_t size_mem = 0;
	void *memory = nullptr;
	memory_chunk *next_mem = nullptr;
};

struct memory_blocks
{
	size_t size_mem = 0;
	memory_chunk *first_mem = nullptr;
};

void *get(memory_blocks &memory_list, memory_chunk *&free_block)
{
	if (memory_list.first_mem == nullptr)
	{
		return nullptr;
	}

	void *memory = memory_list.first_mem->memory;

	memory_chunk *mem_chunk = memory_list.first_mem;
	memory_list.first_mem = memory_list.first_mem->next_mem;

	mem_chunk->next_mem = free_block;
	free_block = mem_chunk;

	return memory;
}

void insert(void *memory, memory_chunk *&free_block, size_t count_memory_lists, memory_blocks *&memory_lists)
{
	memory_chunk *free_chunk = free_block;
	while (free_chunk->memory != memory)
	{
		free_chunk = free_chunk->next_mem;
	}

	for (int i = 0; i < count_memory_lists; ++i)
	{
		if (memory_lists[i].size_mem == free_chunk->size_mem)
		{
			memory_chunk **new_chunk = &free_block;
			if ((*new_chunk)->memory == free_chunk->memory)
			{
				free_block = free_block->next_mem;
			}
			else
			{
				while ((*new_chunk)->next_mem->memory != free_chunk->memory)
				{
					new_chunk = &((*new_chunk)->next_mem);
				}
				(*new_chunk)->next_mem = (*new_chunk)->next_mem->next_mem;
			}

			free_chunk->next_mem = memory_lists[i].first_mem;
			memory_lists[i].first_mem = free_chunk;

			return;
		}
	}
}

void delete_chunks(memory_chunk *mem_chunk)
{
	if (mem_chunk == nullptr)
	{
		return;
	}
	delete_chunks(mem_chunk->next_mem);
	delete mem_chunk;
}

template <typename T, int... Args>
class CMyallocator
{
public:
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef T *pointer;
	typedef const T *const_pointer;
	typedef T &reference;
	typedef const T &const_reference;
	typedef T value_type;

	CMyallocator()
	{
		int args[sizeof...(Args)] = {Args...};
		construct_lists(sizeof...(Args), args);
		construct_blocks(sizeof...(Args), args);
	}

	~CMyallocator()
	{
		if (memory_pool_ != nullptr)
			std::free(memory_pool_);
		memory_pool_ = nullptr;

		delete_chunks(free_blocks_);
		free_blocks_ = nullptr;

		for (int i = 0; i < count_memory_lists_; ++i)
		{
			delete_chunks(memory_lists_[i].first_mem);
		}
		delete[] memory_lists_;
	}

	CMyallocator(const CMyallocator<T, Args...> &other)
	{
		size_memory_pool_ = other.size_memory_pool_;
		memory_pool_ = std::malloc(size_memory_pool_);

		count_memory_lists_ = other.count_memory_lists_;
		memory_lists_ = new memory_blocks[count_memory_lists_];

		size_t memory_offset = 0;

		for (int i = 0; i < count_memory_lists_; ++i)
		{
			memory_lists_[i].size_mem = other.memory_lists_[i].size_mem;
			memory_lists_[i].first_mem = nullptr;
			memory_chunk *next_mem = other.memory_lists_[i].first_mem;

			while (next_mem != nullptr)
			{
				memory_chunk *new_mem = new memory_chunk;
				new_mem->size_mem = next_mem->size_mem;
				new_mem->memory = static_cast<void *>(static_cast<char *>(memory_pool_) + memory_offset);
				new_mem->next_mem = memory_lists_[i].first_mem;
				memory_lists_[i].first_mem = new_mem;
				memory_offset += new_mem->size_mem;
				next_mem = next_mem->next_mem;
			}
		}

		free_blocks_ = nullptr;
		memory_chunk *next_mem = other.free_blocks_;

		while (next_mem != nullptr)
		{
			memory_chunk *new_mem = new memory_chunk;
			new_mem->size_mem = next_mem->size_mem;
			new_mem->memory = static_cast<void *>(static_cast<char *>(memory_pool_) + memory_offset);
			new_mem->next_mem = free_blocks_;
			free_blocks_ = new_mem;
			memory_offset += new_mem->size_mem;
			next_mem = next_mem->next_mem;
		}
	}

	template <typename U>
	struct rebind
	{
		typedef CMyallocator<U, Args...> other;
	};

	pointer allocate(size_type n)
	{
		for (int i = 0; i < count_memory_lists_; ++i)
		{
			if (memory_lists_[i].size_mem >= n * sizeof(value_type))
			{
				pointer mem = static_cast<pointer>(get(memory_lists_[i], free_blocks_));
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
		insert(static_cast<void *>(p), free_blocks_, count_memory_lists_, memory_lists_);
	}

	template <typename U, typename... Argss>
	void construct(U *p, Argss &&...argss)
	{
		new (reinterpret_cast<void *>(p)) U{std::forward<Argss>(argss)...};
	}

	template <typename U>
	void destroy(U *p)
	{
		p->~U();
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
				size_chunk = args[i];
			}
			else
			{
				size_memory += size_chunk * args[i];
			}
		}

		memory_pool_ = std::malloc(size_memory);
		size_memory_pool_ = size_memory;

		memory_lists_ = new memory_blocks[cnt / 2];
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
				size_chunk = args[i];
			}
			else
			{
				memory_lists_[i / 2].size_mem = size_chunk;
				memory_lists_[i / 2].first_mem = nullptr;

				for (int j = 0; j < args[i]; ++j)
				{
					memory_chunk *new_mem = new memory_chunk;
					new_mem->size_mem = size_chunk;
					new_mem->memory = static_cast<void *>(static_cast<char *>(memory_pool_) + memory_offset);
					new_mem->next_mem = memory_lists_[i / 2].first_mem;
					memory_lists_[i / 2].first_mem = new_mem;
					memory_offset += size_chunk;
				}
			}
		}

		free_blocks_ = nullptr;
	}

public:
	void *memory_pool_;
	size_t size_memory_pool_;
	size_t count_memory_lists_;
	memory_blocks *memory_lists_;
	memory_chunk *free_blocks_;
};

template <typename T, typename U, int... Args>
bool operator==(const CMyallocator<T, Args...> &, const CMyallocator<U, Args...> &)
{
	return true;
}

template <class T, class U, int... Args>
bool operator!=(const CMyallocator<T, Args...> &, const CMyallocator<U, Args...> &)
{
	return false;
}