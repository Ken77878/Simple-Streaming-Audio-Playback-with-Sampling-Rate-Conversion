#pragma once
template <class T> class NoCopy {
protected:
  NoCopy() = default;
  ~NoCopy() = default;

public:
  NoCopy &operator=(const NoCopy &) = delete;
  NoCopy(const NoCopy &) = delete;
};
