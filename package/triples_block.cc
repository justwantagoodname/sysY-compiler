#include "triples.h"


void Triples::MinTempVar()
{
	if (temp_count == -1) {
		throw " need make() before. ";
		return;
	}
	int* var_begin = new int[temp_count + 5];
	int* var_end = new int[temp_count + 5];
	int* var_merage = new int[temp_count + 5];

	for (int i = 0; i < temp_count; ++i)var_merage[i] = i;

	std::vector<int> block_begin;

	memset(var_begin, -1, temp_count);
	memset(var_end, -1, temp_count);

	for (int i = 0, j = triples.size() - 1; i < triples.size(); ++i, --j) {
		// 双向扫描确定每个临时变量的始末位置
#define iftin(idx, p, lst) if(triples[idx].##p.type == TT.temp && lst[triples[idx].##p.value] == -1) \
		lst[triples[idx].##p.value] = idx

		iftin(i, e1, var_begin);
		iftin(i, e2, var_begin);
		iftin(i, to, var_begin);

		iftin(j, e1, var_end);
		iftin(j, e2, var_end);
		iftin(j, to, var_end);
#undef iftin

		// 找到每一个连续运行区间始点
		if (triples[i].cmd == Cmd.tag ||
			(triples[i].cmd >= Cmd.jmp && triples[i].cmd <= Cmd.jnef))
		{
			block_begin.emplace_back(i);
		}
	}
	block_begin.emplace_back(triples.size());

	// 遍历每一个区间，迭代合并无重叠临时变量直到不存在无合并变量
	for (int _i = 0; _i < block_begin.size() - 1; ++_i) {
		int b = block_begin[_i];
		int e = block_begin[_i + 1];

		std::vector<int> block_temp;

		// 找到区间内临时变量
		for (int i = 0; i < temp_count; ++i) {
			if (var_begin[i] >= b && var_end[i] < e) {
				block_temp.emplace_back(i);
			}
		}

		// 采用贪心求解每个变量的最大合并占用

		// 合并标记
		std::vector<bool> meraged(block_temp.size(), false);

		// 临时函数：查找未被合并过的离某个起点最近起始的变量
		auto found_nearlest = [var_begin, block_temp, meraged](int begin) {
			int min = -1;
			int min_temp = -1;
			for (int i = 0; i < block_temp.size(); ++i) {
				int t = block_temp[i];
				if (min == -1 || (var_begin[t] - begin >= 0 && var_begin[t] - begin < min)) {
					min = var_begin[t] - begin;
					min_temp = t;
				}
			}
			return min_temp;
			};

		auto meraged_finish = [meraged]() {
			for (auto i : meraged) {
				if (!i) return false;
			}
			return true;
			};

		auto merage_temp_var = [&meraged, &var_merage, &var_begin, &var_end](int a, int b) {
			var_merage[b] = var_merage[a];
			var_end[a] = var_end[b];
			var_begin[b] = var_begin[a];

			meraged[a] = true;
			meraged[b] = true;
			};

		// 合并
		while (!meraged_finish()) {
			
			int bottom = b;
			int nextt = found_nearlest(bottom);
			int begint = nextt;
			while (nextt != -1) {
				merage_temp_var(begint, nextt);
				nextt = found_nearlest(bottom);
			}
		}

	}

}
