# AUTOSAR / ESP‑32 / C++14 Reference
## 1. Built‑in error names (ESP‑IDF v4.4)
ESP‑IDF defines the following macros for errors:
```c
/* Success */
ESP_OK

/* Generic errors */
ESP_FAIL
ESP_ERR_INVALID_STATE
ESP_ERR_TIMEOUT
ESP_ERR_NOT_FINISHED
ESP_ERR_NOT_ALLOWED
ESP_ERR_NOT_SUPPORTED
ESP_ERR_INVALID_CRC
ESP_ERR_INVALID_RESPONSE
ESP_ERR_INVALID_SIZE
ESP_ERR_INVALID_MAC
ESP_ERR_INVALID_VERSION
ESP_ERR_INVALID_ARG
ESP_ERR_NO_MEM

/* Not found */
ESP_ERR_NOT_FOUND

/* Wi‑Fi specific */
ESP_ERR_WIFI_BASE

/* Mesh specific */
ESP_ERR_MESH_BASE

/* Flash specific */
ESP_ERR_FLASH_BASE

/* Hardware crypto module */
ESP_ERR_HW_CRYPTO_BASE

/* Memory‑Protection API */
ESP_ERR_MEMPROT_BASE
```

## 2. Static‑size key/value map for function pointers
```cpp
#include <array>
#include <string_view>
using fn_t = void (*)();
struct kvp{ std::string_view key; fn_t fn; };
constexpr std::array<kvp,4> command_table{
    kvp{"init", &my_init},
    kvp{"reset", &my_reset},
    kvp{"start", &my_start},
    kvp{"stop", &my_stop}
};
constexpr fn_t lookup(std::string_view name){
    for(const auto& e: command_table) if(e.key==name) return e.fn;
    return nullptr;
}
```

## 3. Overloading `operator[]` in C++14
```cpp
#include <cstddef>
#include <array>

template<typename T, std::size_t N>
class StaticVector{
public:
    const T& operator[](std::size_t i) const{ check(i); return data_[i]; }
    T& operator[](std::size_t i){ check(i); return data_[i]; }
    constexpr std::size_t size() const noexcept{ return N; }
private:
    void check(std::size_t i) const{
        if(i>=N) throw std::out_of_range("StaticVector: index out of range");
    }
    std::array<T,N> data_{};
};
```

## 4. ESP‑IDF `gpio_*` function signatures (v4.4)
| Function | Return type | Parameters |
| -------- | ----------- | ---------- |
| `gpio_set_direction` | `esp_err_t` | `gpio_num_t gpio_num, gpio_mode_t mode` |
| `gpio_set_level` | `esp_err_t` | `gpio_num_t gpio_num, uint32_t level` |
| `gpio_get_level` | `int` | `gpio_num_t gpio_num` |
| `gpio_reset_pin` | `esp_err_t` | `gpio_num_t gpio_num` |

Note: `gpio_get_direction` is **not part of** v4.4; ESP‑IDF only provides `gpio_get_level`.

## 5. Template + using declaration patterns
* Template alias:
```cpp
template<typename T>
using vec = std::vector<T>;
```
* Using declaration to expose base-class member (possibly a member template):
```cpp
struct Base{ template<class U> void foo(); };
struct Derived: Base{ using Base::foo; };
```

## 6. Operator `[]` overload example
```cpp
template<typename T, std::size_t N>
class StaticVector { /* as above */ };
```

## 7. Static‑size map with string keys and function pointers
(see section 2)

---

**The above is a compilation of all responses to the AUTOSAR, C++14, and ESP‑32 related questions.**