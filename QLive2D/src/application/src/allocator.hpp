#ifndef ALLOCATOR_H
#define ALLOCATOR_H
#include <CubismFramework.hpp>
#include <ICubismAllocator.hpp>
#include <memory>
#include <cstdlib>
class Allocator:
        public Csm::ICubismAllocator,
        private std::allocator<std::byte>
{
public:
    inline void* Allocate(const Csm::csmSizeType size){
        return malloc(size);
    }
    inline void Deallocate(void* memory){
        free(memory);
    }
    inline void* AllocateAligned(const Csm::csmSizeType size, const Csm::csmUint32 alignment){
        size_t realsize=((size-1)/alignment+1)*alignment;
        return _aligned_malloc(realsize,alignment);
    }
    inline void DeallocateAligned(void* alignedMemory){
        _aligned_free(alignedMemory);
    }
};

#endif // ALLOCATOR_H
