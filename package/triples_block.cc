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

	// ��-1
	memset(var_begin, -1, sizeof(int) * temp_count);
	memset(var_end, -1, sizeof(int) * temp_count);

	for (int i = 0, j = triples.size() - 1; i < triples.size(); ++i, --j) {
		// ˫��ɨ��ȷ��ÿ����ʱ������ʼĩλ��
#define iftin(idx, p, lst) if(triples[idx].p.type == TT.temp && lst[triples[idx].p.value] == -1) \
		lst[triples[idx].p.value] = idx

		iftin(i, e1, var_begin);
		iftin(i, e2, var_begin);
		iftin(i, to, var_begin);

		iftin(j, e1, var_end);
		iftin(j, e2, var_end);
		iftin(j, to, var_end);
#undef iftin

		// �ҵ�ÿһ��������������ʼ��
		if (triples[i].cmd == Cmd.tag ||
			(triples[i].cmd >= Cmd.jmp && triples[i].cmd <= Cmd.jnef))
		{
			block_begin.emplace_back(i);
		}
	}
	block_begin.emplace_back(triples.size());

	// ����ÿһ�����䣬�����ϲ����ص���ʱ����ֱ���������޺ϲ�����
	for (int _i = 0; _i < block_begin.size() - 1; ++_i) {
		int b = block_begin[_i];
		int e = block_begin[_i + 1];

		std::vector<int> block_temp;

		// �ҵ���������ʱ����
		for (int i = 0; i < temp_count; ++i) {
			if (var_begin[i] >= b && var_end[i] < e) {
				block_temp.emplace_back(i);
			}
		}

		// ����̰�����ÿ�����������ϲ�ռ��

		// �ϲ����
		std::vector<bool> meraged(block_temp.size(), false);

		// ��ʱ����������δ���ϲ�������ĳ����������ʼ�ı���
		auto found_nearlest = [&var_begin, &block_temp, &meraged](int begin) {
			int min = -1;
			int min_temp = -1;
			for (auto t : block_temp) {
				if (var_begin[t] - begin >= 0 && (min == -1 || var_begin[t] - begin < min) && !meraged[t]) {
					min = var_begin[t] - begin;
					min_temp = t;
				}
			}
			return min_temp;
			};

		auto meraged_finish = [&meraged]() {
			for (auto i : meraged) {
				if (!i) return false;
			}
			return true;
			};

		auto merage_temp_var = [&meraged, &var_merage, &var_begin, &var_end](int a, int b) {
			var_merage[b] = var_merage[a];
			var_end[a] = var_end[b];
			var_begin[b] = var_begin[a];

			meraged[b] = true;
			};

		// �ϲ�
		while (!meraged_finish()) {

			int bottom = b;
			int nextt = found_nearlest(bottom);
			int begint = nextt;
			if (begint == -1) {
				break;
			}
			while (nextt != -1) {
				merage_temp_var(begint, nextt);
				bottom = var_end[nextt];
				nextt = found_nearlest(bottom);
			}
			meraged[begint] = true;
		}
	}

	// д�غϲ����
	for (int i = 0; i < triples.size(); ++i) {
#define sett(p) if(triples[i].p.type ==	TT.temp) triples[i].p.value = var_merage[triples[i].p.value]
		sett(e1);
		sett(e2);
		sett(to);
#undef sett
	}

	delete[] var_begin;
	delete[] var_end;
	delete[] var_merage;
}

void Triples::EliUnnecVar()
{
	// �������������������м����

	int* imd_temp = new int[temp_count + 5];
	memset(imd_temp, -1, sizeof(int) * temp_count);

#define sett(p) if(triples[i].p.type ==	TT.temp && imd_temp[triples[i].p.value] != -1) do {\
triples[i].p.value = imd_temp[triples[i].p.value];\
triples[i].p.type = TT.imd;\
} while (0)

#define gett if(triples[i].cmd == Cmd.mov && triples[i].e1.type == TT.imd && triples[i].to.type == TT.temp)\
do{\
imd_temp[triples[i].to.value] = triples[i].e1.value;\
triples.erase(triples.begin() + i);\
--i;\
}while(0)

	for (int i = 0; i < triples.size(); ++i) {
		sett(e1);
		sett(e2);
		gett;
	}

#undef sett
#undef gett

	delete[] imd_temp;
}
