#ifndef SRC_HPP
#define SRC_HPP

#include <stdexcept>
#include <initializer_list>
#include <typeinfo>
#include <type_traits>
#include <utility>
// 以上是你所需要的头文件，如果你想使用其它头文件，请询问助教
// 禁止使用 std::shared_ptr 与 std::any

namespace sjtu {

struct _control_block_base {
  int refcnt = 1;
  virtual ~_control_block_base() = default;
  virtual const std::type_info &type() const = 0;
  virtual void *ptr() = 0;
  virtual const void *ptr() const = 0;
};

template <class T>
struct _control_block : _control_block_base {
  T *p;
  explicit _control_block(T *pp) : p(pp) {}
  ~_control_block() override { delete p; }
  const std::type_info &type() const override { return typeid(T); }
  void *ptr() override { return static_cast<void *>(p); }
  const void *ptr() const override { return static_cast<const void *>(p); }
};

class any_ptr {
 public:
  /**
   * @brief 默认构造函数，行为应与创建空指针类似
   * 
   */
  any_ptr() : cb(nullptr) {}

  /**
   * @brief 拷贝构造函数，要求拷贝的层次为浅拷贝，即该对象与被拷贝对象的内容指向同一块内存
   * @note 若将指针作为参数传入，则指针的生命周期由该对象管理
   * @example
   *  any_ptr a = make_any_ptr(1);
   *  any_ptr b = a;
   *  a.unwrap<int> = 2;
   *  std::cout << b << std::endl; // 2
   * @param other
   */
  any_ptr(const any_ptr &other) : cb(other.cb) {
    if (cb) ++cb->refcnt;
  }
  template <class T> any_ptr(T *ptr) : cb(nullptr) {
    if (ptr) cb = new _control_block<T>(ptr);
  }

  /**
   * @brief 析构函数，注意若一块内存被多个对象共享，那么只有最后一个析构的对象需要释放内存
   * @example
   *  any_ptr a = make_any_ptr(1);
   *  {
   *    any_ptr b = a;
   *  }
   *  std::cout << a << std::endl; // valid
   * @example
   *  int *p = new int(1);
   *  any_ptr a = p;
    *  {
   */
  ~any_ptr() { release(); }

  /**
   * @brief 拷贝赋值运算符，要求拷贝的层次为浅拷贝，即该对象与被拷贝对象的内容指向同一块内存
   * @note 应与指针拷贝赋值运算符的语义相近
   * @param other
   * @return any_ptr&
   */
  any_ptr &operator=(const any_ptr &other) {
    if (this == &other) return *this;
    release();
    cb = other.cb;
    if (cb) ++cb->refcnt;
    return *this;
  }
  template <class T> any_ptr &operator=(T *ptr) {
    release();
    cb = nullptr;
    if (ptr) cb = new _control_block<T>(ptr);
    return *this;
  }

  /**
   * @brief 获取该对象指向的值的引用
   * @note 若该对象指向的值不是 T 类型，则抛出异常 std::bad_cast
   * @example
   *  any_ptr a = make_any_ptr(1);
   *  std::cout << a.unwrap<int>() << std::endl; // 1
   * @tparam T
   * @return T&
   */
  template <class T> T &unwrap() {
    if (!cb || cb->type() != typeid(T)) throw std::bad_cast();
    return *static_cast<T *>(cb->ptr());
  }
  template <class T> const T &unwrap() const {
    if (!cb || cb->type() != typeid(T)) throw std::bad_cast();
    return *static_cast<const T *>(cb->ptr());
  }
  // 某一个 any_ptr 类对象可能为 const，请你补充 unwrap 函数

 private:
  _control_block_base *cb;

  void release() {
    if (cb) {
      if (--cb->refcnt == 0) delete cb;
      cb = nullptr;
    }
  }
};

/**
 * @brief 由指定类型的值构造一个 any_ptr 对象
 * @example
 *  any_ptr a = make_any_ptr(1);
 *  any_ptr v = make_any_ptr<std::vector<int>>(1, 2, 3);
 *  any_ptr m = make_any_ptr<std::map<int, int>>({{1, 2}, {3, 4}});
 * @tparam T
 * @param t
 * @return any_ptr
 */
template <class T> any_ptr make_any_ptr(const T &t) { return any_ptr(new T(t)); }
// Perfect-forwarding constructor for arbitrary types
// Trait to detect T::value_type
template <class, class = void>
struct _has_value_type : std::false_type {};
template <class X>
struct _has_value_type<X, std::void_t<typename X::value_type>> : std::true_type {};

// Perfect-forwarding constructor for arbitrary types, but disabled when
// Args all look like container value elements to prefer the range/ilist forms.
template <class T, class... Args,
          class = std::enable_if_t<
              (sizeof...(Args) > 0) &&
              !(
                  _has_value_type<T>::value &&
                  std::conjunction<std::is_convertible<Args, typename T::value_type>...>::value)
              >>
any_ptr make_any_ptr(Args &&...args) {
  return any_ptr(new T(std::forward<Args>(args)...));
}

// Construct containers directly from elements: make_any_ptr<std::vector<int>>(1,2,3)
template <class T, class... Args,
          typename VT = typename T::value_type,
          typename std::enable_if<
              (sizeof...(Args) > 0) && _has_value_type<T>::value &&
                  std::conjunction<std::is_convertible<Args, VT>...>::value,
              int>::type = 0>
any_ptr make_any_ptr(Args... args) {
  VT tmp_arr[] = {static_cast<VT>(args)...};
  return any_ptr(new T{tmp_arr, tmp_arr + sizeof...(Args)});
}

// Explicit initializer_list overload
template <class T>
any_ptr make_any_ptr(std::initializer_list<typename T::value_type> il) {
  return any_ptr(new T(il));
}
// 某些 any_ptr 类对象可能由不定长参数或初始化列表构造，请你参照上方的 example 补充 make_any_ptr 函数，我们会有一个特殊的测试点测试你的程序是否完成要求

}  // namespace sjtu

#endif
