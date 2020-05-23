#pragma once
template <class T> class NoCopyAndMove {
protected:
  NoCopyAndMove() = default;
  ~NoCopyAndMove() = default;

public:
  NoCopyAndMove &operator=(const NoCopyAndMove &) = delete;
  NoCopyAndMove(const NoCopyAndMove &) = delete;
  NoCopyAndMove(NoCopyAndMove &&) = delete;
  NoCopyAndMove &operator=(NoCopyAndMove &&) = delete;
};
