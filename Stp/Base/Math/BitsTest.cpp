// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Math/Bits.h"

#include "Base/Crypto/CryptoRandom.h"
#include "Base/Test/GTest.h"

namespace stp {

TEST(BitsTest, extractFirstOneBit32) {
  auto slow = [](uint32_t x) {
    uint32_t bit = 1;
    for (; bit; bit <<= 1) {
      if (x & bit)
        break;
    }
    return bit;
  };

  const uint32_t input[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
    UINT32_C(0x11111111),
    UINT32_C(0xAABBAABB)
  };

  for (auto x : input)
    EXPECT_EQ(slow(x), extractFirstOneBit(x));

  CryptoRandom rng;
  for (int i = 0; i < 100; ++i) {
    uint32_t x = rng.nextUint32();
    EXPECT_EQ(slow(x), extractFirstOneBit(x));
  }
}

TEST(BitsTest, extractFirstOneBit64) {
  auto slow = [](uint64_t x) {
    uint64_t bit = 1;
    for (; bit; bit <<= 1) {
      if (x & bit)
        break;
    }
    return bit;
  };

  const uint64_t input[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
    UINT64_C(0x1111111111111111),
    UINT64_C(0xAABBAABBAABBAABB),
  };

  for (auto x : input)
    EXPECT_EQ(slow(x), extractFirstOneBit(x));

  CryptoRandom rng;
  for (int i = 0; i < 100; ++i) {
    uint64_t x = rng.nextUint64();
    EXPECT_EQ(slow(x), extractFirstOneBit(x));
  }
}

TEST(BitsTest, extractLastOneBit32) {
  auto slow = [](uint32_t x) {
    uint32_t bit = UINT32_C(1) << 31;
    for (; bit; bit >>= 1) {
      if (x & bit)
        break;
    }
    return bit;
  };

  const uint32_t input[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
    UINT32_C(0x11111111),
    UINT32_C(0xAABBAABB)
  };

  for (auto x : input)
    EXPECT_EQ(slow(x), extractLastOneBit(x));

  CryptoRandom rng;
  for (int i = 0; i < 100; ++i) {
    uint32_t x = rng.nextUint32();
    EXPECT_EQ(slow(x), extractLastOneBit(x));
  }
}

TEST(BitsTest, extractLastOneBit64) {
  auto slow = [](uint64_t x) {
    uint64_t bit = UINT64_C(1) << 63;
    for (; bit; bit >>= 1) {
      if (x & bit)
        break;
    }
    return bit;
  };

  const uint64_t input[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
    UINT64_C(0x1111111111111111),
    UINT64_C(0xAABBAABBAABBAABB),
  };

  for (auto x : input)
    EXPECT_EQ(slow(x), extractLastOneBit(x));

  CryptoRandom rng;
  for (int i = 0; i < 100; ++i) {
    uint64_t x = rng.nextUint64();
    EXPECT_EQ(slow(x), extractLastOneBit(x));
  }
}

TEST(BitsTest, countBitsPopulation32) {
  auto slow = [](uint32_t x) {
    int count = 0;
    for (; x; x >>= 1)
      count += x & 1;
    return count;
  };

  const uint32_t input[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
    UINT32_C(0x11111111),
    UINT32_C(0xF0F0F0F0),
    UINT32_C(0xFFF0F0FF),
    UINT32_C(0xFFFFFFFF),
  };

  for (auto x : input)
    EXPECT_EQ(slow(x), countBitsPopulation(x));

  CryptoRandom rng;
  for (int i = 0; i < 100; ++i) {
    uint32_t x = rng.nextUint32();
    EXPECT_EQ(slow(x), countBitsPopulation(x));
  }
}

TEST(BitsTest, countBitsPopulation64) {
  auto slow = [](uint64_t x) {
    int count = 0;
    for (; x; x >>= 1)
      count += x & 1;
    return count;
  };

  const uint64_t input[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
    UINT64_C(0x1111111111111111),
    UINT64_C(0xF0F0F0F0F0F0F0F0),
    UINT64_C(0xFFF0F0FFFFF0F0FF),
    UINT64_C(0xFFFFFFFFFFFFFFFF),
  };

  for (auto x : input)
    EXPECT_EQ(slow(x), countBitsPopulation(x));

  CryptoRandom rng;
  for (int i = 0; i < 100; ++i) {
    uint64_t x = rng.nextUint64();
    EXPECT_EQ(slow(x), countBitsPopulation(x));
  }
}

TEST(BitsTest, getBitsParity32) {
  auto slow = [](uint32_t x) {
    int count = 0;
    for (; x; x >>= 1)
      count += x & 1;
    return (count & 1) != 0;
  };

  const uint32_t input[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
    UINT32_C(0x11111111),
    UINT32_C(0xF0F0F0F0),
    UINT32_C(0xFFF0F0FF),
    UINT32_C(0xFFFFFFFF),
  };

  for (auto x : input)
    EXPECT_EQ(slow(x), getBitsParity(x));

  CryptoRandom rng;
  for (int i = 0; i < 100; ++i) {
    uint32_t x = rng.nextUint32();
    EXPECT_EQ(slow(x), getBitsParity(x));
  }
}

TEST(BitsTest, getBitsParity64) {
  auto slow = [](uint64_t x) {
    int count = 0;
    for (; x; x >>= 1)
      count += x & 1;
    return (count & 1) != 0;
  };

  const uint64_t input[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
    UINT64_C(0x1111111111111111),
    UINT64_C(0xF0F0F0F0F0F0F0F0),
    UINT64_C(0xFFF0F0FFFFF0F0FF),
    UINT64_C(0xFFFFFFFFFFFFFFFF),
  };

  for (auto x : input)
    EXPECT_EQ(slow(x), getBitsParity(x));

  CryptoRandom rng;
  for (int i = 0; i < 100; ++i) {
    uint64_t x = rng.nextUint64();
    EXPECT_EQ(slow(x), getBitsParity(x));
  }
}

TEST(BitsTest, findFirstOneBit32) {
  auto slow = [](uint32_t x) {
    for (int i = 0; i < 32; ++i) {
      if (x & (UINT32_C(1) << i))
        return i;
    }
    return -1;
  };

  const uint32_t input[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
    UINT32_C(0x11111111),
    UINT32_C(0xF0F0F0F0),
    UINT32_C(0xFFF0F0FF),
    UINT32_C(0xFFFFFFFF),
  };

  for (auto x : input)
    EXPECT_EQ(slow(x), findFirstOneBit(x));

  CryptoRandom rng;
  for (int i = 0; i < 100; ++i) {
    uint32_t x = rng.nextUint32();
    if (!x)
      continue;
    EXPECT_EQ(slow(x), findFirstOneBit(x));
  }
}

TEST(BitsTest, findFirstOneBit64) {
  auto slow = [](uint64_t x) {
    for (int i = 0; i < 64; ++i) {
      if (x & (UINT64_C(1) << i))
        return i;
    }
    return -1;
  };

  const uint64_t input[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
    UINT64_C(0x1111111111111111),
    UINT64_C(0xF0F0F0F0F0F0F0F0),
    UINT64_C(0xFFF0F0FFFFF0F0FF),
    UINT64_C(0xFFFFFFFFFFFFFFFF),
  };

  for (auto x : input)
    EXPECT_EQ(slow(x), findFirstOneBit(x)) << x;

  CryptoRandom rng;
  for (int i = 0; i < 100; ++i) {
    uint64_t x = rng.nextUint64();
    if (!x)
      continue;
    EXPECT_EQ(slow(x), findFirstOneBit(x));
  }
}

TEST(BitsTest, findLastOneBit32) {
  auto slow = [](uint32_t x) {
    for (int i = 31; i >= 0; --i) {
      if (x & (UINT32_C(1) << i))
        return i;
    }
    return -1;
  };

  const uint32_t input[] = {
    1, 2, 3, 4, 5, 6, 7, 8, 9,
    UINT32_C(0x11111111),
    UINT32_C(0xF0F0F0F0),
    UINT32_C(0xFFF0F0FF),
    UINT32_C(0xFFFFFFFF),
  };

  for (auto x : input)
    EXPECT_EQ(slow(x), findLastOneBit(x));

  CryptoRandom rng;
  for (int i = 0; i < 100; ++i) {
    uint32_t x = rng.nextUint32();
    if (!x)
      continue;
    EXPECT_EQ(slow(x), findLastOneBit(x));
  }
}

TEST(BitsTest, findLastOneBit64) {
  auto slow = [](uint64_t x) {
    for (int i = 63; i >= 0; --i) {
      if (x & (UINT64_C(1) << i))
        return i;
    }
    return -1;
  };

  const uint64_t input[] = {
    1, 2, 3, 4, 5, 6, 7, 8, 9,
    UINT64_C(0x1111111111111111),
    UINT64_C(0xF0F0F0F0F0F0F0F0),
    UINT64_C(0xFFF0F0FFFFF0F0FF),
    UINT64_C(0xFFFFFFFFFFFFFFFF),
  };

  for (auto x : input)
    EXPECT_EQ(slow(x), findLastOneBit(x)) << x;

  CryptoRandom rng;
  for (int i = 0; i < 100; ++i) {
    uint64_t x = rng.nextUint64();
    if (!x)
      continue;
    EXPECT_EQ(slow(x), findLastOneBit(x));
  }
}

TEST(BitsTest, countTrailingZeroBits32) {
  auto slow = [](uint32_t x) {
    for (int i = 0; i < 32; ++i) {
      if (x & (UINT32_C(1) << i))
        return i;
    }
    return 32;
  };

  const uint32_t input[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
    UINT32_C(0x11111111),
    UINT32_C(0xF0F0F0F0),
    UINT32_C(0xFFF0F0FF),
    UINT32_C(0xFFFFFFFF),
  };

  for (auto x : input)
    EXPECT_EQ(slow(x), countTrailingZeroBits(x));

  CryptoRandom rng;
  for (int i = 0; i < 100; ++i) {
    uint32_t x = rng.nextUint32();
    EXPECT_EQ(slow(x), countTrailingZeroBits(x));
  }
}

TEST(BitsTest, countTrailingZeroBits64) {
  auto slow = [](uint64_t x) {
    for (int i = 0; i < 64; ++i) {
      if (x & (UINT64_C(1) << i))
        return i;
    }
    return 64;
  };

  const uint64_t input[] = {
    1, 2, 3, 4, 5, 6, 7, 8, 9,
    UINT64_C(0x1111111111111111),
    UINT64_C(0xF0F0F0F0F0F0F0F0),
    UINT64_C(0xFFF0F0FFFFF0F0FF),
    UINT64_C(0xFFFFFFFFFFFFFFFF),
  };

  for (auto x : input)
    EXPECT_EQ(slow(x), countTrailingZeroBits(x)) << x;

  CryptoRandom rng;
  for (int i = 0; i < 100; ++i) {
    uint64_t x = rng.nextUint64();
    EXPECT_EQ(slow(x), countTrailingZeroBits(x));
  }
}

TEST(BitsTest, countLeadingZeroBits16) {
  auto slow = [](uint16_t x) {
    for (int i = 0; i < 16; ++i) {
      if (x & (UINT16_C(1) << (15 - i)))
        return i;
    }
    return 16;
  };

  const uint16_t input[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
    UINT32_C(0x1111),
    UINT32_C(0xF0F0),
    UINT32_C(0xF0FF),
    UINT32_C(0xFFFF),
  };

  for (auto x : input)
    EXPECT_EQ(slow(x), countLeadingZeroBits(x));
}

TEST(BitsTest, countLeadingZeroBits32) {
  auto slow = [](uint32_t x) {
    for (int i = 0; i < 32; ++i) {
      if (x & (UINT32_C(1) << (31 - i)))
        return i;
    }
    return 32;
  };

  const uint32_t input[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
    UINT32_C(0x11111111),
    UINT32_C(0xF0F0F0F0),
    UINT32_C(0xFFF0F0FF),
    UINT32_C(0xFFFFFFFF),
  };

  for (auto x : input)
    EXPECT_EQ(slow(x), countLeadingZeroBits(x));

  CryptoRandom rng;
  for (int i = 0; i < 100; ++i) {
    uint32_t x = rng.nextUint32();
    EXPECT_EQ(slow(x), countLeadingZeroBits(x));
  }
}

TEST(BitsTest, countLeadingZeroBits64) {
  auto slow = [](uint64_t x) {
    for (int i = 0; i < 64; ++i) {
      if (x & (UINT64_C(1) << (63 - i)))
        return i;
    }
    return 64;
  };

  const uint64_t input[] = {
    1, 2, 3, 4, 5, 6, 7, 8, 9,
    UINT64_C(0x1111111111111111),
    UINT64_C(0xF0F0F0F0F0F0F0F0),
    UINT64_C(0xFFF0F0FFFFF0F0FF),
    UINT64_C(0xFFFFFFFFFFFFFFFF),
  };

  for (auto x : input)
    EXPECT_EQ(slow(x), countLeadingZeroBits(x)) << x;

  CryptoRandom rng;
  for (int i = 0; i < 100; ++i) {
    uint64_t x = rng.nextUint64();
    EXPECT_EQ(slow(x), countLeadingZeroBits(x));
  }
}

TEST(BitsTest, rotateRight32) {
  struct Pair {
    uint32_t expected;
    uint32_t input;
    unsigned shift;
  } input[] = {
    { UINT32_C(0), UINT32_C(0), 1 },
    { UINT32_C(1), UINT32_C(1), 0 },
    { UINT32_C(0x80000000), UINT32_C(1), 1 },
    { UINT32_C(0x0001F000), UINT32_C(0xF0000001), 16 },
    { UINT32_C(0xABBAABBA), UINT32_C(0xAABBAABB), 28 },
  };

  for (Pair x : input)
    EXPECT_EQ(x.expected, rotateBitsRight(x.input, x.shift));
}

TEST(BitsTest, RotateLeft32) {
  struct Pair {
    uint32_t expected;
    uint32_t input;
    unsigned shift;
  } input[] = {
    { UINT32_C(0), UINT32_C(0), 1 },
    { UINT32_C(1), UINT32_C(1), 0 },
    { UINT32_C(1), UINT32_C(0x80000000), 1 },
    { UINT32_C(2), UINT32_C(1), 1 },
    { UINT32_C(0x0001F000), UINT32_C(0xF0000001), 16 },
    { UINT32_C(0xBAABBAAB), UINT32_C(0xAABBAABB), 28 },
  };

  for (Pair x : input)
    EXPECT_EQ(x.expected, rotateBitsLeft(x.input, x.shift));
}

TEST(BitsTest, reverse8) {
  auto slow = [](uint8_t x) {
    uint8_t m = 1 << 7;
    uint8_t c = 0;
    for (unsigned i = 0; i < 8; ++i, x >>= 1, m >>= 1) {
      if (x & 1)
        c |= m;
    }
    return c;
  };

  for (int i = 0; i < 0x100; ++i) {
    uint8_t x = static_cast<uint8_t>(i);
    EXPECT_EQ(slow(x), reverseBits(x));
  }
}

TEST(BitsTest, reverse16) {
  auto slow = [](uint16_t x) {
    uint16_t m = UINT16_C(1) << 15;
    uint16_t c = 0;
    for (unsigned i = 0; i < 16; ++i, x >>= 1, m >>= 1) {
      if (x & 1)
        c |= m;
    }
    return c;
  };

  const uint16_t input[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
    UINT16_C(0x1111),
    UINT16_C(0xF0F0),
    UINT16_C(0xF0FF),
    UINT16_C(0xFFFF),
  };

  for (auto x : input)
    EXPECT_EQ(slow(x), reverseBits(x));

  CryptoRandom rng;
  for (int i = 0; i < 100; ++i) {
    uint16_t x = static_cast<uint16_t>(rng.nextUint32());
    EXPECT_EQ(slow(x), reverseBits(x));
  }
}

TEST(BitsTest, reverse32) {
  auto slow = [](uint32_t x) {
    uint32_t m = UINT32_C(1) << 31;
    uint32_t c = 0;
    for (unsigned i = 0; i < 32; ++i, x >>= 1, m >>= 1) {
      if (x & 1)
        c |= m;
    }
    return c;
  };

  const uint32_t input[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
    UINT32_C(0x11111111),
    UINT32_C(0xF0F0F0F0),
    UINT32_C(0xFFF0F0FF),
    UINT32_C(0xFFFFFFFF),
  };

  for (uint32_t x : input)
    EXPECT_EQ(slow(x), reverseBits(x));

  CryptoRandom rng;
  for (int i = 0; i < 100; ++i) {
    uint32_t x = rng.nextUint32();
    EXPECT_EQ(slow(x), reverseBits(x));
  }
}

TEST(BitsTest, reverse64) {
  auto slow = [](uint64_t x) {
    uint64_t m = UINT64_C(1) << 63;
    uint64_t c = 0;
    for (unsigned i = 0; i < 64; ++i, x >>= 1, m >>= 1) {
      if (x & 1)
        c |= m;
    }
    return c;
  };

  const uint64_t input[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
    UINT64_C(0x11111111),
    UINT64_C(0xF0F0F0F0),
    UINT64_C(0xFFF0F0FFABCDEF01),
    UINT64_C(0xFFFFFFFF),
  };

  for (uint64_t x : input)
    EXPECT_EQ(slow(x), reverseBits(x));

  CryptoRandom rng;
  for (int i = 0; i < 100; ++i) {
    uint64_t x = rng.nextUint64();
    EXPECT_EQ(slow(x), reverseBits(x));
  }
}

} // namespace stp
