#pragma once

#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

template <typename TYPE>
class TypeEntry;

using destructor_callback_t = void (*)(void*);

struct TypeInfo {
  destructor_callback_t destructor;
};

using type_id_type = std::size_t;

struct TypeRegistry {
  private:
  std::vector<TypeInfo> type_info;
  TypeRegistry() {}
  TypeRegistry(const TypeRegistry&) = delete;
  TypeRegistry(TypeRegistry&&) = delete;
  TypeRegistry& operator=(const TypeRegistry&) = delete;
  TypeRegistry& operator=(TypeRegistry&&) = delete;

  public:
  void destroy(type_id_type id, void* data) const {
    type_info[id].destructor(data);
  }

  template <typename T>
  TypeEntry<T>* add_type();

  static TypeRegistry& instance() {
    static TypeRegistry* instance = new TypeRegistry;

    return *instance;
  }
};

template <typename TYPE>
class TypeEntry {
  private:
  TypeEntry(type_id_type id) : type_id(id) {}
  TypeEntry(const TypeEntry&) = delete;
  TypeEntry(TypeEntry&&) = delete;
  TypeEntry& operator=(const TypeEntry&) = delete;
  TypeEntry& operator=(TypeEntry&&) = delete;

  public:
  const type_id_type type_id;

  static TypeEntry& instance() {
    static TypeEntry* instance = TypeRegistry::instance().add_type<TYPE>();
    return *instance;
  }

  template <typename T>
  friend TypeEntry<T>* TypeRegistry::add_type();
};

template <typename T>
void destructor(void* data) {
  delete static_cast<T*>(data);
}

template <typename T>
TypeEntry<T>* TypeRegistry::add_type() {
  auto type_id = type_info.size();

  type_info.push_back({
    &destructor<T>,
  });

  return new TypeEntry<T>(type_id);
}

class Object {
  private:
  type_id_type type_id;
  void* data;

  public:
  template <typename T>
  Object(T* data) : type_id(TypeEntry<T>::instance().type_id), data(data) {}

  Object(const Object& o) = delete;
  Object(Object&& o) = delete;
  Object& operator=(const Object& o) = delete;
  Object& operator=(Object&& o) = delete;

  template <typename T, typename... Args>
  static Object make_object(Args&&... args) {
    return {new T(std::forward<Args>(args)...)};
  }

  ~Object() {
    // this could be done with a unique_ptr, but that *might* be less efficient
    // as it would require more copies of the destructor callback. (note that
    // this may actually have the same memory footprint, since sizeof(type_id)
    // may be equal to sizeof(void(*)(...))
    TypeRegistry::instance().destroy(type_id, data);
  }

  template <typename T>
  friend T* as_a(Object&);

  template <typename T>
  friend const T* as_a(const Object&);

  template <typename T>
  friend bool is_a(const Object&);
};

template <typename T>
T* as_a(Object& o) {
  if (TypeEntry<T>::instance().type_id == o.type_id) {
    return static_cast<T*>(o.data);
  } else {
    return nullptr;
  }
}

template <typename T>
const T* as_a(const Object& o) {
  if (TypeEntry<T>::instance().type_id == o.type_id) {
    return static_cast<const T*>(o.data);
  } else {
    return nullptr;
  }
}

template <typename T>
bool is_a(const Object& o) {
  return TypeEntry<T>::instance().type_id == o.type_id;
}