
// add operator--

namespace
{
	constexpr size_t tableSize = 10000;
};

class TableBlock
{
public:
	int32_t dataIndex = -1;
	int32_t next = -1;
	int32_t prev = -1;
};

template <class Pair>
class PairData
{
public:
	PairData() {}
	PairData(const Pair& pair) : pair(pair) {}
	Pair pair;
	int32_t next = -1;
	int32_t prev = -1;
};

template <class FlatHashMap>
class ConstIterator
{
public:
	using Pair = FlatHashMap::Pair;

	ConstIterator(const FlatHashMap& flatHashMap, int32_t tableBlockIndex, int32_t dataIndex)
		: flatHashMap(flatHashMap), tableBlockIndex(tableBlockIndex), dataIndex(dataIndex)
	{}

	bool operator==(const ConstIterator& it) const
	{
		return tableBlockIndex == it.tableBlockIndex && dataIndex == it.dataIndex;
	}

	bool operator!=(const ConstIterator& it) const
	{
		return dataIndex != it.dataIndex;
	}

	ConstIterator& operator++()
	{
		if (tableBlockIndex == -1) {
			throw std::exception();
		}

		dataIndex = flatHashMap.pairDatas[dataIndex].next;

		if (dataIndex == -1) {
			tableBlockIndex = flatHashMap.tableBlocks[tableBlockIndex].next;

			if (tableBlockIndex == -1) {
				return *this;
			}

			dataIndex = flatHashMap.tableBlocks[tableBlockIndex].dataIndex;
		}
	}

	const Pair& operator*()
	{
		return flatHashMap.pairDatas[dataIndex].pair;
	}

	const Pair* operator->()
	{
		return &**this;
	}

public:
	int32_t tableBlockIndex = -1;
	int32_t dataIndex = -1;
	const FlatHashMap& flatHashMap;
};

template <class FlatHashMap>
class Iterator : public ConstIterator<FlatHashMap>
{
public:
	using Mybase = ConstIterator<FlatHashMap>;
	using Mybase::Mybase;

	Iterator(const Mybase& constIt)
		: Iterator(constIt.flatHashMap, constIt.tableBlockIndex, constIt.dataIndex)
	{
	}

	Iterator& operator++()
	{
		if (ConstIterator<FlatHashMap>::tableBlockIndex == -1) {
			throw std::exception();
		}

		ConstIterator<FlatHashMap>::dataIndex = ConstIterator<FlatHashMap>::flatHashMap.pairDatas[ConstIterator<FlatHashMap>::dataIndex].next;

		if (ConstIterator<FlatHashMap>::dataIndex == -1) {
			ConstIterator<FlatHashMap>::tableBlockIndex = ConstIterator<FlatHashMap>::flatHashMap.tableBlocks[ConstIterator<FlatHashMap>::tableBlockIndex].next;

			if (ConstIterator<FlatHashMap>::tableBlockIndex == -1) {
				return *this;
			}

			ConstIterator<FlatHashMap>::dataIndex = ConstIterator<FlatHashMap>::flatHashMap.tableBlocks[ConstIterator<FlatHashMap>::tableBlockIndex].dataIndex;
		}

		return *this;
	}

	FlatHashMap::Pair& operator*()
	{
		const FlatHashMap::Pair* pair = &ConstIterator<FlatHashMap>::flatHashMap.pairDatas[ConstIterator<FlatHashMap>::dataIndex].pair;
		return *const_cast<FlatHashMap::Pair*>(pair);
	}

	FlatHashMap::Pair* operator->()
	{
		const FlatHashMap::Pair* pair = &**this;
		return const_cast<FlatHashMap::Pair*>(pair);
	}
};

template <class Key, class Value, class Hasher = std::hash<Key>>
class FlatHashMap
{
public:
	using Iterator = Iterator<FlatHashMap>;
	using ConstIterator = ConstIterator<FlatHashMap>;
	using Pair = std::pair<Key, Value>;

	ConstIterator begin() const
	{
		if (firstTableBlock == -1) {
			return end();
		}
		return ConstIterator(*this, firstTableBlock, tableBlocks[firstTableBlock].dataIndex);
	}

	ConstIterator end() const
	{
		return ConstIterator(*this, -1, -1);
	}

	Iterator begin()
	{
		return const_cast<const FlatHashMap*>(this)->begin();
	}

	Iterator end()
	{
		return const_cast<const FlatHashMap*>(this)->end();
	}

	FlatHashMap()
	{
		table.resize(tableSize, -1);
	}

