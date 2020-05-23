#pragma once
template <class T> class NoMove {
protected:
  NoMove() = default;
  ~NoMove() = default;

public:
  NoMove(NoMove &&) = delete;
  NoMove &operator=(NoMove &&) = delete;
};
