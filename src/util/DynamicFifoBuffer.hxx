/*
 * Copyright (C) 2003-2013 Max Kellermann <max@duempel.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the
 * distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * FOUNDATION OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef FIFO_BUFFER_HPP
#define FIFO_BUFFER_HPP

#include "ForeignFifoBuffer.hxx"

/**
 * A first-in-first-out buffer: you can append data at the end, and
 * read data from the beginning.  This class automatically shifts the
 * buffer as needed.  It is not thread safe.
 */
template<typename T>
class DynamicFifoBuffer : protected ForeignFifoBuffer<T> {
public:
	typedef typename ForeignFifoBuffer<T>::size_type size_type;
	typedef typename ForeignFifoBuffer<T>::pointer_type pointer_type;
	typedef typename ForeignFifoBuffer<T>::const_pointer_type const_pointer_type;
	typedef typename ForeignFifoBuffer<T>::Range Range;

	explicit DynamicFifoBuffer(size_type _capacity)
		:ForeignFifoBuffer<T>(new T[_capacity], _capacity) {}
	~DynamicFifoBuffer() {
		delete[] GetBuffer();
	}

	DynamicFifoBuffer(const DynamicFifoBuffer &) = delete;

	using ForeignFifoBuffer<T>::GetCapacity;
	using ForeignFifoBuffer<T>::Clear;
	using ForeignFifoBuffer<T>::IsEmpty;
	using ForeignFifoBuffer<T>::IsFull;
	using ForeignFifoBuffer<T>::GetAvailable;
	using ForeignFifoBuffer<T>::Read;
	using ForeignFifoBuffer<T>::Consume;
	using ForeignFifoBuffer<T>::Write;
	using ForeignFifoBuffer<T>::Append;

	void Grow(size_type new_capacity) {
		assert(new_capacity > GetCapacity());

		T *old_data = GetBuffer();
		T *new_data = new T[new_capacity];
		ForeignFifoBuffer<T>::MoveBuffer(new_data, new_capacity);
		delete[] old_data;
	}

	void WantWrite(size_type n) {
		if (ForeignFifoBuffer<T>::WantWrite(n))
			/* we already have enough space */
			return;

		const size_type in_use = GetAvailable();
		const size_type required_capacity = in_use + n;
		size_type new_capacity = GetCapacity();
		do {
			new_capacity <<= 1;
		} while (new_capacity < required_capacity);

		Grow(new_capacity);
	}

	/**
	 * Write data to the buffer, growing it as needed.  Returns a
	 * writable pointer.
	 */
	pointer_type Write(size_type n) {
		WantWrite(n);
		return Write().data;
	}

	/**
	 * Append data to the buffer, growing it as needed.
	 */
	void Append(const_pointer_type p, size_type n) {
		std::copy_n(p, n, Write(n));
		Append(n);
	}

protected:
	using ForeignFifoBuffer<T>::GetBuffer;
};

#endif