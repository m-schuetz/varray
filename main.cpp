
#include <windows.h>
#include <print>
#include <locale>
#include <chrono>

using namespace std;


// A "virtual" array that reserves copious amounts of virtual memory in advance,
// and then commits as much physical memory as needed as new elements as inserted.
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






constexpr size_t NUM_ELEMENTS = 10'000'000;

static long long start_time = chrono::high_resolution_clock::now().time_since_epoch().count();
auto l = std::locale("en_US.UTF-8");

inline double now() {
	auto now = std::chrono::high_resolution_clock::now();
	long long nanosSinceStart = now.time_since_epoch().count() - start_time;

	double secondsSinceStart = double(nanosSinceStart) / 1'000'000'000.0;

	return secondsSinceStart;
}






void testVarray() {
	double t_start = now();

	varray<int> numbers;
	for(int i = 0; i < NUM_ELEMENTS; i++){
		numbers.push(i);
	}

	float duration_create = now() - t_start;
	
	t_start = now();

	size_t sum = 0;
	for (int i = 0; i < numbers.count; i++) {
		sum += numbers[i];
	}

	float duration_iterate = now() - t_start;

	string str = format(l, "[varray                   ] creation: {:.3f} s, iteration: {:.3f} s, checksum: {:L}", duration_create, duration_iterate, sum);
	println("{}", str);
}

void testVector() {
	double t_start = now();

	vector<int> numbers;
	for(int i = 0; i < NUM_ELEMENTS; i++){
		numbers.push_back(i);
	}

	float duration_create = now() - t_start;
	
	t_start = now();
	size_t sum = 0;
	for (int i = 0; i < numbers.size(); i++) {
		sum += numbers[i];
	}

	float duration_iterate = now() - t_start;

	string str = format(l, "[std::vector              ] creation: {:.3f} s, iteration: {:.3f} s, checksum: {:L}", duration_create, duration_iterate, sum);
	println("{}", str);
}

void testVectorWithCapacity() {
	double t_start = now();

	vector<int> numbers;
	numbers.reserve(NUM_ELEMENTS);
	for(int i = 0; i < NUM_ELEMENTS; i++){
		numbers.push_back(i);
	}

	float duration_create = now() - t_start;
	
	t_start = now();
	size_t sum = 0;
	for (int i = 0; i < numbers.size(); i++) {
		sum += numbers[i];
	}

	float duration_iterate = now() - t_start;

	string str = format(l, "[std::vector with capacity] creation: {:.3f} s, iteration: {:.3f} s, checksum: {:L}", duration_create, duration_iterate, sum);
	println("{}", str);
}

void testMalloc() {
	double t_start = now();

	int* numbers = (int*)malloc(NUM_ELEMENTS * sizeof(int));
	
	for(int i = 0; i < NUM_ELEMENTS; i++){
		numbers[i] = i;
	}

	float duration_create = now() - t_start;
	
	t_start = now();
	size_t sum = 0;
	for (int i = 0; i < NUM_ELEMENTS; i++) {
		sum += numbers[i];
	}

	float duration_iterate = now() - t_start;

	string str = format(l, "[malloc                   ] creation: {:.3f} s, iteration: {:.3f} s, checksum: {:L}", duration_create, duration_iterate, sum);
	println("{}", str);

	free(numbers);
}

int main() {

	int repetitions = 5;

	println("Testing a virtual-memory-based dynamically growable 'varray', which does not require allocation&copy on grow.");
	println("- 400GB virtual memory is reserved when the array is created.");
	println("- Physical memory is comitted as needed. Whenever it is exhausted, comitted memory is doubled.");
	println("");

	println("Each benchmark first adds {} items to the array, and then computes the sum of the added items.", format(l, "{:L}", NUM_ELEMENTS));
	println("");

	println("Benchmarking virtual array, without initial capacity, growing on-demand.");
	for(int i = 0; i < repetitions; i++){
		testVarray();
	}
	println("");

	println("Benchmarking std::vector, without initial capacity, growing on-demand.");
	for(int i = 0; i < repetitions; i++){
		testVector();
	}
	println("");

	println("Benchmarking std::vector with initialized capacity");
	for(int i = 0; i < repetitions; i++){
		testVectorWithCapacity();
	}
	println("");

	println("Benchmarking malloc with initialized capacity");
	for(int i = 0; i < repetitions; i++){
		testMalloc();
	}

	return 0;
}

