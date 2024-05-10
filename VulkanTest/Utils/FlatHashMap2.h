

// add DataBlock::prev
// Remove TableBlock list

namespace
{
	constexpr size_t tableSize = 10000;
};

template <class Pair>
class DataBlock
{
public:
	DataBlock() {}
	DataBlock(const Pair& pair) : pair(pair) {}
	Pair pair;
	int32_t hash = -1;
	int32_t prev = -1;
	int32_t next = -1;
};

template <class FlatHashMap2>
class ConstIterator
{
public:
	using Pair = FlatHashMap2::Pair;

	ConstIterator(const FlatHashMap2& flatHashMap, int32_t dataBlockIndex)
		: flatHashMap(flatHashMap), dataBlockIndex(dataBlockIndex)
	{}

	bool operator==(const ConstIterator& it) const
	{
		return dataBlockIndex == it.dataBlockIndex;
	}

	bool operator!=(const ConstIterator& it) const
	{
		return dataBlockIndex != it.dataBlockIndex;
	}

	ConstIterator& operator++()
	{
		if (dataBlockIndex == -1) {
			throw std::exception();
		}

		dataBlockIndex = flatHashMap.dataBlocks[dataBlockIndex].next;
	}

	const Pair& operator*()
	{
		return flatHashMap.dataBlocks[dataBlockIndex].pair;
	}

	const Pair* operator->()
	{
		return &**this;
	}

public:
	int32_t dataBlockIndex = -1;
	const FlatHashMap2& flatHashMap;
};

template <class FlatHashMap2>
class Iterator : public ConstIterator<FlatHashMap2>
{
public:
	using Mybase = ConstIterator<FlatHashMap2>;
	using Mybase::Mybase;

	Iterator(const Mybase& constIt)
		: Iterator(constIt.flatHashMap, constIt.dataBlockIndex)
	{
	}

	Iterator& operator++()
	{
		if (ConstIterator<FlatHashMap2>::dataBlockIndex == -1) {
			throw std::exception();
		}

		ConstIterator<FlatHashMap2>::dataBlockIndex = ConstIterator<FlatHashMap2>::flatHashMap.dataBlocks[ConstIterator<FlatHashMap2>::dataBlockIndex].next;
	}

	FlatHashMap2::Pair& operator*()
	{
		const FlatHashMap2::Pair* pair = &ConstIterator<FlatHashMap2>::flatHashMap.dataBlocks[ConstIterator<FlatHashMap2>::dataBlockIndex].pair;
		return *const_cast<FlatHashMap2::Pair*>(pair);
	}

	FlatHashMap2::Pair* operator->()
	{
		const FlatHashMap2::Pair* pair = &**this;
		return const_cast<FlatHashMap2::Pair*>(pair);
	}
};

template <class Key, class Value, class Hasher = std::hash<Key>>
class FlatHashMap2
{
public:
	using Iterator = Iterator<FlatHashMap2>;
	using ConstIterator = ConstIterator<FlatHashMap2>;
	using Pair = std::pair<Key, Value>;

	ConstIterator begin() const
	{
		return ConstIterator(*this, firstDataBlock);
	}

	ConstIterator end() const
	{
		return ConstIterator(*this, -1);
	}

	Iterator begin()
	{
		return const_cast<const FlatHashMap2*>(this)->begin();
	}

	Iterator end()
	{
		return const_cast<const FlatHashMap2*>(this)->end();
	}

	FlatHashMap2()
	{
		table.resize(tableSize, -1);
	}

