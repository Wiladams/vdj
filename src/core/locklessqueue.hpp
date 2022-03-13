#pragma once

//
// Implementation of a lock-less multi-reader, single-writer queue
// fixed length ring buffer
// T is a copy assignable type
// N is a power of two
//
// Reference: https://codereview.stackexchange.com/questions/44896/lockless-queue-multiple-reader-singler-writer-in-c
// https://laptrinhx.com/c-lock-free-queue-1644523275/#:~:text=C%2B%2B%20lock-free%20queue.%201%20atomic_queue.%20C%2B%2B11%20multiple-producer-multiple-consumer%20lockless,Notes.%20...%205%20Benchmarks.%20...%206%20Contributing.%20
//

#include <atomic>
#include <memory>

namespace vdj {
	constexpr ptrdiff_t CACHE_LINE_SIZE = 64;

	//
	// This FIFO queue is implemented as a circular buffer.  This is good
	// as the size is constrained.
	// This queue supports a single writer, and multiple readers.
	// This could be used as the basis of a pub/sub system, but typically
	// the reader would be singular, and turn that into something else.
	// It is good for work stealing out of the box.
	template <typename T, size_t N>
	struct SingleWriterFifo
	{
	private:
		static constexpr uint32_t MASK = 2 * N - 1;
		
		// use alignment to ensure these two don't
		// end up on the same cache line
		alignas(CACHE_LINE_SIZE) std::atomic<uint32_t> m_read;
		alignas(CACHE_LINE_SIZE) std::atomic<uint32_t> m_write;

		alignas(CACHE_LINE_SIZE) std::unique_ptr<T[]> m_array;


	public:
		SingleWriterFifo()
			:m_array(new T[N]),
			m_read(0),
			m_write(0)
		{
			// Some static constraints on template parameters
			static_assert(std::is_default_constructible<T>::value, "T does not have a default constructor");
			static_assert(std::is_copy_assignable<T>::value, "T does not support copy assignment");
			static_assert(N != 0, "N is too small.");
			static_assert((N & (N - 1)) == 0, "N is not a power of two");
		}

		// One thread can write at a time
		bool write(T t)
		{
			// check for empty
			if (m_write - m_read == N)
				return false;

			// Automatically circle around the ring buffer
			m_array[m_write & MASK] = t;

			m_write++;
			return true;
		}

		// multiple readers
		bool read(T& t)
		{
			while (true)
			{
				uint32_t read = m_read;

				// check for empty
				if (read == m_write)
					return false;

				//if (std::atomic_compare_exchange_strong(&m_read, read, read + 1))
				if (m_read.compare_exchange_strong(read, read+1))
				{
					t = m_array[read & MASK];
					return true;
				}
			}
		}

	private:

	};

}