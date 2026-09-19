#pragma once

#include "../external/picojson.h"
#include <glm/glm.hpp>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <vector>

namespace eclipse::core::json {
using Value = picojson::value;
using Object = picojson::object;
using Array = picojson::array;

inline void ValidateNumbers(const Value& v) {
  if (v.is<double>() && !std::isfinite(v.get<double>()))
    throw std::runtime_error("Non-finite scene number");
  if (v.is<Array>()) for (const auto& x : v.get<Array>()) ValidateNumbers(x);
  if (v.is<Object>()) for (const auto& [key, x] : v.get<Object>()) ValidateNumbers(x);
}

inline Value Encode(const std::string& x) { return Value(x); }
inline Value Encode(bool x) { return Value(x); }
inline Value Encode(int x) { return Value(double(x)); }
inline Value Encode(float x) {
  if (!std::isfinite(x)) throw std::runtime_error("Non-finite scene number");
  return Value(double(x));
}
inline Value Encode(glm::vec2 x) { return Value(Array{Encode(x.x), Encode(x.y)}); }
template<class T> Value Encode(T x);
template<class T> Value Encode(const std::vector<T>& xs) {
  Array a;
  for (const auto& x : xs) a.push_back(Encode(x));
  return Value(a);
}
template<class T> Value Encode(T x) {
  Object o;
  x.Fields([&](const char* key, const auto& field) { o[key] = Encode(field); });
  return Value(o);
}

inline void Decode(const Value& v, std::string& x) {
  if (!v.is<std::string>()) throw std::runtime_error("Expected string");
  x = v.get<std::string>();
}
inline void Decode(const Value& v, bool& x) {
  if (!v.is<bool>()) throw std::runtime_error("Expected boolean");
  x = v.get<bool>();
}
inline void Decode(const Value& v, float& x) {
  if (!v.is<double>() || !std::isfinite(v.get<double>()) ||
      std::abs(v.get<double>()) > std::numeric_limits<float>::max())
    throw std::runtime_error("Expected finite float");
  x = float(v.get<double>());
}
inline void Decode(const Value& v, int& x) {
  if (!v.is<double>()) throw std::runtime_error("Expected integer");
  const double n = v.get<double>();
  if (!std::isfinite(n) || std::floor(n) != n ||
      n < std::numeric_limits<int>::min() || n > std::numeric_limits<int>::max())
    throw std::runtime_error("Integer out of range");
  x = int(n);
}
inline void Decode(const Value& v, glm::vec2& x) {
  if (!v.is<Array>() || v.get<Array>().size() != 2)
    throw std::runtime_error("Expected two coordinates");
  Decode(v.get<Array>()[0], x.x); Decode(v.get<Array>()[1], x.y);
}
template<class T> void Decode(const Value& v, T& x);
template<class T> void Decode(const Value& v, std::vector<T>& xs) {
  if (!v.is<Array>()) throw std::runtime_error("Expected array");
  xs.clear();
  for (const auto& entry : v.get<Array>()) {
    T x{}; Decode(entry, x); xs.push_back(std::move(x));
  }
}
template<class T> void Decode(const Value& v, T& x) {
  if (!v.is<Object>()) throw std::runtime_error("Expected object");
  const auto& o = v.get<Object>();
  x.Fields([&](const char* key, auto& field) {
    const auto it = o.find(key);
    if (it == o.end()) return; // defaults keep old scene files compatible
    try { Decode(it->second, field); }
    catch (const std::exception& e) { throw std::runtime_error(std::string(key) + ": " + e.what()); }
  });
}

inline Value Read(const std::filesystem::path& path) {
  std::ifstream in(path);
  if (!in) throw std::runtime_error("Cannot read " + path.string());
  Value v;
  const auto error = picojson::parse(v, in);
  if (!error.empty()) throw std::runtime_error(error);
  return v;
}

// Write beside the destination and replace only after a successful close.
inline void Write(const std::filesystem::path& path, const Value& v) {
  ValidateNumbers(v);
  if (path.empty()) throw std::runtime_error("Empty save path");
  if (!path.parent_path().empty()) std::filesystem::create_directories(path.parent_path());
  auto tmp = path; tmp += ".tmp";
  try {
    std::ofstream out(tmp, std::ios::binary | std::ios::trunc);
    out << v.serialize(true);
    out.close();
    if (!out) throw std::runtime_error("Cannot write " + path.string());
#ifdef _WIN32
    // C++17 rename does not replace an existing destination on Windows.
    std::error_code removeError;
    std::filesystem::remove(path, removeError);
    if (removeError) throw std::runtime_error("Cannot replace " + path.string());
#endif
    std::filesystem::rename(tmp, path);
  } catch (...) {
    std::error_code ec; std::filesystem::remove(tmp, ec); throw;
  }
}
} // namespace eclipse::core::json