	std::pair<Iterator, bool> emplace(const Key& key, const Value& value)
	{
		auto hash = Hasher{}(key);
		int32_t tableIndex = hash % tableSize;

		int32_t reserved = nextFreeDataBlock();
		int32_t current = -1;
		for (int32_t* indirectIndex = &table[tableIndex]; ; indirectIndex = &dataBlocks[*indirectIndex].next) {
			if (*indirectIndex == -1 || dataBlocks[*indirectIndex].hash != tableIndex) {
				dataBlocks[reserved] = DataBlock<Pair>({ key, value });
				dataBlocks[reserved].hash = tableIndex;
				dataBlocks[reserved].next = *indirectIndex;
				dataBlocks[reserved].prev = current;

				if (*indirectIndex != -1) {
					dataBlocks[*indirectIndex].prev = reserved;
				}
				else {
					if (firstDataBlock == -1) {
						firstDataBlock = reserved;
						lastDataBlock = reserved;
					}
					else {
						if (current == -1) {
							dataBlocks[lastDataBlock].next = reserved;
							dataBlocks[reserved].prev = lastDataBlock;
						}
						lastDataBlock = reserved;
					}
				}

				*indirectIndex = reserved;

				return { Iterator(*this, *indirectIndex), true };
			}

			if (dataBlocks[*indirectIndex].pair.first == key) {
				dataBlocksFreeBuckets.push_back(reserved);
				return { Iterator(*this, *indirectIndex), false };
			}

			current = *indirectIndex;
		}
	}

	bool erase(const Key& key)
	{
		auto it = find(key);
		return erase(it) != end();
	}

	Iterator erase(Iterator it)
	{
		if (it == end()) {
			return end();
		}

		int32_t dataBlockIndex = it.dataBlockIndex;
		int32_t prevBlockIndex = dataBlocks[dataBlockIndex].prev;
		int32_t nextBlockIndex = dataBlocks[dataBlockIndex].next;

		if (firstDataBlock == dataBlockIndex) {
			firstDataBlock = nextBlockIndex;
		}
		else {
			dataBlocks[prevBlockIndex].next = nextBlockIndex;
		}

		if (lastDataBlock == dataBlockIndex) {
			lastDataBlock = prevBlockIndex;
		}
		else {
			dataBlocks[nextBlockIndex].prev = prevBlockIndex;
		}

		int32_t tableIndex = dataBlocks[dataBlockIndex].hash;
		if (table[tableIndex] == dataBlockIndex) {
			table[tableIndex] = nextBlockIndex != -1 && dataBlocks[nextBlockIndex].hash == dataBlocks[dataBlockIndex].hash
				? nextBlockIndex
				: -1;
		}

		dataBlocksFreeBuckets.push_back(dataBlockIndex);
		return Iterator(*this, nextBlockIndex);
	}

	ConstIterator find(const Key& key) const
	{
		auto hash = Hasher{}(key);
		int32_t tableIndex = hash % tableSize;

		for (int32_t dataBlockIndex = table[tableIndex]; ; dataBlockIndex = dataBlocks[dataBlockIndex].next) {
			if (dataBlockIndex == -1 || dataBlocks[dataBlockIndex].hash != tableIndex) {
				return end();
			}

			if (dataBlocks[dataBlockIndex].pair.first == key) {
				return ConstIterator(*this, dataBlockIndex);
			}
		}
	}

	Iterator find(const Key& key)
	{
		return const_cast<const FlatHashMap2*>(this)->find(key);
	}

	bool contains(const Key& key) const
	{
		return find(key) != end();
	}

	size_t size() const
	{
		return dataBlocks.size() - dataBlocksFreeBuckets.size();
	}

public:
	int32_t nextFreeDataBlock()
	{
		if (!dataBlocksFreeBuckets.empty()) {
			auto bucket = dataBlocksFreeBuckets.back();
			dataBlocksFreeBuckets.pop_back();
			return bucket;
		}

		dataBlocks.push_back({});
		return dataBlocks.size() - 1;
	}

	std::vector<int32_t> table;

	int32_t firstDataBlock = -1;
	int32_t lastDataBlock = -1;
	std::vector<DataBlock<Pair>> dataBlocks;
	std::vector<int32_t> dataBlocksFreeBuckets;
};
