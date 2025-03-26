# Virtual Array

Testing a dynamically growable array, using virtual memory.

- No realloc/copy. The initially constructed pointer stays the same as the array grows. 
- 400 GB virtual memory is reserved on array creation. 
- Physical memory is comitted on demand. 4kb (page size) at minimum, and then doubled whenever physical capacity is exhausted.
- Windows only, implemented via WinAPI's VirtualAlloc().

### Issues/Limitations

- Quick test that is not meant to be production ready. 
- Reserving 400 GB virtual mem for a contiguous array is pointless on hardware with only 32 GB RAM, and you likely can't create too many arrays since even virtual memory is not infinite. 
- Ideally we would reserve a smaller amount of virtual memory, and also grow that as needed. This can be done in CUDA by trying to increase the virtual address range, and if that fails, request an entirely new virtual address range and then remapping the physical memory to the new virtual memory, without actually having to copy the memory. Not sure if that can be done with the WinAPI, but functions like AllocateUserPhysicalPages sound like it could be possible. 

### Benchmark

- Windows 11, AMD Ryzen 9 7950X.
- Each benchmark first adds 10,000,000 items to the array, and then computes the sum of the added items.

```
Benchmarking virtual array, without initial capacity, growing on-demand.
[varray                   ] creation: 0.007 s, iteration: 0.001 s, checksum: 49,999,995,000,000
[varray                   ] creation: 0.007 s, iteration: 0.001 s, checksum: 49,999,995,000,000
[varray                   ] creation: 0.007 s, iteration: 0.001 s, checksum: 49,999,995,000,000
[varray                   ] creation: 0.007 s, iteration: 0.001 s, checksum: 49,999,995,000,000
[varray                   ] creation: 0.007 s, iteration: 0.001 s, checksum: 49,999,995,000,000

Benchmarking std::vector, without initial capacity, growing on-demand.
[std::vector              ] creation: 0.032 s, iteration: 0.004 s, checksum: 49,999,995,000,000
[std::vector              ] creation: 0.026 s, iteration: 0.004 s, checksum: 49,999,995,000,000
[std::vector              ] creation: 0.025 s, iteration: 0.004 s, checksum: 49,999,995,000,000
[std::vector              ] creation: 0.024 s, iteration: 0.004 s, checksum: 49,999,995,000,000
[std::vector              ] creation: 0.024 s, iteration: 0.004 s, checksum: 49,999,995,000,000

Benchmarking std::vector with initialized capacity
[std::vector with capacity] creation: 0.011 s, iteration: 0.004 s, checksum: 49,999,995,000,000
[std::vector with capacity] creation: 0.011 s, iteration: 0.004 s, checksum: 49,999,995,000,000
[std::vector with capacity] creation: 0.011 s, iteration: 0.004 s, checksum: 49,999,995,000,000
[std::vector with capacity] creation: 0.011 s, iteration: 0.004 s, checksum: 49,999,995,000,000
[std::vector with capacity] creation: 0.011 s, iteration: 0.004 s, checksum: 49,999,995,000,000

Benchmarking malloc with initialized capacity
[malloc                   ] creation: 0.005 s, iteration: 0.001 s, checksum: 49,999,995,000,000
[malloc                   ] creation: 0.005 s, iteration: 0.001 s, checksum: 49,999,995,000,000
[malloc                   ] creation: 0.005 s, iteration: 0.001 s, checksum: 49,999,995,000,000
[malloc                   ] creation: 0.005 s, iteration: 0.001 s, checksum: 49,999,995,000,000
[malloc                   ] creation: 0.005 s, iteration: 0.001 s, checksum: 49,999,995,000,000
```

### Code


```C++
template<typename T>
struct varray{
	void* ptr = nullptr;                                  // pointer to the reserved address range
	size_t pageSize = 4096;                               // Assuming a typical small page size of 4kb
	size_t virtualCapacity = 100'000'000 * pageSize;      // reserved virtual memory, hardcoded to 400GB
	size_t physicalCapacity = 0;                          // The amount of actually used physical memory
	size_t count = 0;                                     // Number of inserted eleements
	
	varray(){
		// Reserve <virtualCapacity> virtual memory
		ptr = VirtualAlloc(NULL, virtualCapacity, MEM_RESERVE, PAGE_READWRITE);
	}

	~varray(){
		VirtualFree(ptr, 0, MEM_RELEASE);
	}

	void push(T value){

		size_t required = (count + 1) * sizeof(T);
		if(physicalCapacity < required){
			raiseCapacity();
		}

		reinterpret_cast<T*>(ptr)[count] = value;
		count++;
	}

	void raiseCapacity(){

		size_t newCapacity = 2 * physicalCapacity;

		if (newCapacity == 0) {
			newCapacity = pageSize;
		}

		// commit <newCapacity> physical memory
		VirtualAlloc(ptr, newCapacity, MEM_COMMIT, PAGE_READWRITE);

		physicalCapacity = newCapacity;
	}

	T& operator[](size_t index) {
		return reinterpret_cast<T*>(ptr)[index];
	}
};
```

### Compile

Create visual studio solution files:

```
mkdir build
cd build
cmake ..
```

Then compile and run in visual studio.