# 07 – Memory Model

## 1. Design Principles

- No global `operator new` override – the library never touches the global
  heap directly.
- All allocation goes through an **Allocator concept** parameter on the
  library policy type.
- A built-in **ArenaAllocator** is provided for bare-metal targets with no OS
  heap.
- Smart pointer wrappers (`lv::UniquePtr<T>`, `lv::SharedPtr<T>`) respect the
  injected allocator.
- Widget trees are **uniquely owned** by their parent; no shared ownership is
  imposed by the framework.

## 2. Allocator Concept

```cpp
namespace lv {

template<typename A>
concept LvAllocator = requires(A alloc,
                               std::size_t n, std::size_t align,
                               void* p) {
    { alloc.allocate(n, align)  } -> std::same_as<void*>;
    { alloc.deallocate(p, n)    } -> std::same_as<void>;
    { alloc.reallocate(p, n, n) } -> std::same_as<void*>;
};

} // namespace lv
```

## 3. Built-in Allocators

### 3.1 SystemAllocator (default)

Delegates to `std::malloc` / `std::free` / `std::realloc`.  Suitable for
Linux, macOS, Windows, and MCUs with a libc heap.

```cpp
namespace lv {

struct SystemAllocator {
    void*  allocate  (std::size_t n, std::size_t align = alignof(std::max_align_t));
    void   deallocate(void* p, std::size_t n);
    void*  reallocate(void* p, std::size_t old_n, std::size_t new_n);
};
static_assert(LvAllocator<SystemAllocator>);

} // namespace lv
```

### 3.2 ArenaAllocator\<N\>

A bump-pointer arena that never frees individual allocations; the entire arena
is reset at once.  Ideal for screen-lifetime or frame-lifetime allocations.

```cpp
namespace lv {

template<std::size_t N, std::size_t Align = alignof(std::max_align_t)>
class ArenaAllocator {
public:
    constexpr ArenaAllocator() noexcept;

    void*  allocate  (std::size_t n, std::size_t align = Align);
    void   deallocate(void* /*p*/, std::size_t /*n*/) noexcept {} // no-op
    void*  reallocate(void* p, std::size_t old_n, std::size_t new_n);

    void   reset()      noexcept;  // free all in one shot
    [[nodiscard]] std::size_t used()      const noexcept;
    [[nodiscard]] std::size_t available() const noexcept;

private:
    alignas(Align) std::byte storage_[N];
    std::size_t               cursor_{0};
};
static_assert(LvAllocator<ArenaAllocator<1024>>);

} // namespace lv
```

### 3.3 PoolAllocator\<T, N\>

Fixed-size object pool for types allocated in high frequency (e.g.,
animation descriptors, draw tasks, event descriptors):

```cpp
namespace lv {

template<typename T, std::size_t N>
class PoolAllocator {
public:
    void*  allocate  (std::size_t n, std::size_t align = alignof(T));
    void   deallocate(void* p, std::size_t n) noexcept;
    void*  reallocate(void* p, std::size_t old_n, std::size_t new_n); // falls back to system

    [[nodiscard]] std::size_t free_slots() const noexcept;

private:
    std::array<std::aligned_storage_t<sizeof(T), alignof(T)>, N> pool_;
    // free-list linkage
};

} // namespace lv
```

## 4. Smart Pointers

```cpp
namespace lv {

// Like std::unique_ptr but uses the library allocator
template<typename T, LvAllocator Alloc = SystemAllocator>
class UniquePtr {
public:
    UniquePtr() noexcept = default;
    explicit UniquePtr(T* p, Alloc alloc = {}) noexcept;
    ~UniquePtr();

    UniquePtr(UniquePtr&&) noexcept;
    UniquePtr& operator=(UniquePtr&&) noexcept;

    [[nodiscard]] T*       get()       noexcept;
    [[nodiscard]] const T* get() const noexcept;
    T&  operator*()  noexcept;
    T*  operator->() noexcept;
    explicit operator bool() const noexcept;
    void reset(T* p = nullptr) noexcept;
    [[nodiscard]] T* release() noexcept;
};

template<typename T, LvAllocator Alloc = SystemAllocator, typename... Args>
[[nodiscard]] UniquePtr<T, Alloc>
make_unique(Alloc alloc, Args&&... args);

} // namespace lv
```

## 5. Widget Ownership in the Tree

```
Screen (std::vector<UniquePtr<Object>>)
  └── Container (std::vector<UniquePtr<Object>>)
        ├── Label       ← owned, destroyed with parent
        └── Button      ← owned, destroyed with parent
              └── Label ← owned, destroyed with Button
```

External code holds non-owning `ObjectRef<T>` handles.  When an object is
destroyed, all `ObjectRef`s pointing to it are silently invalidated via an
intrusive validity token stored inside `Object`.

## 6. String Storage

`lv::Label`, `lv::TextArea`, and similar text widgets store text in:
- **Static mode**: `std::string_view` into a caller-owned `const char[]`
  literal (zero-copy, zero-allocation).
- **Dynamic mode**: `std::string` (or `std::pmr::string` when a PMR allocator
  policy is active).

```cpp
// Static – zero allocation
label.set_text(std::string_view{"Hello"});

// Dynamic – copied into internal buffer
label.set_text(std::format("Temp: {} °C", t));
```

## 7. Memory Budget Targets

| Configuration                                       | Flash (kB) | RAM (kB) | Notes               |
|-----------------------------------------------------|------------|----------|---------------------|
| Minimal (ArenaAllocator, no widgets)                | ~80        | ~8       | Core + display only |
| Default desktop (SystemAllocator, all widgets)      | ~320       | ~64      | Linux/Windows host  |
| Bare-metal embedded (ArenaAllocator, 8 widgets)     | ~128       | ~32      | Cortex-M4           |
| Full-featured (SystemAllocator, all widgets + libs) | ~512       | ~128     | Linux MPU           |

These targets are compile-time enforced via static_assert on policy types where
possible and are verified by the integration test suite.