	std::pair<Iterator, bool> emplace(const Key& key, const Value& value)
	{
		auto hash = Hasher{}(key);
		int32_t tableIndex = hash % tableSize;
		int32_t tableBlockIndex = table[tableIndex];

		if (tableBlockIndex == -1) {
			tableBlockIndex = nextFreeTableBlock();
			tableBlocks[tableBlockIndex] = TableBlock();

			if (firstTableBlock != -1) {
				tableBlocks[lastTableBlock].next = tableBlockIndex;
				tableBlocks[tableBlockIndex].prev = lastTableBlock;
				lastTableBlock = tableBlockIndex;
			}
			else {
				firstTableBlock = tableBlockIndex;
				lastTableBlock = tableBlockIndex;
			}

			table[tableIndex] = tableBlockIndex;
		}

		int32_t reserved = nextFreePairData();
		int32_t prev = -1;
		for (int32_t* indirectIndex = &tableBlocks[tableBlockIndex].dataIndex; ; indirectIndex = &pairDatas[*indirectIndex].next) {
			if (*indirectIndex == -1) {
				*indirectIndex = reserved;
				pairDatas[*indirectIndex] = PairData<Pair>({ key, value });
				pairDatas[*indirectIndex].prev = prev;
				return { Iterator(*this, tableBlockIndex, *indirectIndex), true };
			}

			if (pairDatas[*indirectIndex].pair.first == key) {
				pairDataFreeBuckets.push_back(reserved);
				return { Iterator(*this, tableBlockIndex, *indirectIndex), false };
			}

			prev = *indirectIndex;
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

		auto next = it;
		++next;

		int32_t tableBlockIndex = it.tableBlockIndex;
		int32_t dataIndex = it.dataIndex;

		{
			int32_t prevPairDataIndex = pairDatas[dataIndex].prev;
			int32_t nextPairDataIndex = pairDatas[dataIndex].next;

			if (prevPairDataIndex != -1) {
				pairDatas[prevPairDataIndex].next = nextPairDataIndex;
			}

			if (nextPairDataIndex != -1) {
				pairDatas[nextPairDataIndex].prev = prevPairDataIndex;
			}

			if (tableBlocks[tableBlockIndex].dataIndex == dataIndex) {
				tableBlocks[tableBlockIndex].dataIndex = nextPairDataIndex;
			}

			pairDataFreeBuckets.push_back(dataIndex);
		}

		if (tableBlocks[tableBlockIndex].dataIndex == -1) {
			int32_t prevBlockIndex = tableBlocks[tableBlockIndex].prev;
			int32_t nextBlockIndex = tableBlocks[tableBlockIndex].next;

			if (prevBlockIndex != -1) {
				tableBlocks[prevBlockIndex].next = nextBlockIndex;
			}

			if (nextBlockIndex != -1) {
				tableBlocks[nextBlockIndex].prev = prevBlockIndex;
			}

			if (firstTableBlock == tableBlockIndex) {
				firstTableBlock = nextBlockIndex;
			}

			if (lastTableBlock == tableBlockIndex) {
				lastTableBlock = prevBlockIndex;
			}

			tableBlockFreeBuckets.push_back(tableBlockIndex);

			auto hash = Hasher{}(it->first);
			int32_t tableIndex = hash % tableSize;
			table[tableIndex] = -1;
		}

		return next;
	}

	ConstIterator find(const Key& key) const
	{
		auto hash = Hasher{}(key);
		int32_t tableIndex = hash % tableSize;
		int32_t tableBlockIndex = table[tableIndex];

		if (tableBlockIndex == -1) {
			return end();
		}

		for (int32_t dataIndex = tableBlocks[tableBlockIndex].dataIndex; ; dataIndex = pairDatas[dataIndex].next) {
			if (dataIndex == -1) {
				return end();
			}
			
			if (pairDatas[dataIndex].pair.first == key) {
				return ConstIterator(*this, tableBlockIndex, dataIndex);
			}
		}
	}

	Iterator find(const Key& key)
	{
		return const_cast<const FlatHashMap*>(this)->find(key);
	}

	bool contains(const Key& key) const
	{
		return find(key) != end();
	}

	size_t size() const
	{
		return pairDatas.size() - pairDataFreeBuckets.size();
	}

public:
	int32_t nextFreeTableBlock()
	{
		if (!tableBlockFreeBuckets.empty()) {
			auto bucket = tableBlockFreeBuckets.back();
			tableBlockFreeBuckets.pop_back();
			return bucket;
		}

		tableBlocks.push_back({});
		return tableBlocks.size() - 1;
	}

	int32_t nextFreePairData()
	{
		if (!pairDataFreeBuckets.empty()) {
			auto bucket = pairDataFreeBuckets.back();
			pairDataFreeBuckets.pop_back();
			return bucket;
		}

		pairDatas.push_back({});
		return pairDatas.size() - 1;
	}

	std::vector<int32_t> table;

	int32_t firstTableBlock = -1;
	int32_t lastTableBlock = -1;
	std::vector<int32_t> tableBlockFreeBuckets;
	std::vector<TableBlock> tableBlocks;

	std::vector<PairData<Pair>> pairDatas;
	std::vector<int32_t> pairDataFreeBuckets;
};
