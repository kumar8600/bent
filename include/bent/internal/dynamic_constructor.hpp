#pragma once

namespace bent
{
    struct DynamicConstructorInterface
    {
        virtual ~DynamicConstructorInterface() = default;

        virtual void CopyConstruct(void* p, const void* src) = 0;
        virtual void MoveConstruct(void* p, void* src) = 0;
        virtual void Destroy(void* p) = 0;
    };

    template<typename T>
    struct DynamicConstructor : DynamicConstructorInterface
    {
        virtual void CopyConstruct(void* p, const void* src) override
        {
            const T& ref = *static_cast<const T*>(src);
            new (p) T(ref);
        }

        virtual void MoveConstruct(void* p, void* src) override
        {
            T& ref = *static_cast<T*>(src);
            new (p) T(std::move(ref));
        }

        virtual void Destroy(void* p) override
        {
            static_cast<T*>(p)->~T();
        }
    };
}
