template<typename TInt>
void testCompress(const std::initializer_list<TInt>& list, int bitwidth) {
  std::vector<TInt> expected(list.begin(), list.end());
  std::sort(expected.begin(), expected.end(), std::greater<TInt>());

  TInt compressed = compress<TInt>(list.begin(), list.end(), bitwidth);

  std::vector<TInt> uncompressed;

  BitUnpacker<TInt> unpacker(compressed, bitwidth);
  for (TInt v = unpacker.next(); v; v = unpacker.next())
    uncompressed.push_back(v);

  /*
  for (auto v : BitUnpacker<TInt>(compressed, bitwidth)) {
    std::cout << "pb:"<<static_cast<long>(v)<<std::endl;
    uncompressed.push_back(v);
  }*/

  //uncompress<TInt, std::vector<TInt>>(uncompressed, compressed, bitwidth);

  for (auto e : expected)
    std::cout << "exp:"<<e<<std::endl;

  if (expected != uncompressed)
    throw std::runtime_error("FOO");
}



  // ==========================================================================
  {
    testCompress<uint8_t>({1}, 5);
    testCompress<uint8_t>({1,2}, 5);
    testCompress<uint16_t>({3,6,12,10}, 5);

    std::vector<uint32_t> t({5,9,2,13});
    uint32_t compressed = compress<uint32_t>(t.begin(), t.end(), 5);

#if 0
    std::cout << "HERESMT"<<std::endl;
    BitUnpacker<uint32_t> unp(compressed, 5);
    for (uint32_t v = unp.next(); v; v = unp.next())
      std::cout << v << std::endl;
    std::cout << "///HERESMT"<<std::endl;
#endif
  }

  // Test: 32bit
  {
    TinyPackedArray<32, uint32_t> array;
    assert(array.capacity() == 1);
    array.push_back(0xBEEF);
    assert(array[0] == 0xBEEF);
  }

  // Test: 24bit
  {
    TinyPackedArray<24, uint32_t> array;
    assert(array.capacity() == 1);
    array.push_back(0xBEEF);
    assert(array[0] == 0xBEEF);
  }

  // Test: 16bit
  {
    TinyPackedArray<16, uint32_t> array;
    assert(array.capacity() == 2);
    array.push_back(0xDEAD);
    array.push_back(0xBEEF);
    assert(array[0] == 0xDEAD);
    assert(array[1] == 0xBEEF);
  }

  // Test: 8bit
  {
    TinyPackedArray<8, uint32_t> array;
    assert(array.capacity() == 4);
    array.push_back(0xAA);
    array.push_back(0xBB);
    array.push_back(0xCC);
    array.push_back(0xDD);
    assert(array[0] == 0xAA);
    assert(array[1] == 0xBB);
    assert(array[2] == 0xCC);
    assert(array[3] == 0xDD);
  }

  // Test: 3bit
  {
    TinyPackedArray<3, uint32_t> arr;
    assert(arr.capacity() == 10);
    arr.push_back(1);
    arr.push_back(2);
    arr.push_back(3);
    arr.push_back(4);
    arr.push_back(5);
    arr.push_back(1);
    arr.push_back(2);
    arr.push_back(3);
    arr.push_back(4);
    arr.push_back(5);
    assert(arr[0] == 1 && arr[1] == 2 && arr[2] == 3 && arr[3] == 4 && arr[4] == 5);
    assert(arr[5] == 1 && arr[6] == 2 && arr[7] == 3 && arr[8] == 4 && arr[9] == 5);

    size_t sum = 0;
    for (auto i : arr) { sum += i; }
    assert(sum == 30);
  }

  // Test: Access
  {
    TinyPackedArray<3, uint32_t> arr;
    arr.push_back(0);
    arr.push_back(1);
    arr[0] = 4;
    assert(arr[0] == 4);
    arr[1] = arr[0];
    assert(arr[1] == 4);
  }

  // Test: Sorting
  {
    TinyPackedArray<3, uint32_t> arr;
    arr.push_back(5);
    arr.push_back(2);
    arr.push_back(1);
    arr.push_back(4);
    arr.push_back(3);

    std::swap(*(arr.begin()), *(arr.begin()+1));
    std::cout << *(arr.begin()) << ' ' << *(arr.begin()+1) << std::endl;


#if 1
    std::sort(arr.begin(), arr.end());
    for (auto i : arr) { std::cout << i << std::endl; }
    assert(arr[0] == 1 && arr[1] == 2 && arr[2] == 3 && arr[3] == 4 && arr[4] == 5);
#endif
  }

  // Test: Sorting
  {
    TinyPackedArray<5, uint32_t> arr;
    arr.push_back(5);
    arr.push_back(2);
    arr.push_back(1);

    std::sort(arr.begin(), arr.end(), std::greater<uint8_t>());
    std::cout << arr.data() << std::endl;

    std::sort(arr.begin(), arr.end(), std::less<uint8_t>());
    std::cout << arr.data() << std::endl;
  }

